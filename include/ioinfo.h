#ifndef __IOINFO_H__
#define __IOINFO_H__

#include <string>
// 定义一个枚举类
enum class EventType
{
    Multicast = 0,  // 组播发送
    Unicast   = 1,  //
    Read      = 2,  // 表示订阅类型
    Timer     = 3,
};
// 将枚举类型转换为字符串

struct IoInfo
{
    EventType type;
};
#endif  // __IOINFO_H__