#ifndef _rpc_config_H
#define _rpc_config_H

#include <string>
#include <unordered_map>
// RPC配置
class RPCconfig
{
public:
    // 加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查找key对应的value
    std::string Load(const std::string &key);

private:
    // 配置map
    std::unordered_map<std::string, std::string> config_map;
    // 去掉字符串前后的空格
    void Trim(std::string &read_buf);
};

#endif