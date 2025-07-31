# 项目简介

## 运行环境

## 库准备

## 编译指令

```shell
cd myRPC

mkdir build && cd build && cmake .. && make -j${nproc}

cd service
protoc --cpp_out=. user.proto

cd src
proto --cpp_out=. rpc_header.proto

cd bin
./server -i ./test.conf
./ckient -i ./test.conf
```
