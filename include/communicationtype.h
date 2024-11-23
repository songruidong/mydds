#ifndef __COMMUNICATIONTYPE_H__
#define __COMMUNICATIONTYPE_H__

#include <string>
// 定义一个枚举类
enum class CommunicationType
{
    Discover  = 0,  // 表示发现类型
    Publish   = 1,  // 表示发布类型
    Subscribe = 2      // 表示订阅类型
};

// 将枚举类型转换为字符串
std::string toString(CommunicationType type);
#endif  // __COMMUNICATIONTYPE_H__