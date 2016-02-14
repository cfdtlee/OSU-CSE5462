#ifndef LAB3_FUNCTIONS
#define LAB3_FUNCTIONS
#include "head.h"
int SOCKET(int family, int type, int protocol); 
int CONNECT(int sockfd, struct sockaddr *serv_addr, int addrlen); 
int ACCEPT(int sockfd, void *peer, int *addrlen); 
int SEND(int sockfd, const void *msg, int len, int flags);
int RECV(int sockfd, void *buf, int len, unsigned int flags); 
int BIND (int sockfd, struct sockaddr *myaddr, int addrlen); 

#endif