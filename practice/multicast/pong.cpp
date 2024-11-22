#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MULTICAST_GROUP "224.1.1.1"  // 组播地址
#define MULTICAST_PORT 5007          // 组播端口
#define LOCAL_PORT 12345             // 本地端口（接收端口）
int create_send_socker()
{
    int sock;
    int yes = 1;
       // 创建 UDP 套接字
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    // 设置套接字选项，允许重用地址
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        std::cerr << "Setting SO_REUSEADDR failed!" << std::endl;
        return -1;
    }

    // 绑定到指定的本地端口
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LOCAL_PORT);  // 设置本地端口
    local_addr.sin_addr.s_addr = INADDR_ANY;  // 绑定到所有接口

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        return -1;
    }
    return sock;
}
int main() {
    int sock;
    struct sockaddr_in addr;
    struct ip_mreq mreq;

    // 创建 UDP 套接字
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    // 绑定到指定的本地端口
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);  // 设置本地端口
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 绑定到所有接口 // 绑定到所有接口

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        return -1;
    }

    // 加入组播组
    inet_pton(AF_INET, MULTICAST_GROUP, &mreq.imr_multiaddr.s_addr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  // 使用本机所有接口

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        std::cerr << "Join multicast group failed!" << std::endl;
        return -1;
    }

    std::cout << "Client started, listening on: " << MULTICAST_GROUP << ":" << MULTICAST_PORT << std::endl;

    // 接收消息并发送单播
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];
    // int sendsock = create_send_socker();
    while (true) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) {
            std::cerr << "Receive failed!" << std::endl;
            return -1;
        }
        buffer[len] = '\0';  // 确保字符串结束
            // 打印客户端的 IP 地址和端口号
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        uint16_t client_port = ntohs(client_addr.sin_port);  // 转换端口号

        std::cout << "Client IP: " << client_ip << ", Port: " << client_port <<"    ";
        std::cout << "Received: " << buffer << std::endl;

        // 向客户端发送单播消息
        const char *reply_msg = "Pong from client!";
        if (sendto(sock, reply_msg, strlen(reply_msg), 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
            std::cerr << "Send failed!" << std::endl;
            return -1;
        }
        std::cout << "Sent: " << reply_msg << std::endl;
    }

    close(sock);
    return 0;
}
