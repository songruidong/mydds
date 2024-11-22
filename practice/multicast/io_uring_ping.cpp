#include <arpa/inet.h>
#include <liburing.h>
#include <netinet/in.h>
#include <iostream>
int main()
{
    constexpr int QUEUE_DEPTH = 8;  // io_uring 队列深度
    io_uring ring;
    struct sockaddr_in dest_addr
    {};

    // 初始化 io_uring
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0)
    {
        throw std::runtime_error("io_uring_queue_init failed");
    }

    // 创建 UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    // 设置目标地址
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port   = htons(5007);
    if (inet_pton(AF_INET, "224.1.1.1", &dest_addr.sin_addr) <= 0)
    {
        close(sockfd);
        throw std::runtime_error("Invalid IP address");
    }

    while (true)
    {  // 准备消息
        std::string message = "xxxxxxxxxxxxxxxx";
        iovec iov;
        iov.iov_base = (void *)message.c_str();
        iov.iov_len  = message.size();

        msghdr msg      = {};
        msg.msg_name    = &dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;

        // 获取一个提交队列项
        io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            close(sockfd);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("Failed to get SQE");
        }

        // 准备 sendmsg 请求
        io_uring_prep_sendmsg(sqe, sockfd, &msg, 0);

        // 提交请求
        if (io_uring_submit(&ring) < 0)
        {
            close(sockfd);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("io_uring_submit failed");
        }

        // 等待完成队列项
        io_uring_cqe *cqe;
        if (io_uring_wait_cqe(&ring, &cqe) < 0)
        {
            close(sockfd);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("io_uring_wait_cqe failed");
        }

        // 检查结果
        if (cqe->res < 0)
        {
            close(sockfd);
            io_uring_cqe_seen(&ring, cqe);
            io_uring_queue_exit(&ring);
            throw std::runtime_error("sendmsg failed with error");
        }

        std::cout << "Message sent successfully, bytes: " << cqe->res << std::endl;

        // 标记 CQE 已处理
        io_uring_cqe_seen(&ring, cqe);
    }

    // 清理资源
    close(sockfd);
    io_uring_queue_exit(&ring);
}
