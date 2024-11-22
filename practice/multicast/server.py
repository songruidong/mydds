import socket
import struct
import time

# 配置组播地址和端口
MULTICAST_GROUP = '224.1.1.1'  # 组播地址
MULTICAST_PORT = 5007          # 端口号

# 创建UDP套接字
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# 设置TTL (Time-To-Live)，确定组播的生存时间
ttl = struct.pack('b', 1)  # 只在本地网络中传播
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

# 发送消息
message = b'Hello, Multicast!'
while True:
    try:
        # 发送组播消息
        sock.sendto(message, (MULTICAST_GROUP, MULTICAST_PORT))
        print(f"Sent: {message.decode()}")
        time.sleep(2)  # 每2秒发送一次
    except KeyboardInterrupt:
        print("Server stopped.")
        break

sock.close()
    
    
    
    
             
             
             
             
             
             
             
             
             
             