/*************************************************************************
    > File Name: main.c
    > Author: KrisChou
    > Mail:zhoujx0219@163.com
    > Created Time: Fri 05 Sep 2014 11:19:13 PM CST
 ************************************************************************/
#include "server.h"
int main(int argc, char* argv[])//exe chld_cnt
{
    if(argc != 2)
    {
        printf("Usage: exe , child_cnt! \n");
        exit(1);
    }
    int child_cnt = atoi(argv[1]);
    pNODE arr_child = (pNODE)calloc(child_cnt, sizeof(NODE)) ; /* 动态数组维护子进程池 */
    make_child(arr_child, child_cnt);
    
    int fd_listen, fd_client ;
    my_socket(&fd_listen, MY_TCP, SER_IP, SER_PORT);
    my_listen(fd_listen, 10);
    
    fd_set readset, readyset ;
    FD_ZERO(&readset);
    FD_ZERO(&readyset);
    FD_SET(fd_listen, &readset);
    int index ;
    for(index = 0; index < child_cnt; index ++)
    {
        FD_SET(arr_child[index].s_sfd, &readset);
    }
    
    int select_ret ;
    struct timeval tm ;
    while(1)
    {
        tm.tv_sec = 0 ;
        tm.tv_usec = 1000 ;
        readyset = readset ;
        select_ret = select(1024, &readyset, NULL, NULL, &tm);
        if(select_ret == 0)        /* 轮询时间内，所有描述符均没有活动，返回0，继续轮询 */
        {
            continue ;
        }else if(select_ret == -1) /* 信号 */
        {
            if(errno == EINTR)
            {
                continue ;
            }else 
            {
                exit(1);
            }
        }else 
        {
            if(FD_ISSET(fd_listen, &readyset))
            {
            fd_client = accept(fd_listen, NULL, NULL) ;    
            dispatch(arr_child, child_cnt ,fd_client);
            close(fd_client);
            }
            for(index = 0; index < child_cnt; index ++)
            {
                if(FD_ISSET(arr_child[index].s_sfd, &readyset))
                {
                    int val ;
                    read(arr_child[index].s_sfd, &val, 4);
                    arr_child[index].s_state = ST_IDLE ;
                }
            }
            
        }
        
    }   
}
