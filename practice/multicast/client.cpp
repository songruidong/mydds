#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MULTICAST_GROUP "224.1.1.1"  // 组播地址
#define MULTICAST_PORT 5007          // 目标端口

int main() {
    int sock;
    struct sockaddr_in addr;
    struct ip_mreq mreq;

    // 创建 UDP 套接字
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    // 绑定到本地地址和端口
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定到所有接口

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        return -1;
    }

    // 加入组播组
    inet_pton(AF_INET, MULTICAST_GROUP, &mreq.imr_multiaddr.s_addr);
    mreq.imr_interface.s_addr = INADDR_ANY;  // 使用本机所有接口

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        std::cerr << "Join multicast group failed!" << std::endl;
        return -1;
    }

    std::cout << "Client started, listening on: " << MULTICAST_GROUP << ":" << MULTICAST_PORT << std::endl;

    // 接收消息
    char buffer[1024];
    while (true) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
        if (len < 0) {
            std::cerr << "Receive failed!" << std::endl;
            return -1;
        }
        buffer[len] = '\0';  // 确保字符串结束
        std::cout << "Received message: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
