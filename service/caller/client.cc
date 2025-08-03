#include "../user.pb.h"
#include "rpc_application.h"
#include "rpc_controller.h"
#include "rpc_logger.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

void SendRPCRequest(int thread_id, std::atomic<int> &success_count, std::atomic<int> &fail_count)
{
   // 演示RPC方法远程调用Login
   User::UserServiceRPC_Stub stub(new RPCchannel(false));
   // RPC方法请求参数
   User::LoginRequest request;
   request.set_name("test_user_" + std::to_string(thread_id));
   request.set_pwd("test_password_" + std::to_string(thread_id));
   // RPC方法响应参数
   User::LoginResponse response;
   RPCcontroller controller;
   stub.Login(&controller, &request, &response, nullptr);
   if (controller.Failed())
   {
      std::cout << controller.ErrorText() << std::endl;
   }
   else
   {
      if (0 == response.result().errcode())
      {
         std::cout << "Thread " << thread_id << " RPC Login success: "
                   << "Name: " << request.name() << ", "
                   << "Password: " << request.pwd() << std::endl;
         success_count++;
      }
      else
      {
         std::cout << "Thread " << thread_id << " RPC Login failed: "
                   << response.result().errmsg() << std::endl;
         fail_count++;
      }
   }
}

int main(int argc, char **argv)
{
   // 整个程序启动后，想要使用RPC框架的功能，必须先调用RPCapplication::Init()方法进行初始化，且只初始化一次
   RPCapplication::Init(argc, argv);
   RPClogger logger("myRPC");
   // 创建多个线程模拟并发请求，每个线程发送指定次数的RPC请求
   const int thread_count = 1000;
   const int request_per_thread = 2;
   // 使用原子变量来统计成功和失败的请求数量
   std::atomic<int> success_count(0);
   std::atomic<int> fail_count(0);
   std::vector<std::thread> threads;
   auto start_time = std::chrono::high_resolution_clock::now();
   for (int i = 0; i < thread_count; i++)
   {
      threads.emplace_back([argc, argv, i, &success_count, &fail_count, request_per_thread]()
                           {
         for (int j = 0; j < request_per_thread; j++)
         {
            SendRPCRequest(i * request_per_thread + j, success_count, fail_count);
         } });
   }
   for (auto &thread : threads)
   {
      thread.join();
   }
   auto end_time = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> elapsed = end_time - start_time;
   auto qps = thread_count * request_per_thread / elapsed.count();
   // 输出统计信息
   LOG(INFO) << "Total requests: " << thread_count * request_per_thread;
   LOG(INFO) << "Success count: " << success_count.load();
   LOG(INFO) << "Fail count: " << fail_count.load();
   LOG(INFO) << "Elapsed time: " << elapsed.count() << " seconds";
   LOG(INFO) << "QPS: " << qps;
   return 0;
}