/*
 * head.h
 *
 *  Created on: Jan 26, 2016
 *      Author: parallels
 */

#ifndef HEAD_H_
#define HEAD_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/tcp.h>

#include <errno.h>
#include <sys/time.h> 
#include <fcntl.h>
#include <sys/ioctl.h> 


typedef uint16_t crc;

#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0x8005 //CRC-16-IBM

#define BUFFSIZE 1000  /*max size of one block to be transfered */
#define FILE_NAME_SIZE 20
#define FILE_BYTES_SIZE 4
#define SEND_RECV_BUF_SIZE 64

#define TCPDC_LOCAL_PORT 10002
#define TROLLC_PORT 10003
#define TCPDS_REMOTE_PORT 10004
#define FTPS_PORT 10005
#define TROLLS_PORT 10006
#define TCPDC_REMOTE_PORT 10007

#define BUFFER_SIZE 1200
#define INFO_SIZE 1000
#define OPT_SIZE 10
#define ID_SIZE 4
#define TIME_SIZE 8



#define LOCALHOST "127.0.0.1"

#define WINDOW 20

ssize_t SEND(int socket, const void *buffer, size_t length, int flags);

ssize_t RECV(int socket, void *buffer, size_t length, int flags);

int ACCEPT(int socket, struct sockaddr *addr, socklen_t *length_ptr);

int CONNECT(int socket, const struct sockaddr *address,socklen_t address_len);

int SCOKET(int domain, int type, int protocol);

#endif /* HEAD_H_ */
