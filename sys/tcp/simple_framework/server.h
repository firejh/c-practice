#ifndef __SERVER_H__
#define __SERVER_H__
#include "my_socket.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <errno.h>
#define SER_IP "127.0.0.1"
#define SER_PORT 8888
#define ST_BUSY 1
#define ST_IDLE 2
#define SIZE 8192
#define MSG_SIZE (SIZE - 4)

typedef struct tag_mag
{
	    int msg_len ;
		    char msg_buf[MSG_SIZE];//8188
}MSG, *pMSG;

typedef struct tag_chd
{
	    int s_sfd ;
		    int s_state ;
}NODE, *pNODE;

extern int errno ;
void make_child(pNODE arr, int cnt);
void child_main(int sfd) ;
void handle_request(int sfd);
void send_fd(int sfd, int fd_file) ;
void recv_fd(int sfd, int* fd_file) ;
void dispatch(pNODE arr, int cnt, int fd_client);
#endif
