#pragma once

#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/*
 * io_gripper.hpp
 * 飞特机械夹爪SDK
 * 日期: 2026.4.15
 * 作者:
 */
namespace io::gripper {

enum class DriverState {
  DISCONNECTED,
  CONNECTED,
  READY,
  MOVING,
  FAULT,
  ESTOP,
};

struct DeviceProfile {
  std::string model_name;              // 设备型号
  uint8_t servo_id;                    // 默认舵机 ID
  std::string protocol_version;        // 协议版本
  int recommended_baudrate;            // 推荐波特率
  bool supports_sync_write;            // 是否支持同步写
  bool supports_sync_read;             // 是否支持同步读
  bool supports_present_current;       // 是否支持读取当前电流
  float min_voltage_V;                 // 最低电压 (单位：伏特)
  float max_voltage_V;                 // 最高电压 (单位：伏特)
  float max_temperature_C;             // 最高温度 (单位：摄氏度)
  uint16_t max_servo_velocity = 2000;  // 最大速度阈值
  uint8_t start_power = 8;             // 最小启动力默认为8
  bool release_torque_on_disconnect =
      true;  // 3次通信失败：true: 释放力矩 (默认), false: 保持力矩并停止

  // 夹爪特定参数
  float max_width_mm = -1.0;  // 夹爪最大开口宽度 (单位：毫米)
  float min_width_mm = -1.0;  // 夹爪最小开口宽度 (单位：毫米)
  float calib_max_width_mm;   // 基准最大开口宽度 (单位：毫米)
  float calib_min_width_mm;   // 基准最小开口宽度 (单位：毫米)

  float max_position_raw;        // 最大位置原始值 (单位：脉冲数)
  float min_position_raw;        // 最小位置原始值 (单位：脉冲数)
  float calib_max_position_raw;  // 基准最大位置原始值 (单位：脉冲数)
  float calib_min_position_raw;  // 基准最小位置原始值 (单位：脉冲数)

  // 相机参数
  int width = 640;
  int height = 480;
  float fps = 25.0;
  int jpeg_quality = 80;
};

struct GripperState {
  uint64_t timestamp_ns;  // 时间戳，单位纳秒
  uint8_t servo_id;       // 舵机 ID
  uint16_t position_raw;  // 当前位置
  float position_rad;     // 位置值，单位弧度，范围 [0, 2π]
  uint16_t velocity_raw;  // 速度
  float velocity_rad_s;   // 速度，单位弧度每秒
  bool has_load;          // 是否有负载
  uint16_t load_raw;      // 负载值
  bool has_current;       // 是否有电流
  float current_mA;       // 电流值，单位毫安
  float voltage_V;        // 电压值，单位伏特
  float temperature_C;    // 温度值，单位摄氏度
  bool moving;            // 是否正在移动
  bool torque_enabled;    // 扭矩使能状态
  uint16_t error_code;    // 错误代码
  bool communication_ok;  // 通信正常
};

struct GripperCommand {
  bool use_width_mm;            // 是否使用宽度（毫米）来控制夹爪
  float width_mm;               // 夹爪的宽度（毫米）
  bool use_normalized_opening;  // 是否使用归一化开口值（0 到 1）
  float normalized_opening;     // 归一化的开口值（0 到 1）
  float max_effort;             // 最大扭矩值    输入 0～1
  float speed;                  // 夹爪的开合速度
  int timeout_ms;               // 执行操作的超时时间（毫秒）   未使用
};

struct CameraInfo {
  std::string device;
  std::string usb_port;
  std::string vid;
  std::string pid;
  std::string serial;
};

struct GripperCompressedImage {
  uint64_t timestamp_ns = 0;  // SDK 层时间戳，单位 ns
  uint32_t width = 0;
  uint32_t height = 0;
  std::string format = "jpeg";  // 对应 ROS2 CompressedImage 的 format
  std::vector<uint8_t> data;    // JPEG 压缩后的二进制数据
};

struct CameraSettings {
  double width;
  double height;
  double fps;
};

class GripperDriver {
 public:
  explicit GripperDriver(std::string port,
                         std::optional<DeviceProfile> profile = std::nullopt,
                         std::string config_file_path_ = "");
  ~GripperDriver();

  bool connect();
  void disconnect();

  bool ping(uint8_t servo_id);
  std::vector<uint8_t> scanIds(uint8_t start = 1, uint8_t end = 16);
  bool initialize(const std::vector<uint8_t>& servo_ids);

  bool enableTorque(uint8_t servo_id);
  bool disableTorque(uint8_t servo_id);

  bool commandPosition(uint8_t servo_id, std::optional<uint16_t> position_raw,
                       std::optional<float> angle_rad);

  bool commandVelocity(uint8_t servo_id, std::optional<uint16_t> velocity_raw,
                       std::optional<float> velocity_rad_s,
                       std::optional<int> move_time_ms);

  bool setEffortLimit(uint8_t servo_id, std::optional<float> current_limit_mA,
                      std::optional<uint16_t> torque_limit_raw);

  bool commandGripper(const GripperCommand& cmd);
  bool syncMove(const std::vector<uint8_t>& servo_ids,
                const std::vector<uint16_t>& targets);

  GripperState readState(uint8_t servo_id);
  std::vector<GripperState> readGroupState(
      const std::vector<uint8_t>& servo_ids);

  void startPolling(double rate_hz = 50.0);
  void stopPolling();

  bool calibrate();
  bool setSoftLimits(float min_width_mm, float max_width_mm);
  void emergencyStop(bool release_torque = false);
  bool clearFault();

  DriverState state() const;

  GripperState getCachedState(uint8_t servo_id) const;

  bool pickObject(uint8_t servo_id, float width_mm,
                  std::optional<float> speed = 0,
                  std::optional<float> effort = 1000,
                  std::optional<int> timeout_ms = std::nullopt);

  uint16_t getminpos();
  uint16_t getmaxpos();
  uint16_t getcurrentpos();
  float getmaxpos_rad();
  float getspeed_max_rad();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

class find_port {
 public:
  std::vector<CameraInfo> get_usb_cameras_info();
  std::string resolve_gripper_by_camera_serial(
      const std::string& camera_serial);
  std::string find_by_path_from_tty(const std::string& tty_path);

  // 通过相机序列号找到当前 /dev/videoX
  std::string find_video_by_camera_serial(const std::string& camera_serial);

  // 通过 /dev/videoX 获取物理插槽 ID_PATH
  std::string get_video_id_path(const std::string& video_path);

  // 通过物理插槽 ID_PATH 找当前 /dev/videoX
  std::string find_video_by_id_path(const std::string& target_id_path);
};

class GripperCamera {
 public:
  explicit GripperCamera(std::string video_device,
                         std::optional<DeviceProfile> profile = std::nullopt);

  ~GripperCamera();

  bool open();
  void close();
  bool isOpened() const;

  bool captureCompressedImage(GripperCompressedImage& image);
  CameraSettings getCameraSettings(const std::string& device_path);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace io::gripper
