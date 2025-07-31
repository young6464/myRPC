# 设置CMake的最低版本要求
cmake_minimum_required(VERSION 3.0)
# 设置项目名称和版本
project(MyRPC VERSION 1.0)

# 指定全局C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找并配置 Protocol Buffers 库
find_package(Protobuf REQUIRED)
# 包含 Protocol Buffers 的头文件目录
include_directories(${Protobuf_INCLUDE_DIRS})

# 设置需要链接的库
set(LIBS
   protobuf        # Protocol Buffers 库
   pthread         # POSIX 线程库
   zookeeper_mt    # Zookeeper 多线程库
   muduo_net       # Muduo 网络库
   muduo_base      # Muduo 基础库
   glog            # Google 日志库
)

# 添加子目录
add_subdirectory(src)
add_subdirectory(service)