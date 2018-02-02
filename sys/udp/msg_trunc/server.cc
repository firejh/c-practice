// http://blog.csdn.net/aspnet_lyc/article/details/31767767
#include <iostream>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

struct buf
{
    int  order;
    unsigned char data[10];
};

int main()
{
    int       sockfd;
    int       r;
    socklen_t len;
    struct    sockaddr_in saddr;
    struct    buf buf;
    struct    sockaddr_in caddr;
    struct    msghdr msg;
    struct    iovec iov;
    union
    {
        struct cmsghdr cm;
        char   control[CMSG_SPACE(sizeof(int))];
    } control_un;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(9999);
    if (bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
    {
        perror("bind error");
        exit(-1);
    }

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov.iov_base = &buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_flags = 0;

    while (1)
    {
        r = recvmsg(sockfd, &msg, 0);
        if (r < 0)
        {
            perror("recvfrom error");
            exit(-1);
        }
        cout << buf.order << endl;
        cout << buf.data << endl;
        cout << endl;

        int recv_len = r;
        // 判断是否发生数据报截断
        if (msg.msg_flags == MSG_TRUNC)
        {
            cout << "数据报截断" << endl;
            unsigned char temp_buf[200];
            r = recvfrom(sockfd, &temp_buf, sizeof(temp_buf), 0,
                         NULL, 0);
            if (r == recv_len+sizeof(int))
            {
                cout << "超出的部分被保留" << endl;
            } else {
                cout << "超出的部分被丢弃" << endl;
            }
        }
    }

    return 0;
}
