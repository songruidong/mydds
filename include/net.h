#ifndef __NET_H__
#define __NET_H__

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <cstring>      // For memset
#include <arpa/inet.h>  // For sockaddr_in, htons, inet_addr
#include <unistd.h>     // For close()

class Publisher {
public:
    Publisher(int port) : port_(port), stop_(false) {
        // 初始化 socket
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket_ < 0) {
            throw std::runtime_error("Failed to create UDP socket.");
        }

        // 绑定到指定端口
        sockaddr_in addr{};
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(udp_socket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(udp_socket_);
            throw std::runtime_error("Failed to bind UDP socket to port.");
        }

        // 启动接收线程
        receiver_thread_ = std::thread(&Publisher::receiveLoop, this);
    }

    ~Publisher() {
        stop();
    }

    // 发送消息
    void sendMessage(const std::string& message, const std::string& ip, int port) {
        sockaddr_in target_addr{};
        memset(&target_addr, 0, sizeof(target_addr));
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &target_addr.sin_addr);

        ssize_t sent_bytes = sendto(udp_socket_, message.c_str(), message.size(), 0,
                                    (struct sockaddr*)&target_addr, sizeof(target_addr));
        if (sent_bytes < 0) {
            std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
        }
    }

    // 停止接收线程并关闭 socket
    void stop() {
        if (!stop_) {
            stop_ = true;
            if (receiver_thread_.joinable()) {
                receiver_thread_.join();
            }
            close(udp_socket_);
        }
    }

private:
    int udp_socket_;              // UDP 套接字
    int port_;                    // 固定端口
    std::atomic<bool> stop_;      // 停止标志
    std::thread receiver_thread_; // 接收线程

    // 接收循环
    void receiveLoop() {
        char buffer[1024];
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);

        while (!stop_) {  
            memset(buffer, 0, sizeof(buffer));
            ssize_t received_bytes = recvfrom(udp_socket_, buffer, sizeof(buffer) - 1, 0,
                                              (struct sockaddr*)&sender_addr, &sender_len);
            if (received_bytes > 0) {
                buffer[received_bytes] = '\0';
                std::string sender_ip = inet_ntoa(sender_addr.sin_addr);
                int sender_port = ntohs(sender_addr.sin_port);

                std::cout << "Received message from " << sender_ip << ":" << sender_port
                          << " -> " << buffer << std::endl;
            } else if (received_bytes < 0 && errno != EINTR) {
                std::cerr << "Failed to receive message: " << strerror(errno) << std::endl;
            }
        }
    }
};

#endif // __NET_H__