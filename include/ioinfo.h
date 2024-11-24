#ifndef __IOINFO_H__
#define __IOINFO_H__

#include <memory>
#include <string>
#include <vector>
#include <netinet/in.h>
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
    static constexpr int BUFFER_SIZE = 2048;
    EventType type;
    std::unique_ptr<struct sockaddr_in> dest_addr;
    std::unique_ptr<msghdr> msg_ptr;
    std::unique_ptr<iovec> iov_ptr;
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> recv_buffer;
};
#endif  // __IOINFO_H__