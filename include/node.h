#ifndef __DOMAINPARTICIPANT_H__
#define __DOMAINPARTICIPANT_H__
#include <liburing.h>
#include <sys/socket.h>

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
#include "publisherinfo.h"
#include "spdlog/spdlog.h"
#include "topic.hpp"
#include "topicmanager.h"
#include "trie.h"
#include "utils.h"
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
        struct io_uring_params params;
        memset(&params, 0, sizeof(params));
        params.flags |= IORING_SETUP_SQPOLL;
        params.sq_thread_idle = 2000;
        if (io_uring_queue_init_params(QUEUE_DEPTH, &ring, &params) < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }
        if (init_udp_socket() < 0)
        {
            throw std::runtime_error("Failed to initialize io_uring");
        }
        this->init_timer();
        // this->tick();
        this->submit_receive_request(this->udp_socket);
        // 启动运行线程
        running_ = true;
        worker_thread_.reset(new std::thread(&Node::run, this));
    }
    void init_timer(unsigned timeout_ms = DISCOVERTIME)
    {
        ts.tv_sec  = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;
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
        local_addr.sin_port        = htons(MULTICAST_PORT);  // 设置本地端口
        local_addr.sin_addr.s_addr = INADDR_ANY;             // 绑定到所有接口

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
        {
            std::cerr << "Bind failed!" << std::endl;
            return -1;
        }
        // 加入组播组
        struct ip_mreq mreq;
        inet_pton(AF_INET, MULTICAST_GROUP, &mreq.imr_multiaddr.s_addr);
        mreq.imr_interface.s_addr = INADDR_ANY;  // 使用本机所有接口

        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            std::cerr << "Join multicast group failed!" << std::endl;
            return -1;
        }
        return 0;
    }

    bool create_topic(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        auto ret = topic_manager_->create_topic(topic_name, topic);
        return ret;
    }
    bool create_publish_topic(const std::string &topic_name, std::shared_ptr<Topic> topic = nullptr)
    {
        auto ret = topic_manager_->create_publishTopic(topic_name, topic);
        return ret;
    }
    bool create_subscribe_topic(const std::string &topic_name, std::shared_ptr<Topic> topic)
    {
        // auto ret = topic_manager_->create_subscribeTopic(topic_name)
        return true;
    }
    void handler_read(IoInfo *info)
    {
        auto buffer = info->recv_buffer;
        DDSPacket packet;
        SPDLOG_INFO("receive packet size:{}", buffer.size());
        packet.Unpack(buffer.begin(), buffer.end());
        switch (static_cast<CommunicationType>(packet.header.type))
        {
            case CommunicationType::Discover:
                for (auto &topicname : std::get<DiscoverData>(packet.DDSData).data)
                {
                    std::shared_ptr<Topic> topic_ptr = std::make_shared<Topic>(topicname.name);
                    auto addr                        = Util::sockaddr_to_ip_port(*info->dest_addr);
                    topic_ptr->publisherinfos.push_back({std::move(addr.first), addr.second});
                    // topic_ptr->publisherinfos.emplace_back(Util::sockaddr_to_ip_port(*info->dest_addr));
                    create_topic(topicname.name, topic_ptr);
                    submit_receive_request(udp_socket);
                }
                break;
            case CommunicationType::Publish:
                break;
            case CommunicationType::Subscribe:
                break;
        }
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
        subscribedata = DDSPacket::create_subscribe_packet(topics);
        multiacast(subscribedata);

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
        // 设置接收信息的结构体
        IoInfo *info = new IoInfo;
        info->type   = EventType::Read;
        info->dest_addr.reset(new struct sockaddr_in);
        socklen_t client_addr_len = sizeof(*(info->dest_addr));
        info->recv_buffer.resize(IoInfo::BUFFER_SIZE);
        // 创建 iovec 和 msghdr
        iovec *iov    = new iovec;
        iov->iov_base = info->recv_buffer.data();  // 设置接收缓冲区
        iov->iov_len  = IoInfo::BUFFER_SIZE;

        msghdr *msg         = new msghdr{};
        msg->msg_name       = info->dest_addr.get();  // 设置目标地址
        msg->msg_namelen    = client_addr_len;        // 地址长度
        msg->msg_iov        = iov;                    // 数据缓冲区
        msg->msg_iovlen     = 1;                      // 单个缓冲区
        msg->msg_control    = nullptr;                // 无额外控制消息
        msg->msg_controllen = 0;
        msg->msg_flags      = 0;

        // 将 msg 和 iov 保存到 info 中
        info->msg_ptr.reset(msg);
        info->iov_ptr.reset(iov);

        // 获取一个 Submission Queue Entry (SQE)
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            io_uring_queue_exit(&ring);
            throw std::runtime_error("Failed to get SQE");
        }

        // 准备 recvmsg 操作
        io_uring_prep_recvmsg(sqe, udp_socket, info->msg_ptr.get(), 0);
        io_uring_sqe_set_data(sqe, info);

        // // 提交请求
        // if (io_uring_submit(&ring) < 0)
        // {
        //     io_uring_queue_exit(&ring);
        //     throw std::runtime_error("Failed to submit recvmsg request");
        // }

        return 0;
    }

    void set_timer()
    {

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

        // // 提交定时器任务
        // if (io_uring_submit(&ring) < 0)
        // {
        //     throw std::runtime_error("Failed to submit timeout request");
        // }
    }
    int multiacast(const std::vector<uint8_t> &bytes)
    {
        // 设置目标地址
        IoInfo *info = new IoInfo;
        info->type   = EventType::Multicast;
        info->dest_addr.reset(new struct sockaddr_in);
        info->dest_addr->sin_family = AF_INET;
        info->dest_addr->sin_port   = htons(MULTICAST_PORT);
        info->bytes                 = std::move(bytes);

        iovec *iov       = new iovec;
        iov->iov_base    = (void *)info->bytes.data();
        iov->iov_len     = bytes.size();
        msghdr *msg      = new msghdr{};
        msg->msg_name    = info->dest_addr.get();
        msg->msg_namelen = sizeof(*(info->dest_addr));
        msg->msg_iov     = iov;
        msg->msg_iovlen  = 1;
        info->msg_ptr.reset(msg);
        info->iov_ptr.reset(iov);

        // 准备发送数据
        // 设置目标地址
        if (inet_pton(AF_INET, this->MULTICAST_GROUP, &info->dest_addr->sin_addr) <= 0)
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

        // 准备 sendmsg 操作
        io_uring_prep_sendmsg(sqe, this->udp_socket, info->msg_ptr.get(), 0);
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
    // int unicast(const std::vector<uint8_t> &bytes, std::string_view &ipaddr, int port)
    // {
    //     // 设置目标地址
    //     struct sockaddr_in dest_addr;
    //     dest_addr.sin_family = AF_INET;
    //     dest_addr.sin_port   = htons(port);
    //     if (inet_pton(AF_INET, ipaddr.data(), &dest_addr.sin_addr) <= 0)
    //     {
    //         return -1;
    //     }
    //     // 获取一个 Submission Queue Entry (SQE)
    //     struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    //     if (!sqe)
    //     {
    //         // close(udp_socket);
    //         io_uring_queue_exit(&ring);
    //         throw std::runtime_error("Failed to get SQE");
    //     }
    //     iovec iov;
    //     iov.iov_base = (void *)bytes.data();
    //     iov.iov_len  = bytes.size();

    //     msghdr msg      = {};
    //     msg.msg_name    = &dest_addr;
    //     msg.msg_namelen = sizeof(dest_addr);
    //     msg.msg_iov     = &iov;
    //     msg.msg_iovlen  = 1;
    //     // 准备 sendmsg 操作
    //     io_uring_prep_sendmsg(sqe, udp_socket, &msg, 0);
    //     IoInfo *info = new IoInfo;
    //     info->type   = EventType::Read;
    //     io_uring_sqe_set_data(sqe, info);
    //     // 提交请求
    //     if (io_uring_submit(&ring) < 0)
    //     {
    //         // close(udp_socket);
    //         io_uring_queue_exit(&ring);
    //         throw std::runtime_error("Failed to submit sendmsg request");
    //     }
    //     return 0;
    // }
    void tick()
    {
        discovery();
        set_timer();
    }
    void run()
    {
        while (running_)
        {
            struct io_uring_cqe *cqe;
            int ret = io_uring_submit_and_wait(&ring, 1);

            unsigned head;
            unsigned count = 0;
            // 批量处理 CQEs
            io_uring_for_each_cqe(&ring, head, cqe)
            {
                // 处理 CQE
                count++;
                SPDLOG_INFO("cq");
                // 处理完成的请求
                if (cqe->res < 0)
                {
                    // SPDLOG_INFO("case -ETIME:");
                    switch (cqe->res)
                    {
                        case -ETIME:
                        {
                            // SPDLOG_INFO("case -ETIME:");
                            break;
                        }
                        default:
                        {
                            // std::cerr << "Request failed: " << strerror(-cqe->res) << std::endl;
                            SPDLOG_INFO("Request failed: {}", strerror(-cqe->res));
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
                        SPDLOG_INFO("case EventType::Multicast:");
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
                        SPDLOG_INFO("case EventType::Read:");
                        handler_read((IoInfo *)cqe->user_data);
                        // std::cout << "Read event" << std::endl;
                        break;
                    }
                    case EventType::Timer:
                    {
                        SPDLOG_INFO("case EventType::Timer:");
                        tick();
                        break;
                    }
                    default:
                    {
                        SPDLOG_INFO("default");
                    }
                }
                delete (IoInfo *)cqe->user_data;
                // std::cout << "Request completed successfully, res=" << cqe->res << std::endl;
            }
            io_uring_cq_advance(&ring, count);
            SPDLOG_INFO("{}", count);
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

    std::unique_ptr<std::thread> worker_thread_;  // 用于处理 IO 的线程
    std::atomic<bool> running_;                   // 控制线程运行状态

    struct sockaddr_in opposite_addr;  // 对方的地址信息

    struct __kernel_timespec ts;
    static constexpr int DISCOVERTIME = 1000;
    std::vector<uint8_t> discoverdata;
    std::vector<uint8_t> publishdata;
    std::vector<uint8_t> subscribedata;
};

#endif  // __DOMAINPARTICIPANT_H__