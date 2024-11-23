#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <regex>
#include "packet.h"
#include <arpa/inet.h>  // For inet_pton() and inet_ntoa()
#include <netinet/in.h>  // For sockaddr_in and sockaddr_in6
// 单播地址类
class UnicastAddress {
private:
    std::string address; // 存储单播地址
    uint16_t port;      // 存储端口号

public:
    // 构造函数
    UnicastAddress(const std::string& addr, uint16_t p) : address(addr), port(p) {}
    UnicastAddress()  = default;
    // 设置单播地址和端口
    void setAddress(const std::string& addr, uint16_t p) {
        address = addr;
        port = p;
    }

    // 获取单播地址
    std::string getAddress() const {
        return address;
    }

    // 获取端口号
    uint16_t getPort() const {
        return port;
    }

    // 打印单播地址和端口
    void print() const {
        std::cout << "Unicast Address: " << address << ":" << port << std::endl;
    }

    // 验证地址格式（IPv4）
    bool isValid() const {
        struct sockaddr_in sa;
        return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
    }

    // 转换为 sockaddr_in 结构
    struct sockaddr_in toSockAddr() const {
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &(sa.sin_addr));
        sa.sin_port = htons(port);
        return sa;
    }
};

// 多播地址类
class MulticastAddress {
private:
    std::string address; // 存储多播地址
    uint16_t port;      // 存储端口号

public:
    // 构造函数
    MulticastAddress(const std::string& addr, uint16_t p) : address(addr), port(p) {}
    MulticastAddress()  = default;
    // 设置多播地址和端口
    void setAddress(const std::string& addr, uint16_t p) {
        address = addr;
        port = p;
    }

    // 获取多播地址
    std::string getAddress() const {
        return address;
    }

    // 获取端口号
    uint16_t getPort() const {
        return port;
    }

    // 打印多播地址和端口
    void print() const {
        std::cout << "Multicast Address: " << address << ":" << port << std::endl;
    }

    // 验证地址格式（IPv4）
    bool isValid() const {
        struct sockaddr_in sa;
        return inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0;
    }

    // 转换为 sockaddr_in 结构
    struct sockaddr_in toSockAddr() const {
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &(sa.sin_addr));
        sa.sin_port = htons(port);
        return sa;
    }
};

// // 端口和地址转换为字符串
// std::string addressToString(const std::string& address, uint16_t port) {
//     std::ostringstream oss;
//     oss << address << ":" << port;
//     return oss.str();
// }
#endif // __ADDRESS_H__