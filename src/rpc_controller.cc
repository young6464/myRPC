#include "rpc_controller.h"

RPCcontroller::RPCcontroller()
{
    m_failed = false;
    m_errText = "";
}

void RPCcontroller::Reset()
{
    m_failed = false;
    m_errText = "";
}

bool RPCcontroller::Failed() const
{
    return m_failed;
}

std::string RPCcontroller::ErrorText() const
{
    return m_errText;
}

void RPCcontroller::SetFailed(const std::string &reason)
{
    m_failed = true;
    m_errText = reason;
}

// @todo RPC服务端提供的取消功能
void RPCcontroller::StartCancel()
{
}

bool RPCcontroller::IsCanceled() const
{
    return false;
}

void RPCcontroller::NotifyOnCancel(google::protobuf::Closure *callback)
{
}