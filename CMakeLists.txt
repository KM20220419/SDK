cmake_minimum_required(VERSION 3.10)
project(test_gripper_link)

set(CMAKE_CXX_STANDARD 17)

# 1. 指定头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. 指定库文件搜索路径 
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 3. 创建可执行程序
add_executable(sdk_test main.cpp)

# 4. 链接动态库
target_link_libraries(sdk_test io_gripper yaml-cpp)

# 5. 设置 RPATH，让程序运行时自动去 lib 目录下找 .so
set_target_properties(sdk_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN/lib'")