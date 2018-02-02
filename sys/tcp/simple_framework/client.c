/*************************************************************************
    > File Name: client.c
    > Author: KrisChou
    > Mail:zhoujx0219@163.com
    > Created Time: Fri 05 Sep 2014 03:48:27 PM CST
 ************************************************************************/
#include "my_socket.h"
#define MY_IP "127.0.0.1"
#define MY_PORT 6666
#define SER_IP "127.0.0.1"
#define SER_PORT 8888
#define SIZE 8192
#define MSG_SIZE (SIZE - 4)

typedef struct tag_mag//
{
    int msg_len ;
    char msg_buf[MSG_SIZE];//8188
}MSG, *pMSG;

int main(int argc, char* argv[])
{
    int sfd ;
    my_socket(&sfd, MY_TCP, MY_IP, atoi(argv[1]));
    my_connect(sfd, SER_IP, SER_PORT);
    MSG my_msg ;
    while(memset(&my_msg, 0, sizeof(MSG)), fgets(my_msg.msg_buf, MSG_SIZE, stdin)!= NULL)
    {
        my_msg.msg_len = strlen(my_msg.msg_buf);
        my_send(NULL, sfd, &my_msg, 4 + my_msg.msg_len );
        memset(&my_msg, 0, sizeof(MSG));
        my_recv(NULL, sfd, &my_msg, 4);
        my_recv(NULL, sfd, &my_msg.msg_buf, my_msg.msg_len);
        printf("recv from server : %s \n", my_msg.msg_buf);

    }
    /* 客户端退出时，向服务器发送一个长度为0的消息 ，用于通知服务器退出 */
    memset(&my_msg, 0, sizeof(MSG));
    my_send(NULL, sfd, &my_msg, 4 + my_msg.msg_len);
    close(sfd);

}

