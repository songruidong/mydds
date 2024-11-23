#ifndef __DOMAINPARTICIPANT_H__
#define __DOMAINPARTICIPANT_H__

#include <liburing.h>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include "address.h"
#include "communicationtype.h"
#include "packet.h"
#include "publisher.h"
#include "subscriber.h"
#include "topic.hpp"
#include "topicmanager.h"

class Node
{
  public:
    explicit Node(int domain_id) : domain_id_(domain_id)
    {
        topic_manager_ = std::make_shared<TopicManager>();

        // this->unicast_address = std::make_shared<UnicastAddress>("127.0.0.1",8401);确定本机地址
    }
    void init()
    {;
        if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }
        if (init_udp_socket() < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }

    }
    int init_udp_socket()
    {
        int &sock = this->udp_socket;
        // 创建 UDP 套接字
        struct sockaddr_in addr;
        int yes = 1;

        // 创建 UDP 套接字
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            std::cerr << "Socket creation failed!" << std::endl;
            return -1;
        }

        // 设置套接字选项，允许重用地址
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        {
            std::cerr << "Setting SO_REUSEADDR failed!" << std::endl;
            return -1;
        }

        // 绑定到指定的本地端口
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family      = AF_INET;
        local_addr.sin_port        = htons(LOCAL_PORT);  // 设置本地端口
        local_addr.sin_addr.s_addr = INADDR_ANY;         // 绑定到所有接口

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
        {
            std::cerr << "Bind failed!" << std::endl;
            return -1;
        }
        return 0;
    }

    int multiacast(const std::vector<uint8_t> &bytes) 
    {
        // 设置目标地址
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port   = htons(MULTICAST_PORT);
        if (inet_pton(AF_INET, this->MULTICAST_GROUP, &dest_addr.sin_addr) <= 0)
        {
            return -1;
        }
        // 获取一个 Submission Queue Entry (SQE)
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            // close(udp_socket);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("Failed to get SQE");
        }
        iovec iov;
        iov.iov_base = (void *)bytes.data();
        iov.iov_len  = bytes.size();

        msghdr msg      = {};
        msg.msg_name    = &dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;
        // 准备 sendmsg 操作
        io_uring_prep_sendmsg(sqe, udp_socket, &msg, 0);

        // 提交请求
        if (io_uring_submit(&ring) < 0)
        {
            // close(udp_socket);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("Failed to submit sendmsg request");
        }
        return 0;
    }
    bool create(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        auto ret = topic_manager_->insertTopic(topic_name, topic);
        return ret.get() == nullptr;
    }
    bool publish(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        auto ret = topic_manager_->getTopics(topic_name);
        if (ret.size() != 1)
        {
            return false;
        }
        ret[0] = topic;
        // 发送消息
        auto data = DDSPacket::create_publish_packet(topic);
        multiacast(data);
        return true;
    }
    bool subscribe(const std::string &topic_name)
    {
        auto ret = topic_manager_->getTopics(topic_name);
        if (ret.size() == 0)
        {
            return false;
        }
        auto data = DDSPacket::create_subscribe_packet(ret);
        // 发送消息
        return true;
    }

  private:
    int domain_id_;  // Domain ID
    std::shared_ptr<TopicManager> topic_manager_;
    // MulticastAddress multicast_address;
    // UnicastAddress unicast_address;
    struct io_uring ring;
    static constexpr int QUEUE_DEPTH = 1024;
    int udp_socket;
    // int unicast_socket;
    const char *MULTICAST_GROUP = "224.1.1.1";  // 目标组播地址
    const int MULTICAST_PORT    = 5007;         // 目标端口
    const int LOCAL_PORT        = 12345;        // 本地端口（固定发送端口）
};

#endif  // __DOMAINPARTICIPANT_H__