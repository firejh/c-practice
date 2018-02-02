/*> readme: server.c
  > Author: KrisChou
  > Mail:zhoujx0219@163.com
  > Created Time: Fri 05 Sep 2014 11:17:56 PM CST
 ************************************************************************/
#include "server.h"
void make_child(pNODE arr, int cnt)
{
    int index ;
    for(index = 0; index < cnt; index ++)
    {
        pid_t pid ;
        int fds[2] ;//fds[0] - c  fds[1] - p
        socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
        pid = fork() ;
        if(pid == 0)// child
        {
            close(fds[1]);         /* 子进程用fds[0]，关闭fds[1] */
            child_main(fds[0]) ;   /* 每创建一个子进程，子进程就进入该函数中(死循环),接收请求，处理请求，如此循环。*/

        }else
        {
            /* 初始化进程池队列中的每一个子进程 */
            arr[index].s_sfd = fds[1] ;
            arr[index].s_state = ST_IDLE ;
            close(fds[0]);         /* 父进程用fds[1], 关闭fds[0] */
        }

    }
}

void child_main(int sfd)
{
    int fd_client ;
    int flag ;
    int readn ;
    pid_t pid = getpid();
    while(1)
    {
        readn = read(sfd, &flag, 4);
        printf("readn: %d \n", readn);
        printf("read from father: %d \n", flag);
        recv_fd(sfd, &fd_client);
        handle_request(fd_client);
        write(sfd, &pid, sizeof(pid));
    }
}
void handle_request(int sfd)
{

    MSG my_msg ;
    int recvn ;
    while(1)
    {
        memset(&my_msg, 0, sizeof(MSG));
        my_recv(&recvn, sfd, &my_msg, 4);
        if(my_msg.msg_len  == 0)
        {
            break ;
        }
        my_recv(NULL, sfd, my_msg.msg_buf, my_msg.msg_len);
        my_send(NULL, sfd, &my_msg, my_msg.msg_len + 4);

    }

}
void send_fd(int sfd, int fd_file)
{
    struct msghdr my_msg ;
    memset(&my_msg, 0, sizeof(my_msg));

    struct iovec bufs[1] ;
    char buf[32] = "hello world ! \n";
    bufs[0].iov_base = buf ;
    bufs[0].iov_len = strlen(buf) ;

    my_msg.msg_name = NULL ;
    my_msg.msg_namelen = 0 ;
    my_msg.msg_iov = bufs ;
    my_msg.msg_iovlen = 1 ;
    my_msg.msg_flags = 0 ;

    struct cmsghdr *p  ;
    int cmsg_len = CMSG_LEN(sizeof(int)) ;     /* 所传为文件描述符，因此sizeof(int) */
    p = (struct cmsghdr*)calloc(1, cmsg_len) ;
    p -> cmsg_len = cmsg_len ;
    p -> cmsg_level = SOL_SOCKET ;
    p -> cmsg_type = SCM_RIGHTS ;
    *(int*)CMSG_DATA(p) = fd_file ;

    my_msg.msg_control = p ;
    my_msg.msg_controllen = cmsg_len ;

    int sendn ;
    sendn = sendmsg(sfd, &my_msg, 0);
    printf("send masg len : %d \n", sendn);
}
void recv_fd(int sfd, int* fd_file)
{
    struct msghdr my_msg ;

    struct iovec bufs[1] ;
    char buf1[32]="" ;
    bufs[0].iov_base = buf1 ;
    bufs[0].iov_len = 31 ;

    my_msg.msg_name = NULL ;
    my_msg.msg_namelen = 0 ;
    my_msg.msg_iov = bufs ;
    my_msg.msg_iovlen = 2 ;
    my_msg.msg_flags = 0 ;

    struct cmsghdr *p  ;
    int cmsg_len = CMSG_LEN(sizeof(int)) ;
    p = (struct cmsghdr*)calloc(1, cmsg_len) ;
    my_msg.msg_control = p ;
    my_msg.msg_controllen = cmsg_len ;

    int recvn ;
    recvn = recvmsg(sfd, &my_msg, 0);

    *fd_file = *(int*)CMSG_DATA((struct cmsghdr*)my_msg.msg_control); //写成*(int*)CMSG_DATA(P)也可

    printf("buf1: %s, recv msg len : %d   \n", buf1, recvn);

}
void dispatch(pNODE arr, int cnt, int fd_client)
{
    int index ;
    for(index = 0 ; index < cnt; index ++)
    {
        if(arr[index].s_state == ST_IDLE)
        {
            write(arr[index].s_sfd, &index, 4);
            send_fd(arr[index].s_sfd, fd_client); /* 向空闲的子进程分配任务，将服务器accept返回的socket描述符发送给子进程*/
            arr[index].s_state = ST_BUSY ;
            break ;
        }
    }
}
