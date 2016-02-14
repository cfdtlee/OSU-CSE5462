#include "head.h"
#include "function.h"
int SOCKET(int family, int type, int protocol)
{
	return socket(family, type, protocol);
}

int SEND(int sockfd, const void *msg, int len, int flags)
{
	MyMessage mmsg;

	struct sockaddr_in tcpdSockAddr;
	tcpdSockAddr.sin_family = AF_INET;
    tcpdSockAddr.sin_port = htons(atoi(TCPD_C_PORT));
    tcpdSockAddr.sin_addr.s_addr = inet_addr(atoi(TCPD_C_IP));

    mmsg.msg_header = tcpdSockAddr;
    mmsg.client = 1;
    strcpy(mmsg.contents, msg);
	int n = sendto(sockfd, (char *)&mmsg, sizeof mmsg, flags,
					(struct sockaddr *)&tcpdSockAddr, sizeof tcpdSockAddr);
	if (n!=sizeof mmsg) {
		perror("ftpc sendto");
		exit(1);
	}
}

int RECV(int sockfd, void *buf, int len, unsigned int flags)
{

}

int BIND (int sockfd, struct sockaddr *myaddr, int addrlen)
{
	return NULL;
}

int CONNECT(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
	return NULL;
}

int ACCEPT(int sockfd, void *peer, int *addrlen)
{
	return NULL;
}