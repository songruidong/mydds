import socket
import struct

# 配置组播地址和端口
MULTICAST_GROUP = '224.1.1.1'  # 组播地址
MULTICAST_PORT = 5007          # 端口号

# 创建UDP套接字
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# 绑定到所有接口
sock.bind(('', MULTICAST_PORT))

# 组播地址加入操作
mreq = struct.pack('4sL', socket.inet_aton(MULTICAST_GROUP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print(f"Listening for messages on {MULTICAST_GROUP}:{MULTICAST_PORT}")

while True:
    try:
        # 接收数据
        data, addr = sock.recvfrom(1024)
        print(f"Received message from {addr}: {data.decode()}")
    except KeyboardInterrupt:
        print("Client stopped.")
        break

sock.close()
