#include "rpc_config.h"
#include <memory>

// 加载配置文件
void RPCconfig::LoadConfigFile(const char *config_file)
{
    // 使用智能指针打开文件，确保文件在使用后自动关闭
    std::unique_ptr<FILE, decltype(&fclose)> pf(fopen(config_file, "r"),
                                                &fclose);
    if (pf == nullptr)
    {
        exit(EXIT_FAILURE);
    }
    char buf[1024];
    // fgets函数逐行读取文件内容
    while (fgets(buf, 1024, pf.get()) != nullptr)
    {
        std::string read_buf(buf);
        Trim(read_buf);
        // 跳过空行和注释行
        if (read_buf[0] == '#' || read_buf.empty()) continue;
        // 如果在单行中没有找到等号，便跳过
        int index = read_buf.find('=');
        if (index == -1) continue;
        // 找到key值
        std::string key = read_buf.substr(0, index);
        Trim(key);
        //找到value值，-1目的是排除行尾\n
        int endindex = read_buf.find('\n', index);
        std::string value = read_buf.substr(index + 1, endindex - index - 1);
        Trim(value);
        // 将键值对插入到配置映射中
        config_map.insert({key, value});
    }
}

// 查找key对应的value
std::string RPCconfig::Load(const std::string &key)
{
    auto it = config_map.find(key);
    if (it == config_map.end())
    {
        return "";
    }
    return it->second;
}

// 去掉字符串前后的空格
void RPCconfig::Trim(std::string &read_buf)
{
    // 找到第一个非空格字符的位置
    int index = read_buf.find_first_not_of(' ');
    if (index != -1)
    {
        read_buf = read_buf.substr(index, read_buf.size() - index);
    }
    // 找到最后一个非空格字符的位置
    index = read_buf.find_last_not_of(' ');
    if (index != -1)
    {
        read_buf = read_buf.substr(0, index + 1);
    }
}