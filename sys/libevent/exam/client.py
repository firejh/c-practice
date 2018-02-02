客户端代码 client.py
import socket
import datetime

"""
定义基本的信息: 主机和端口要和服务器一致
"""
HOST = "localhost"  #服务其地址
PORT = 23151       #服务器端口
BUFFERSIZE = 1024
ADDRESS = (HOST, PORT)

"""
建立套接字,开始连接
"""
TCPClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
TCPClient.connect(ADDRESS) #连接服务器

"""
开始进行数据的传输
"""
while True:
    senddate = raw_input("input:")
    if senddate:
        TCPClient.send('%s' % (senddate))  #发送数据

    recvdate = TCPClient.recv(BUFFERSIZE)    #接受数据
    curTime = datetime.datetime.now()  #获得当前时间 格式是：datetime.datetime(2012, 3, 13, 1, 29, 51, 872000)
    curTime = curTime.strftime('%Y-%m-%m %H:%M:%S')     #转换格式
    print "%s  %s" % (HOST, curTime)
    print  recvdate
    if recvdate == '88':
            break

"""
传输完毕，关闭套接字
"""

print "client close"
TCPClient.close()


