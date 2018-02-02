/*************************************************************************
    > File Name: my_socket.c
    > Author: KrisChou
    > Mail:zhoujx0219@163.com 
    > Created Time: Mon 01 Sep 2014 06:54:48 PM CST
 ************************************************************************/

/* 本代码用于在一台主机上模拟socket通信。因此IP地址对于server和client而言是一样的。
 * 为了简化代码，此处即使是客户端，也提前分配好端口号。事实上，主动方的端口号可以由系统分配，不用提前绑定
 * --> 无论server或者client，都会预先绑定本地socket */

/* 本代码local_sfd代表本地socket描述符。
 * 对于服务器而言，就是用于监听的socket; 对于客户端而言就是用于通信的socket
 * peer_sfd，代表与对方通信的socket描述符。
 * 对于服务器而言，由accept以传出参数形式返回；对于客户端而言，就是本地socket */

#include "my_socket.h"

void my_socket(OUT int *local_sfd, int protocal, char *local_ip, int local_port)
{
    MY_ASSERT(protocal == MY_TCP || protocal == MY_UDP, "wrong arg! protocal is MY_TCP or MY_UDP! \n");
    /* 创建本地socket */
    if(protocal == MY_TCP)
    {
        MY_ASSERT(-1 != (*local_sfd = socket(AF_INET, SOCK_STREAM, 0)), "tcp_socket init falure!\n");
    }else if(protocal == MY_UDP)
    {
        MY_ASSERT(-1 != (*local_sfd = socket(AF_INET, SOCK_DGRAM, 0)),  "udp_socket init failure!\n");
    }
    /* 将本地联系方式bind到本地socket */
    SA local_addr;
    memset(&local_addr, 0, sizeof(SA));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = htons(local_port);
    local_addr.sin_addr.s_addr = inet_addr(local_ip);
    MY_ASSERT( 0 == bind(*local_sfd, (pSA)&local_addr, sizeof(SA)), "bind failure!\n");
}

/*----------------------------- 以下针对TCP ----------------------------------------------------- */

/* server: listen + accept */
void my_listen(int local_sfd, int backlog)
{
    MY_ASSERT( 0 == listen(local_sfd, backlog), "listen failure!\n");
}

void my_accept(OUT int *peer_sfd, int local_sfd, OUT pSA peer_addr, IN_OUT int *addr_len )
{
    MY_ASSERT(-1 != (*peer_sfd = accept(local_sfd, peer_addr, addr_len)), "accept failure!\n");
}

/* client: connect */
void my_connect(int local_sfd, pSA peer_addr, int addr_len)
{
    int cnt = 0;
    // 10次连不上就退出程序
    while(-1 == connect(local_sfd, peer_addr, addr_len))
    {
        cnt++;
        if(cnt == 10)
        {
            printf("connect failure!\n");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
}

/* recv and send */
void my_recv(OUT int *recv_len, int peer_sfd, IN_OUT void *base, int len)
{
    MY_ASSERT(-1 != (*recv_len = recv(peer_sfd, base, len, 0)), "recv error! \n");
}

void my_send(OUT int *send_len, int peer_sfd, void *base, int len)
{
    MY_ASSERT(-1 != (*send_len = send(peer_sfd, base, len, 0)), "send error! \n");
}

/*---------------------------- 以下针对UDP--------------------------------------------------------*/

void my_recvfrom(OUT int *recvfrom_len, int peer_sfd, IN_OUT void *base, int len, OUT pSA peer_addr, IN_OUT int *addr_len)
{
    MY_ASSERT(-1 != (*recvfrom_len = recvfrom(peer_sfd, base, len, 0, peer_addr, addr_len)), "recvfrom failure!\n");
}

void my_sendto(OUT int *sendto_len, int peer_sfd, OUT void *base, int len,  pSA peer_addr, int addr_len)
{
    MY_ASSERT(-1 != (*sendto_len = sendto(peer_sfd, base, len, 0, peer_addr, addr_len)), "sendto failure!\n");
}

/* close */
void my_close(int sfd)
{
    MY_ASSERT(0 == close(sfd), "close failure!\n");
}

