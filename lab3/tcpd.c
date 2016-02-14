#include "head.h"

void main(int argc, char *argv[])
{
    int localSock, trollSock, ftpcSock, buflen;
    char cli_buf[BUFFER_MAX_LENGTH] = "Hello in UDP from client";
    struct sockaddr_in localSockAddr, remoteSockAddr, trollSockAddr, ftpcSockAddr;
    struct hostent *hp, *gethostbyname();
    MyMessage packet;

    /* create socket for connecting to server */
    localSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(localSock < 0) {
        perror("opening datagram socket");
        exit(2);
    }

    /* construct localSockAddr for connecting to local process */
    localSockAddr.sin_family = AF_INET;
    localSockAddr.sin_port = htons(atoi(LOCAL_PORT));
    localSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(localSock, (struct sockaddr *)&localSockAddr, sizeof localSockAddr) < 0) {
        perror("client bind");
        exit(1);
    }

    trollSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(trollSock < 0) {
        perror("opening datagram socket");
        exit(2);
    }

    /* construct trollSockAddr for connecting to troll process */
    trollSockAddr.sin_family = AF_INET;
    trollSockAddr.sin_port = htons(atoi(TROLL_PORT));
    trollSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(trollSock, (struct sockaddr *)&trollSockAddr, sizeof trollSockAddr) < 0) {
        perror("client bind");
        exit(1);
    }

    ftpcSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ftpcSock < 0) {
        perror("opening datagram socket");
        exit(2);
    }

    /* construct ftpcSockAddr for connecting to troll process */
    ftpcSockAddr.sin_family = AF_INET;
    ftpcSockAddr.sin_port = htons(atoi(TCPD_C_PORT));
    ftpcSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(ftpcSock, (struct sockaddr *)&ftpcSockAddr, sizeof ftpcSockAddr) < 0) {
        perror("client bind");
        exit(1);
    }

    /* Loop to do all packet forwarding */
    while (1) {
        // printf("while 1\n");
        // bzero(&packet, sizeof(packet)); 
        // if (recvfrom(localSock, &packet, sizeof(packet), 0, ) < 0, ) {
        //     perror("Receiving datagram message from local process");
        //     exit(1);
        // }
        // printf("while 2\n");

        if (argv[1] == "c"){//if it's from the ftpc
            bzero(&packet, sizeof(packet));
            if (recvfrom(localSock, &packet, sizeof(packet), 0, (struct sockaddr *)ftpcSockAddr， sizeof(ftpcSockAddr)) < 0 ) {
                perror("Receiving datagram message from local process");
                exit(1);
            }

            if(sendto(trollSock, &packet, sizeof(packet), 0, (struct sockaddr *)trollSockAddr, sizeof(trollSockAddr)) < 0) {
                perror("sending datagram message");
                exit(4);
            }
        }

        else {//if it's from ftps
            // unsigned int size = ntohl(*((unsigned int*)packet.contents));
            bzero(&packet, sizeof(packet));
            
            if (recvfrom(trollSock, &packet, sizeof(packet), 0, (struct sockaddr *)ftpcSockAddr， sizeof(ftpcSockAddr)) < 0) {
                perror("Receiving datagram message from troll");
                exit(1);
            }

            if(sendto(localSock, &packet, sizeof(packet), 0, (struct sockaddr *)localSockAddr, sizeof(localSockAddr)) < 0) {
                perror("sending datagram message to ftps");
                exit(4);
            }
        }
    }
}