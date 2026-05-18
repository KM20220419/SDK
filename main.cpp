#include <yaml-cpp/yaml.h>

#include <iostream>

#include "include/io_gripper.hpp"

using namespace io::gripper;

struct GripperContext {
  std::unique_ptr<GripperDriver> driver;
  DeviceProfile profile;
};

DeviceProfile create_gripper_driver(const std::string& config_file_path) {
  DeviceProfile profile;
  YAML::Node config = YAML::LoadFile(config_file_path);
  auto DeviceProfile_Node = config["DeviceProfile"];
  // 基础信息
  profile.model_name =
      DeviceProfile_Node["device_info"]["model_name"].as<std::string>();
  profile.servo_id = static_cast<uint8_t>(
      DeviceProfile_Node["device_info"]["servo_id"].as<int>());
  profile.recommended_baudrate =
      DeviceProfile_Node["device_info"]["recommended_baudrate"].as<int>();

  // 安全限制
  profile.min_voltage_V =
      DeviceProfile_Node["safety_limits"]["min_voltage_v"].as<float>();
  profile.max_voltage_V =
      DeviceProfile_Node["safety_limits"]["max_voltage_v"].as<float>();
  profile.max_temperature_C =
      DeviceProfile_Node["safety_limits"]["max_temperature_c"].as<float>();
  profile.start_power =
      DeviceProfile_Node["safety_limits"]["start_pow"].as<uint8_t>(8);
  profile.release_torque_on_disconnect =
      DeviceProfile_Node["safety_limits"]["release_torque_on_disconnect"]
          .as<bool>(true);
  profile.max_servo_velocity =
      DeviceProfile_Node["safety_limits"]["max_servo_velocity"].as<uint16_t>(
          2000);

  // 标定参数
  profile.calib_max_position_raw =
      DeviceProfile_Node["calibration"]["calib_max_position_raw"]
          .as<uint16_t>();
  profile.calib_min_position_raw =
      DeviceProfile_Node["calibration"]["calib_min_position_raw"]
          .as<uint16_t>();
  profile.calib_max_width_mm =
      DeviceProfile_Node["calibration"]["calib_max_width_mm"].as<float>();
  profile.calib_min_width_mm =
      DeviceProfile_Node["calibration"]["calib_min_width_mm"].as<float>();

  //   std::string port =
  //       DeviceProfile_Node["device_info"]["port"].as<std::string>("/dev/ttyUSB0");

  //   auto driver_ptr =
  //       std::make_unique<GripperDriver>(port, profile, config_file_path);
  //   return {std::move(driver_ptr), profile};
  return profile;
}

// 用夹爪语义控制
GripperCommand cmd_gripper_move(std::string config_path) {
  try {
    // 重新加载配置
    YAML::Node config_root = YAML::LoadFile("../gripper_config.yaml");
    auto gripper_node = config_root["gripper"]["use_width_normalized"];

    GripperCommand cmd;
    cmd.use_width_mm = gripper_node["use_width_mm"].as<bool>();
    cmd.use_normalized_opening =
        gripper_node["use_normalized_opening"].as<bool>();
    cmd.width_mm = gripper_node["width_mm"].as<float>();
    cmd.normalized_opening = gripper_node["normalized_opening"].as<float>();
    cmd.max_effort = gripper_node["max_effort"].as<float>();
    cmd.speed = gripper_node["speed"].as<float>();

    return cmd;
  } catch (const std::exception& e) {
    std::cerr << "加载动作配置失败: " << e.what() << std::endl;
    throw;
  }
}

int main() {
  try {
    std::cout << "正在尝试初始化库..." << std::endl;

    // 查找端口
    std::unique_ptr<find_port> port_;
    std::vector<CameraInfo> res_caminfo = port_->get_usb_cameras_info();
    std::string first_need_port = port_->find_by_path_from_tty(
        port_->resolve_gripper_by_camera_serial(res_caminfo[0].serial));

    std::cout << "first_need_port: " << first_need_port << std::endl;

    std::string path = "../gripper_config.yaml";

    // 配置profile文件
    DeviceProfile profile = create_gripper_driver(path);
    auto driver =
        std::make_unique<GripperDriver>(first_need_port, profile, path);
    if (driver->connect() && driver->ping(profile.servo_id)) {
      driver->initialize({profile.servo_id});

      std::cout << "标定" << std::endl;
      driver->calibrate();

      // 动态加载配置文件，使用夹爪语义
      std::cout << "-------" << std::endl;
      driver->commandGripper(cmd_gripper_move(path));
      driver->disconnect();
    }
  } catch (const std::exception& e) {
    std::cerr << "发生异常: " << e.what() << std::endl;
  }
  return 0;
}