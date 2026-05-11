# IO Gripper SDK

飞特机械夹爪 C++ SDK 文档

**日期**: 2026.4.15  
**作者**:

## 概述

IO Gripper SDK 是一个基于 SCServo 协议的机械夹爪驱动库，提供完整的夹爪控制、状态读取和高级功能接口。

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

## 核心类和结构体

### DeviceProfile

设备配置文件，包含波特率、协议版本、夹爪宽度范围等参数。

### GripperState

实时状态结构体，包含位置、速度、电流、温度、错误码等信息。

### GripperCommand

控制命令结构体，支持宽度、归一化开口、速度、力矩等参数。

### GripperDriver

主驱动类，提供所有控制和查询接口。

## 配置文件使用指南

### 配置文件位置

配置文件位于 `gripper_config.yaml`，在创建类对象时加载。

### 配置文件结构

#### 1. DeviceProfile（设备配置）

##### device_info - 设备信息

```yaml
device_info:
  model_name: Feetech_DeviceProfile # 设备型号名称
  port: /dev/serial/by-id/usb-1a86_USB_Serial-if00-port0 # 串口设备路径
  servo_id: 1 # 默认舵机 ID
  recommended_baudrate: 128000 # 推荐波特率（bps）
```

**修改建议**:

- `port`: 上面的port大概率不用修改。
- `servo_id`: 舵机id
- `recommended_baudrate`: 常用值115200, 128000

##### capabilities - 功能特性

```yaml
capabilities:
  supports_sync_write: true # 是否支持同步写入
  supports_sync_read: false # 是否支持同步读取
  supports_present_current: true # 是否支持电流读取
```

**说明**: 根据实际硬件能力配置，一般无需修改。

##### safety_limits - 安全限制

```yaml
safety_limits:
  min_voltage_v: 4.5 # 最低工作电压（伏特）
  max_voltage_v: 25.4 # 最高工作电压（伏特）
  max_temperature_c: 70.0 # 最高工作温度（摄氏度）
  max_servo_velocity: 3000 # 最大舵机速度阈值
  start_power: 8 # 最小启动力（0-255）
  release_torque_on_disconnect: true # 3次通信失败后是否泄力
```

**修改建议**:

- 根据实际工作环境调整温度和电压限制以及速度上限
- `start_power`: 默认为8,值越大会抖动
- `release_torque_on_disconnect`: 为 `true` 或者 `false`

##### calibration - 标定参数

```yaml
calibration:
  calib_max_position_raw: 2742 # 基准最大位置原始值（脉冲数）
  calib_min_position_raw: 2293 # 基准最小位置原始值（脉冲数）
  calib_max_width_mm: 100.0 # 基准最大开口宽度（毫米）
  calib_min_width_mm: 0.0 # 基准最小开口宽度（毫米）
```

**修改建议**:

- 首次使用运行 `calibrate()` 自动获取calib_max_position_raw，calib_min_position_raw并写入配置文件。
- calib_max_width_mm，calib_min_width_mm需要物理测量实际开口宽度
- 这些参数用于位置到宽度的线性映射，这些参数作为基准不可随意修改,

#### 2. gripper（夹爪语义控制参数）

```yaml
gripper:
  use_width_normalized:
    use_width_mm: true # 是否使用毫米单位宽度控制
    use_normalized_opening: false # 是否使用归一化开口值（0-1）
    width_mm: 80.0 # 默认控制宽度（毫米）
    normalized_opening: 0.1 # 默认开口比例（0 闭合, 1 全开）
    max_effort: 0.15 # 最大夹爪力（0-1）
    speed: 0.5 # 夹爪运动速度（0-1）
```

**修改建议**:

- `use_width_mm`: `true` 使用宽度控制，`false` 则不使用该方式控制
- `use_normalized_opening`: `true` 使用归一化控制，`false` 则不使用该方式控制
- `max_effort`: 根据产品需求调整 输入范围0 - 1 ，最大表示1000
- `speed`: 运动速度，输入范围0 - 1 最大为之前设置的最大速度


## API 参考

### 连接控制

- `bool connect()` - 连接设备
- `void disconnect()` - 断开连接

### 设备管理

- `bool ping(uint8_t servo_id)` - ping设备是否在线
- `std::vector<uint8_t> scanIds(uint8_t start, uint8_t end)` - 扫描设备，检查哪些id在线
- `bool initialize(const std::vector<uint8_t> &servo_ids)` - 初始化一组id

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
- `GripperState getCachedState()` - 获取缓存的状态（开启轮询后）
- `uint16_t getminpos()` - 获取目前的最小限位
- `uint16_t getmaxpos()` - 获取目前的最大限位
- `uint16_t getcurrentpos()` - 获取当前位置
- `float getmaxpos_rad()` - 获取可输入的最大位置弧度
- `flaot getspeed_max_rad()` - 获取可输入的最大速度（rad/s）

### 高级功能

- `void startPolling()` - 启动后台轮询
- `void stopPolling()` - 停止轮询
- `bool calibrate()` - 标定 在将标定结果写入文件后，之后启动不用再次标定 前提：基准限位结果没变
- `bool clearFault()` - 清除故障
- `void emergencyStop()` - 紧急停止

## 常见问题

**Q: 如何找到正确的串口？**  
A: 使用命令 `ls /dev/ttyUSB*` 或 `ls /dev/serial/by-id/` 查看

**Q: 出现：启动失败: bad file: gripper_config.yaml**  
A: 找不到配置文件，确保路径正确

**Q: 如何调整夹爪力度？**  
A: 修改配置文件中的 `max_effort` 或通过 `setEffortLimit()` API 动态调整

## 编译依赖

- 系统库安装 yaml-cpp：执行 sudo apt-get install libyaml-cpp-dev。

- 编译器：支持 C++17 或更高版本。

- 头文件：请确保将本 SDK 提供的 include/ 目录加入到项目的包含路径中。

- 库链接：在编译时链接 libio_gripper.so。


## 加载配置文件示例

- 提供使用示例
- 提供CMake编译

### CMake编译使用方法
mkdir build && cd build
cmake ..
make
./sdk_test
