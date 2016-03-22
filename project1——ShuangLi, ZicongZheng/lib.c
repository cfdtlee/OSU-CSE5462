/*
 * library.c
 *
 *  Created on: Feb 1, 2016
 *      Author: Zicong
 */
#include "head.h"

ssize_t SEND(int socket, const void *buffer, size_t length, int flags){
	struct sockaddr_in destination;
	destination.sin_family = AF_INET;
	destination.sin_port = htons(TCPDC_LOCAL_PORT);
	struct hostent *hp = gethostbyname("0");
	if(hp == 0) {
		fprintf(stderr, "%s:unknown host\n", LOCALHOST);
		exit(3);
	}
	bcopy((char *)hp->h_addr, (char *)&destination.sin_addr, hp->h_length);

	return sendto(socket, buffer, length, flags,(struct sockaddr *)&destination, sizeof(destination));

}

ssize_t RECV(int socket, void *buffer, size_t length, int flags){
	struct sockaddr_in source;
	source.sin_family = AF_INET;
	source.sin_port = htons(FTPS_PORT);
	struct hostent *hp = gethostbyname(LOCALHOST);
	if (hp == 0) {
		fprintf(stderr, "%s:unknown host\n", LOCALHOST);
		exit(3);
	}
	bcopy((char *)hp->h_addr, (char *)&source.sin_addr, hp->h_length);
	int len = sizeof(source);
	return recvfrom(socket,buffer,length, flags, (struct sockaddr *)&source, ( socklen_t *)&len);
}

int ACCEPT (int socket, struct sockaddr *addr, socklen_t *length_ptr){
	return 0;
}

int CONNECT(int socket, const struct sockaddr *address,socklen_t address_len){
	return 0;
}

int SOCKET(int namespace, int style, int protocol){
	return socket(namespace,style,protocol);
}

