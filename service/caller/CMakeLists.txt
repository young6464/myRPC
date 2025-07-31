# 获取client端的源文件
file(GLOB CLIENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# 获取protobuf生成的.cc文件
file(GLOB PROTO_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../*.pb.cc)

# 创建client 端可执行文件
add_executable(client ${CLIENT_SRC} ${PROTO_SRCS})

# 为 client 可执行文件添加必要链接库，尤其是生成的静态库
target_link_libraries(client rpc_core ${LIBS})

# 为 client 可执行文件添加编译选项，使用 C++11 标准并启用所有警告
target_compile_options(client PRIVATE -std=c++11 -Wall)

# 设置 client 可执行文件的属性及其输出目录
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)