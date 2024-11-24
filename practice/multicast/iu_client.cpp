#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <liburing.h>
#include <memory>
#include<vector>

#define MULTICAST_GROUP "224.1.1.1"  // 组播地址
#define MULTICAST_PORT 5007          // 目标端口
#define QUEUE_DEPTH 64               // io_uring 队列深度
#define BUFFER_SIZE 1024             // 缓冲区大小

struct IoInfo {
    struct sockaddr_in client_addr;  // 客户端地址
    std::vector<char> buffer;        // 缓冲区
};

int setup_multicast_socket() {
    int sock;
    struct sockaddr_in addr;
    struct ip_mreq mreq;

    // 创建 UDP 套接字
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 绑定到本地地址和端口
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定到所有接口

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return -1;
    }

    // 加入组播组
    inet_pton(AF_INET, MULTICAST_GROUP, &mreq.imr_multiaddr.s_addr);
    mreq.imr_interface.s_addr = INADDR_ANY;  // 使用本机所有接口

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Join multicast group failed");
        close(sock);
        return -1;
    }

    return sock;
}

void handle_receive(IoInfo *info, ssize_t len) {
    if (len < 0) {
        perror("Receive failed");
        return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &info->client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(info->client_addr.sin_port);

    std::cout << "Received " << len << " bytes from "
              << client_ip << ":" << client_port << "\n"
              << "Data: " << std::string(info->buffer.begin(), info->buffer.begin() + len) << "\n";
}

int main() {
    // 初始化组播套接字
    int sock = setup_multicast_socket();
    if (sock < 0)
        return 1;

    // 初始化 io_uring
    struct io_uring ring;
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("Failed to initialize io_uring");
        close(sock);
        return 1;
    }

    // 提交初始请求
    for (int i = 0; i < QUEUE_DEPTH; ++i) {
        auto info = new IoInfo();
        info->buffer.resize(BUFFER_SIZE);
        socklen_t addr_len = sizeof(info->client_addr);

        struct iovec iov = {
            .iov_base = info->buffer.data(),
            .iov_len = BUFFER_SIZE,
        };

        struct msghdr msg = {
            .msg_name = &info->client_addr,
            .msg_namelen = addr_len,
            .msg_iov = &iov,
            .msg_iovlen = 1,
            .msg_control = nullptr,
            .msg_controllen = 0,
            .msg_flags = 0,
        };

        // 获取 SQE
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (!sqe) {
            std::cerr << "Failed to get SQE" << std::endl;
            delete info;
            io_uring_queue_exit(&ring);
            close(sock);
            return 1;
        }

        // 准备 recvmsg 操作
        io_uring_prep_recvmsg(sqe, sock, &msg, 0);
        io_uring_sqe_set_data(sqe, info);
    }

    // 提交请求
    if (io_uring_submit(&ring) < 0) {
        perror("Failed to submit initial requests");
        io_uring_queue_exit(&ring);
        close(sock);
        return 1;
    }

    std::cout << "Client started, listening on: " << MULTICAST_GROUP << ":" << MULTICAST_PORT << std::endl;

    // 事件循环
    while (true) {
        struct io_uring_cqe *cqe;

        // 等待完成事件
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            perror("Error waiting for CQE");
            break;
        }

        // 获取完成的请求信息
        auto *info = static_cast<IoInfo *>(io_uring_cqe_get_data(cqe));
        ssize_t len = cqe->res;

        // 处理接收到的数据
        handle_receive(info, len);

        // 重新提交接收请求
        socklen_t addr_len = sizeof(info->client_addr);
        struct iovec iov = {
            .iov_base = info->buffer.data(),
            .iov_len = BUFFER_SIZE,
        };

        struct msghdr msg = {
            .msg_name = &info->client_addr,
            .msg_namelen = addr_len,
            .msg_iov = &iov,
            .msg_iovlen = 1,
            .msg_control = nullptr,
            .msg_controllen = 0,
            .msg_flags = 0,
        };

        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        if (sqe) {
            io_uring_prep_recvmsg(sqe, sock, &msg, 0);
            io_uring_sqe_set_data(sqe, info);
            io_uring_submit(&ring);
        } else {
            std::cerr << "Failed to get SQE for re-submission" << std::endl;
            delete info;
        }

        // 标记 CQE 为已完成
        io_uring_cqe_seen(&ring, cqe);
    }

    io_uring_queue_exit(&ring);
    close(sock);
    return 0;
}
