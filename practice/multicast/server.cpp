#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MULTICAST_GROUP "224.1.1.1"  // 组播地址
#define MULTICAST_PORT 5007          // 目标端口
#define LOCAL_PORT 12345             // 本地端口（固定发送端口）

int main() {
    int sock;
    struct sockaddr_in addr;
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

    // 设置目标地址（组播地址）
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);
    inet_pton(AF_INET, MULTICAST_GROUP, &addr.sin_addr);

    // 发送消息
    const char *message = "Hello, Multicast from Server with fixed port!";
    while (true) {
        if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            std::cerr << "Send failed!" << std::endl;
            return -1;
        }
        std::cout << "Sent: " << message << std::endl;
        sleep(2);  // 每隔 2 秒发送一次消息
    }

    close(sock);
    return 0;
}
