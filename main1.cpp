/*
 * @Author: 培岩 樊 fanpy@io-ai.tech
 * @Date: 2026-06-04 10:03:51
 * @LastEditors: 培岩 樊 fanpy@io-ai.tech
 * @LastEditTime: 2026-06-04 10:04:09
 * @FilePath: /io_gripper_sdk/main1.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置
 * 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <yaml-cpp/yaml.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "io_gripper.hpp"

using namespace io::gripper;

void saveImageToFile(const GripperCompressedImage& image,
                     const std::string& filename) {
  std::ofstream file(filename, std::ios::binary);
  if (file.is_open()) {
    file.write(reinterpret_cast<const char*>(image.data.data()),
               image.data.size());
    file.close();
    std::cout << "图像已保存: " << filename << " (" << image.data.size()
              << " bytes)" << std::endl;
  } else {
    std::cerr << "无法保存图像: " << filename << std::endl;
  }
}

int main() {
  std::unique_ptr<find_port> port_;
  std::vector<CameraInfo> res_caminfo = port_->get_usb_cameras_info();
  std::string first_id_path = port_->get_video_id_path(
      port_->find_video_by_camera_serial(res_caminfo[1].serial));
  std::cout << "first_id_path: " << first_id_path << std::endl;
  std::string video_device = port_->find_video_by_id_path(first_id_path);
  std::cout << "video_device: " << video_device << std::endl;

  // 2. 创建相机对象（使用默认配置）
  GripperCamera camera(video_device);

  // 3. 检查相机状态
  std::cout << "\n相机状态: " << (camera.isOpened() ? "已打开" : "未打开")
            << std::endl;

  // 4. 获取相机设置信息
  CameraSettings s = camera.getCameraSettings(video_device);
  std::cout << "\n相机设置:" << std::endl;
  std::cout << "  宽度: " << s.width << std::endl;
  std::cout << "  高度: " << s.height << std::endl;
  std::cout << "  帧率: " << s.fps << std::endl;

  // 5. 捕获图像
  GripperCompressedImage image;
  std::cout << "\n正在捕获图像..." << std::endl;

  if (camera.captureCompressedImage(image)) {
    std::cout << "捕获成功!" << std::endl;
    std::cout << "  时间戳: " << image.timestamp_ns << " ns" << std::endl;
    std::cout << "  分辨率: " << image.width << " x " << image.height
              << std::endl;
    std::cout << "  格式: " << image.format << std::endl;
    std::cout << "  数据大小: " << image.data.size() << " bytes" << std::endl;

    // 6. 保存图像到文件
    std::string filename = "captured_image.jpg";
    saveImageToFile(image, filename);

  } else {
    std::cerr << "捕获失败!" << std::endl;
    return -1;
  }

  std::cout << "\n=== 测试完成 ===" << std::endl;
  return 0;
}