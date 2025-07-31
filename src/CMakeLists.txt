# 搜索当前目录下的所有源文件，并将它们的路径存储在 SRC_FILES 变量中
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# 搜索当前目录下 protobuf 生成的文件，并将它们的路径存储在 PROTO_SRCS 变量中
file(GLOB PROTO_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.pb.cc)

# 创建一个名为 rpc_core 的静态库，包含 SRC_FILES 和 PROTO_SRCS 中的所有源文件
add_library(rpc_core STATIC ${SRC_FItarget_include_directoriesLES} ${PROTO_SRCS})

# 将 protobuf 库链接到 rpc_core 库，使其成为公共依赖项
target_link_libraries(rpc_core PUBLIC protobuf)

# 将 ../include 目录添加到 rpc_core 库的公共包含目录中
target_include_directories(rpc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# 为 rpc_core 库添加编译选项，使用 C++11 标准并启用所有警告
target_compile_options(rpc_core PRIVATE -std=c++11 -Wall)