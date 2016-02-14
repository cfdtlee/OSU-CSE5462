/*
 * head.h
 *
 *  Created on: Jan 26, 2016
 *      Author: parallels
 */

#ifndef HEAD_H_
#define HEAD_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "function.h"
#define BUFFSIZE 1000  /*max size of one block to be transfered */
#define FILE_NAME_SIZE 20
#define FILE_BYTES_SIZE 4
#define BUFFER_MAX_LENGTH 1000

// #define FTPC_IP "127.0.0.0"
// #define FTPC_PORT 1270
#define FTPS_IP "127.0.0.0"
#define FTPS_PORT "2525"

#define TROLL_IP "127.0.0.0"
#define TROLL_PORT "2626"

#define TCPD_C_IP "164.135.2.2"
#define TCPD_C_PORT "2323"

#define TCPD_S_IP "164.135.2.2"
#define TCPD_S_PORT "2424"

#define LOCAL_PORT "2323"
#define GLOBAL_PORT "2727"

typedef struct MyMessage {
    struct sockaddr_in msg_header;
    int client;
    char contents[BUFFER_MAX_LENGTH];
} MyMessage;

#endif /* HEAD_H_ */
