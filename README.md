# IO Gripper SDK

飞特机械夹爪 C++ SDK 文档

**日期**: 2026.6.4  
**作者**: fpy

## 概述

IO Gripper SDK 是一个基于 SCServo 协议的机械夹爪驱动库，提供完整的夹爪控制、状态读取和高级功能接口，同时支持相机图像捕获功能。

## 主要功能

- **连接管理**: 支持串口连接、断开和设备扫描
- **力控制**: 扭矩使能、力限制设置
- **位置控制**: 支持原始脉冲值和弧度制控制
- **速度控制**: 支持多种速度设置方式
- **状态监测**: 实时读取位置、速度、电流、温度等参数
- **同步控制**: 支持多舵机同步动作
- **标定功能**: 自动标定最大/最小开口宽度
- **轮询机制**: 后台轮询更新状态
- **故障处理**: 紧急停止、故障清除
- **相机功能**: USB 相机图像捕获、JPEG 压缩、设备枚举

## 核心类和结构体

### DeviceProfile
设备配置文件，包含波特率、协议版本、夹爪宽度范围和相机参数等。

### GripperState
实时状态结构体，包含位置、速度、电流、温度、错误码等信息。

### GripperCommand
控制命令结构体，支持宽度、归一化开口、速度、力矩等参数。

### GripperDriver
主驱动类，提供所有夹爪控制和查询接口。

### GripperCamera
相机驱动类，提供图像捕获功能，查询相机支持的设置。

### GripperCompressedImage
压缩图像结构体，包含时间戳、分辨率、格式和 JPEG 数据。

### CameraInfo
相机信息结构体，包含设备路径、USB 端口、VID/PID 和序列号。

### CameraSettings
相机设置结构体，包含宽度、高度和帧率。

### find_port
设备查找类，提供 USB 设备枚举和路径转换功能。

## 配置文件使用指南

### 配置文件位置

配置文件位于 `gripper_config.yaml`，在初始化驱动时加载。

### 配置文件结构

#### 1. DeviceProfile（设备配置）

##### device_info - 设备信息
```yaml
device_info:
  model_name: Feetech_DeviceProfile      # 设备型号名称
  port: /dev/serial/by-id/usb-1a86_USB_Serial-if00-port0        # 串口设备路径
  servo_id: 1                             # 默认舵机 ID
  recommended_baudrate: 128000            # 推荐波特率（bps）
```

**修改建议**:
- `port`: 上面的port大概率不用修改。
- `servo_id`: 自己的舵机id
- `recommended_baudrate`: 常用值115200, 128000

##### capabilities - 功能特性
```yaml
capabilities:
  supports_sync_write: true       # 是否支持同步写入
  supports_sync_read: false       # 是否支持同步读取
  supports_present_current: true  # 是否支持电流读取
```

**说明**: 根据实际硬件能力配置，一般无需修改。

##### safety_limits - 安全限制
```yaml
safety_limits:
  min_voltage_v: 4.5              # 最低工作电压（伏特）
  max_voltage_v: 25.4             # 最高工作电压（伏特）
  max_temperature_c: 70.0         # 最高工作温度（摄氏度）
  max_servo_velocity: 3000        # 最大舵机速度阈值
  start_power: 8                  # 最小启动力（0-255）
  release_torque_on_disconnect: true  # 3次通信失败后是否泄力
```

**修改建议**:
- 根据实际工作环境调整温度和电压限制以及速度上限
- `start_power`: 默认为8,值越大会抖动
- `release_torque_on_disconnect`: 为 `true` 或者 `false` 

##### calibration - 标定参数
```yaml
calibration:
  calib_max_position_raw: 2742    # 基准最大位置原始值（脉冲数）
  calib_min_position_raw: 2293    # 基准最小位置原始值（脉冲数）
  calib_max_width_mm: 100.0       # 基准最大开口宽度（毫米）
  calib_min_width_mm: 0.0         # 基准最小开口宽度（毫米）
```

**修改建议**:
- 首次使用运行 `calibrate()` 自动获取calib_max_position_raw，calib_min_position_raw并写入配置文件。
    注：写入后不能随意修改
- calib_max_width_mm，calib_min_width_mm需要物理测量实际开口宽度
- 这些参数用于位置到宽度的线性映射

##### camera - 相机参数
```yaml
camera:
  width: 640                      # 相机分辨率宽度（像素）
  height: 480                     # 相机分辨率高度（像素）
  fps: 25.0                       # 帧率（帧/秒）
  jpeg_quality: 80                # JPEG 压缩质量（0-100）
  camera_serial: "G2026061"       # 相机序列号（用于设备持久化）
```

**修改建议**:
- `width/height`: 根据相机能力调整，常用值：640x480, 1280x720
- `fps`: 帧率，常用值：15, 25, 30
- `jpeg_quality`: 图像质量，值越高质量越好，文件越大
- `camera_serial`: 相机序列号，用于设备拔插后重新定位

#### 2. gripper（夹爪语义控制参数）

```yaml
gripper:
  use_width_normalized:
    use_width_mm: true            # 是否使用毫米单位宽度控制
    use_normalized_opening: false # 是否使用归一化开口值（0-1）
    width_mm: 80.0                # 默认控制宽度（毫米）
    normalized_opening: 0.1       # 默认开口比例（0 闭合, 1 全开）
    max_effort: 0.15              # 最大夹爪力（0-1）
    speed: 0.5                    # 夹爪运动速度（0-1）
```

**修改建议**:
- `use_width_mm`: `true` 使用宽度控制，`false` 则不使用该方式控制
- `use_normalized_opening`: `true` 使用归一化控制，`false` 则不使用该方式控制
- `max_effort`: 根据产品需求调整 输入范围0 - 1 ，最大表示1000
- `speed`: 运动速度，输入范围0 - 1 最大为之前设置的最大速度

### 加载配置文件示例

```cpp
#include "io_gripper.hpp"

int main() {
    // 方式1: 使用配置文件初始化
    io::gripper::GripperDriver driver(
        "/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0 ",
        std::nullopt,
        "build/gripper_config.yaml"  // 指定配置文件  相对路径
    );
    
    // 方式2: 程序中动态创建配置，确保基准最大位置原始值以及基准最小位置原始值准确
    io::gripper::DeviceProfile profile;
    profile.model_name = "Feetech_DeviceProfile";
    profile.servo_id = 1;
    profile.recommended_baudrate = 128000;
    profile.max_voltage_V = 25.4;
    profile.min_voltage_V = 4.5;
    profile.calib_max_position_raw: 2742 
    profile.calib_min_position_raw: 2293
    profile.calib_max_width_mm = 100.0;
    profile.calib_min_width_mm = 0.0;
    
    // 设置相机参数
    profile.width = 640;
    profile.height = 480;
    profile.fps = 25.0;
    profile.jpeg_quality = 80;
    
    io::gripper::GripperDriver driver(
        "/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0 ",
        profile
    );
    
    if (driver.connect()) {
        // 使用驱动...
    }
    
    return 0;
}
```

## 使用示例

### 夹爪控制示例 参考main.cpp

```cpp
// 创建驱动实例
io::gripper::GripperDriver driver("/dev/ttyUSB0");

// 连接设备
if (driver.connect()) {
    // 扫描设备 ID
    auto ids = driver.scanIds();
    
    // 初始化
    driver.initialize(ids);
    
    // 启用扭矩
    driver.enableTorque(ids[0]);
    
    // 控制夹爪
    io::gripper::GripperCommand cmd;
    cmd.use_width_mm = true;
    cmd.width_mm = 50.0;
    cmd.max_effort = 1.0;
    driver.commandGripper(cmd);
    
    // 读取状态
    auto state = driver.readState(ids[0]);
    
    // 断开连接
    driver.disconnect();
}
```

### 相机使用示例  参考main1.cpp

```cpp
// 创建相机对象
io::gripper::GripperCamera camera("/dev/video0");

// 或者使用 DeviceProfile 配置
io::gripper::DeviceProfile profile;
profile.width = 1280;
profile.height = 720;
profile.fps = 30.0;
io::gripper::GripperCamera camera_with_profile("/dev/video0", profile);

// 打开相机（可选，捕获时会自动打开）
camera.open();

// 捕获图像
io::gripper::GripperCompressedImage image;
if (camera.captureCompressedImage(image)) {
    std::cout << "图像大小: " << image.data.size() << " bytes" << std::endl;
    std::cout << "分辨率: " << image.width << "x" << image.height << std::endl;
    
    // 保存图像到文件
    std::ofstream file("image.jpg", std::ios::binary);
    file.write(reinterpret_cast<const char*>(image.data.data()), image.data.size());
}

// 关闭相机
camera.close();
```

### USB 设备枚举示例

```cpp
// 枚举所有 USB 相机
io::gripper::find_port port_finder;
std::vector<io::gripper::CameraInfo> cameras = port_finder.get_usb_cameras_info();

for (const auto& cam : cameras) {
    std::cout << "设备: " << cam.device << std::endl;
    std::cout << "  USB端口: " << cam.usb_port << std::endl;
    std::cout << "  序列号: " << cam.serial << std::endl;
}

// 通过序列号查找设备路径（解决热插拔问题）
std::string video_path = port_finder.find_video_by_camera_serial("G2026061");
std::cout << "找到设备: " << video_path << std::endl;
```

## API 参考

### 连接控制
- `bool connect()` - 连接设备
- `void disconnect()` - 断开连接

### 设备管理
- `bool ping(uint8_t servo_id)` - ping 设备是否在线
- `std::vector<uint8_t> scanIds(uint8_t start, uint8_t end)` - 扫描设备，检查哪些id在线
- `bool initialize(const std::vector<uint8_t> &servo_ids)` - 初始化

### 扭矩控制
- `bool enableTorque(uint8_t servo_id)` - 启用扭矩
- `bool disableTorque(uint8_t servo_id)` - 禁用扭矩

### 运动控制
- `bool commandPosition()` - 位置控制
- `bool commandVelocity()` - 速度控制
- `bool commandGripper()` - 夹爪语义控制
- `bool syncMove()` - 同步移动

### 状态查询
- `GripperState readState(uint8_t servo_id)` - 读取单个状态
- `std::vector<GripperState> readGroupState()` - 读取多个状态
- `GripperState getCachedState()` - 获取缓存的状态

### 高级功能
- `void startPolling()` - 启动后台轮询
- `void stopPolling()` - 停止轮询
- `bool calibrate()` - 标定
- `bool clearFault()` - 清除故障
- `void emergencyStop()` - 紧急停止

### 相机功能
- `bool GripperCamera::open()` - 打开相机
- `void GripperCamera::close()` - 关闭相机
- `bool GripperCamera::isOpened()` - 检查相机是否打开
- `bool GripperCamera::captureCompressedImage(GripperCompressedImage&)` - 捕获压缩图像
- `CameraSettings GripperCamera::getCameraSettings(const std::string&)` - 获取相机设置

### USB 设备枚举
- `std::vector<CameraInfo> find_port::get_usb_cameras_info()` - 获取所有相机信息
- `std::string find_port::find_video_by_camera_serial(const std::string&)` - 通过序列号找设备
- `std::string find_port::get_video_id_path(const std::string&)` - 获取物理插槽路径
- `std::string find_port::find_video_by_id_path(const std::string&)` - 通过物理插槽找设备

## 常见问题

**Q: 如何找到正确的串口？**  
A: 使用命令 `ls /dev/ttyUSB*` 或 `ls /dev/serial/by-id/` 查看

**Q: 出现：启动失败: bad file: gripper_config.yaml**  
A: 找不到配置文件，确保路径正确

**Q: 如何调整夹爪力度？**  
A: 修改配置文件中的 `max_effort` 或通过 `setEffortLimit()` API 动态调整

**Q: 相机设备路径变化怎么办？**  
A: 使用 `find_port` 类的 `find_video_by_camera_serial()` 或 `find_video_by_id_path()` 方法，通过序列号或物理插槽定位设备

**Q: 如何设置相机分辨率？**  
A: 修改配置文件中的 `camera.width` 和 `camera.height`，或在 `DeviceProfile` 中设置相应字段

## 编译依赖

- Linux系统 根镜像为Ubuntu22.04

- 系统库安装 yaml-cpp：执行 sudo apt-get install libyaml-cpp-dev。

- 安装：sudo apt install libopencv-dev

- 安装：sudo apt install libudev-dev

- 编译器：支持 C++17 或更高版本。

- 头文件：请确保将本 SDK 提供的 include/ 目录加入到项目的包含路径中。

- 库链接：在编译时链接 libio_gripper.so。

## 版本历史

### v1.1 (2026.6.4)
- 新增相机图像捕获功能
- 新增 `GripperCamera` 类
- 新增 `find_port` 类，支持 USB 设备枚举和路径持久化
- 添加 `GripperCompressedImage`、`CameraInfo`、`CameraSettings` 结构体
- 在 `DeviceProfile` 中添加相机参数

### v1.0 (2026.4.15)
- 初始版本
- 支持夹爪基本控制功能