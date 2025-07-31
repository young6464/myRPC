#include "../user.pb.h"
#include "rpc_application.h"
#include "rpc_provider.h"
#include <iostream>
#include <string>

/// @brief UserService是一个本地服务类，使用在RPC服务发布端，提供用户登录功能
/// 它包含一个本地的Login方法用于处理登录逻辑，以及一个重载的Login方法用于处理基于Protobuf的远程过程调用（RPC）
class UserService : public User::UserServiceRPC
{
public:
   bool Login(std::string name, std::string pwd)
   {
      std::cout << "doing local service: Login" << std::endl;
      std::cout << "name:" << name << " pwd:" << pwd << std::endl;
      return true;
   }
   /// @brief 重载的Login方法，处理远程过程调用（RPC）
   void Login(::google::protobuf::RpcController *controller,
              const ::User::LoginRequest *request,
              ::User::LoginResponse *response,
              ::google::protobuf::Closure *done)
   {
      // 这里的request是一个指向LoginRequest对象的指针，包含了远程调用传递的参数
      // 框架将远程调用的参数上报给本地业务Login(name, pwd)
      std::string name = request->name();
      std::string pwd = request->pwd();
      // 调用本地的Login方法处理业务
      bool login_request = Login(name, pwd);
      // 将处理结果封装到response中，处理结果包括错误码、错误信息和返回值
      User::ResultCode *code = response->mutable_result();
      code->set_errcode(0);
      code->set_errmsg("");
      response->set_success(login_request);
      // 调用done回调函数，RPC框架会将response对象序列化后网络发送给远程调用方
      done->Run();
   }
};

int main(int argc, char **argv)
{

   // 调用RpcApplication::Init()来初始化RPC应用程序
   RpcApplication::Init(argc, argv);

   // 将UserService对象注册到RPC节点上, provider是一个RPC网络服务对象
   RpcProvider provider;
   provider.RegisterService(new UserService());
   std::cout << "Register UserService successfully!" << std::endl;

   // 启动一个RPC服务发布节点
   // 进入Run()方法后,进程会进入阻塞状态，等待远程的RPC调用请求
   provider.Run();

   return 0;
}