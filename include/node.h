#ifndef __DOMAINPARTICIPANT_H__
#define __DOMAINPARTICIPANT_H__
#include <liburing.h>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <vector>
#include "address.h"
#include "communicationtype.h"
#include "ioinfo.h"
#include "packet.h"
#include "spdlog/spdlog.h"
#include "topic.hpp"
#include "topicmanager.h"
#include "trie.h"
class Node
{
  public:
    explicit Node(int domain_id = 0) : domain_id_(domain_id)
    {
        topic_manager_ = std::make_shared<TopicManager>();

        // this->unicast_address = std::make_shared<UnicastAddress>("127.0.0.1",8401);确定本机地址
    }
    void init()
    {
        if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }
        if (init_udp_socket() < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }
        this->tick();
        // 启动运行线程
        running_       = true;
        worker_thread_ = std::thread(&Node::run, this);
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
        IoInfo info;
        info.type = EventType::Multicast;
        io_uring_sqe_set_data(sqe, &info);
        // 提交请求
        if (io_uring_submit(&ring) < 0)
        {
            // close(udp_socket);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("Failed to submit sendmsg request");
        }
        return 0;
    }
    int unicast(const std::vector<uint8_t> &bytes, std::string_view &ipaddr, int port)
    {
        // 设置目标地址
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port   = htons(port);
        if (inet_pton(AF_INET, ipaddr.data(), &dest_addr.sin_addr) <= 0)
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
        IoInfo *info = new IoInfo;
        info->type   = EventType::Read;
        io_uring_sqe_set_data(sqe, info);
        // // 提交请求
        // if (io_uring_submit(&ring) < 0)
        // {
        //     // close(udp_socket);
        //     io_uring_queue_exit(&ring);
        //     throw std::runtime_error("Failed to submit sendmsg request");
        // }
        return 0;
    }
    bool create_publish_topic(const std::string &topic_name, std::shared_ptr<Topic> topic = nullptr)
    {
        auto ret = topic_manager_->create_publishTopic(topic_name,topic);
        return ret;
    }
    bool create_subscribe_topic(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        // auto ret = topic_manager_->create_subscribeTopic(topic_name)
        return true;
    }
    bool publish(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        auto ret = topic_manager_->getTopics(topic_name);
        if (ret.size() != 1)
        {
            return false;
        }
        ret[0] = topic;
        // TODO 发送发布消息，多播还是单播
        auto data = DDSPacket::create_publish_packet({topic});
        multiacast(data);
        return true;
    }
    bool subscribe(const std::string &topic_name)
    {
        auto topics = topic_manager_->getTopics(topic_name);
        if (topics.size() == 0)
        {
            return false;
        }
        //广播订阅消息
        auto data = DDSPacket::create_subscribe_packet(topics);
        multiacast(data);

        return true;
    }
    bool discovery()
    {
        auto topics = topic_manager_->get_all_publishTopics();
        if (topics.size() == 0)
        {
            return false;
        }
        auto data = DDSPacket::create_discover_packet(topics);
        multiacast(data);
        return true;
    }
    int submit_receive_request(int udp_socket)
    {
        // 获取一个 Submission Queue Entry (SQE)
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            std::cerr << "Failed to get SQE" << std::endl;
            return -1;
        }

        // 设置接收缓冲区和目标地址
        struct iovec iov = {
            .iov_base = recv_buffer.data(),  // 接收缓冲区,
            .iov_len  = BUFFER_SIZE,
        };

        socklen_t client_addr_len = sizeof(this->opposite_addr);

        struct msghdr msg = {
            .msg_name       = &this->opposite_addr,         // 目标地址
            .msg_namelen    = sizeof(this->opposite_addr),  // 地址长度
            .msg_iov        = &iov,                         // 数据缓冲区
            .msg_iovlen     = 1,                            // 单个缓冲区
            .msg_control    = nullptr,                      // 无额外控制消息
            .msg_controllen = 0,
            .msg_flags      = 0,
        };

        // 准备 recvmsg 操作
        io_uring_prep_recvmsg(sqe, this->udp_socket, &msg, 0);
        IoInfo *info = new IoInfo;
        info->type   = EventType::Read;
        io_uring_sqe_set_data(sqe, info);

        // // 提交请求
        // io_uring_submit(&ring);
        return 0;
    }
    void set_timer(unsigned timeout_ms)
    {
        struct __kernel_timespec ts = {};
        ts.tv_sec                   = timeout_ms / 1000;
        ts.tv_nsec                  = (timeout_ms % 1000) * 1000000;

        // 获取 SQE 并设置为定时器请求
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            throw std::runtime_error("Failed to get SQE");
        }
        io_uring_prep_timeout(sqe, &ts, 0, 0);
        IoInfo *info = new IoInfo;
        info->type   = EventType::Timer;
        io_uring_sqe_set_data(sqe, info);
        // 提交定时器任务
        if (io_uring_submit(&ring) < 0)
        {
            throw std::runtime_error("Failed to submit timeout request");
        }
    }
    void tick()
    {
        discovery();
        set_timer(DISCOVERTIME);
    }
    void run()
    {
        while (running_)
        {
            struct io_uring_cqe *cqe;
            // spdlog::info("1");
            int ret = io_uring_submit_and_wait(&ring, 0);
            // int ret = io_uring_wait_cqe(&ring, &cqe);
            // spdlog::info("2");
            // if (ret < 0)
            // {
            //     std::cerr << "io_uring_wait_cqe failed: " << strerror(-ret) << std::endl;
            // }

            unsigned head;
            unsigned count = 0;
            // 批量处理 CQEs
            io_uring_for_each_cqe(&ring, head, cqe)
            {
                // 处理 CQE
                count++;
                std::cout << "count:" << count << std::endl;
                // 处理完成的请求
                if (cqe->res < 0)
                {
                    // spdlog::info("case -ETIME:");
                    switch (cqe->res)
                    {
                        case -ETIME:
                        {
                            spdlog::info("case -ETIME:");
                            break;
                        }
                        default:
                        {
                            // std::cerr << "Request failed: " << strerror(-cqe->res) << std::endl;
                            spdlog::info("Request failed: {}", strerror(-cqe->res));
                            continue;
                        }
                    }
                }
                // std::cout << "count2:" << count << std::endl;
                // 自定义逻辑，处理 CQE
                switch (((IoInfo *)cqe->user_data)->type)
                {
                    case EventType::Multicast:
                    {
                        // 处理 Multicast 事件
                        // std::cout << "Multicast event" << std::endl;
                        spdlog::info("case EventType::Multicast:");
                        break;
                    }
                    case EventType::Unicast:
                    {
                        // 处理 Unicast 事件
                        // std::cout << "Unicast event" << std::endl;
                        break;
                    }
                    case EventType::Read:
                    {
                        // 处理 Read 事件
                        // std::cout << "Read event" << std::endl;

                        break;
                    }
                    case EventType::Timer:
                    {
                        spdlog::info("case EventType::Timer:");
                        tick();
                        break;
                    }
                    default:{
                        spdlog::info("default");
                    }
                }
                // std::cout << "Request completed successfully, res=" << cqe->res << std::endl;
            }
            io_uring_cq_advance(&ring, count);
            // 标记 CQE 已完成
            // io_uring_cqe_seen(&ring, cqe);
        }
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

    std::thread worker_thread_;  // 用于处理 IO 的线程
    std::atomic<bool> running_;  // 控制线程运行状态

    struct sockaddr_in opposite_addr;  // 对方的地址信息
    static constexpr int BUFFER_SIZE = 2048;
    std::array<uint8_t, BUFFER_SIZE> recv_buffer;

    static constexpr int DISCOVERTIME = 1000;
};

#endif  // __DOMAINPARTICIPANT_H__