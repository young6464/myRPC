#ifndef _rpc_controller_H
#define _rpc_controller_H

#include <string>
#include <google/protobuf/service.h>
// 用于描述RPC调用的控制器
// 主要作用为跟踪RPC方法的调用状态、错误信息并提供控制功能（如取消调用）
class RPCcontroller :pubilc google::protobuf::RpcController
{
private:
    // RPC方法执行过程中的状态
    bool m_failed;
    // RPC方法执行过程中的错误信息
    std::string m_errText;

public:
    // 
    RPCcontroller();
    // 重置
    void Reset();
    // 调用失败
    bool Failed() const;
    // 调用错误信息
    std::string ErrorText() const;
    // 
    void SetFailed(const std::string &reason);

    // 目前未实现具体的功能
    void StartCancel();
    bool IsCancel() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
};

#endif