#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define IN
#define OUT
#define IN_OUT
#define MY_TCP 1
#define MY_UDP 2
typedef struct sockaddr* pSA ;
typedef struct sockaddr_in SA ;
#define MY_ASSERT(flag,msg) ( (flag) ? NULL : ( fprintf(stdout,msg), exit(EXIT_FAILURE) ) )   // NULL代表什么也不做

void my_socket(OUT int *local_sfd, int protocal, char *local_ip, int local_port);
void my_listen(int local_sfd, int backlog);
void my_accept(OUT int *peer_sfd, int local_sfd, OUT pSA peer_addr, IN_OUT int *addr_len );
void my_connect(int local_sfd, pSA peer_addr, int addr_len);
void my_recv(OUT int *recv_len, int peer_sfd, IN_OUT void *base, int len);
void my_send(OUT int *send_len, int peer_sfd, void *base, int len);
void my_recvfrom(OUT int *recvfrom_len, int peer_sfd, IN_OUT void *base, int len, OUT pSA peer_addr, IN_OUT int *addr_len);
void my_sendto(OUT int *sendto_len, int peer_sfd, OUT void *base, int len,  pSA peer_addr, int addr_len);
void my_close(int sfd);


#endif
