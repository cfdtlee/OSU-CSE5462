/* client.c using TCP */
#include "head.h"

/* client program called with host name where server is run */
main(int argc, char *argv[]) {
	int sock; /* initial SOCKET descriptor */
	struct sockaddr_in sin_addr; /* structure for SOCKET name
	 * setup */
	char buf[BUFFSIZE]; /* one buf size every time */
	struct hostent *hp;
	char filename[FILE_NAME_SIZE]; /* Input the file name */
	int port;

	if (argc < 4) {
		printf("usage: ftpc <remote-IP> <remote-port> <local-file-to-transfer>\n");
		exit(1);
	}

	/* initialize SOCKET connection in unix domain */
	if ((sock = SOCKET(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("error openting datagram SOCKET");
		exit(1);
	}

//	hp = gethostbyname(argv[1]);
//	if (hp == 0) {
//		fprintf(stderr, "%s: unknown host\n", argv[1]);
//		exit(2);
//	}
//
//	/* construct name of SOCKET to send to */
//	memcpy((char*)&port, argv[2],sizeof(int));
//	bcopy((void *) hp->h_addr, (void *)&sin_addr.sin_addr, hp->h_length);
//	sin_addr.sin_family = AF_INET;
//	sin_addr.sin_port = htons(port); /* fixed by adding htons() */

	/* establish connection with server */
//	if (connect(sock, (struct sockaddr *) &sin_addr, sizeof(struct sockaddr_in))
//			< 0) {
//		close(sock);
//		perror("error connecting stream SOCKET");
//		exit(1);
//	}

	// set the file name
	bzero(filename, FILE_NAME_SIZE);
	strncpy(filename, argv[3],strlen(argv[3]) > FILE_NAME_SIZE ?FILE_NAME_SIZE : strlen(argv[3]));

	//read file
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("File :%s not found!\n", filename);
		exit(1);
	}

	//count the file size
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	fseek(file,0L,SEEK_SET);

	//send file imformation including size and name
	int count;
	bzero(buf, BUFFSIZE);
	//set the file size and file name to the buffer
	strncpy(buf, (char*) &size, FILE_BYTES_SIZE);
	strncpy(buf + FILE_BYTES_SIZE, filename, strlen(filename));
	//printf("%s\n",buf);
	count = SEND(sock, buf, BUFFSIZE, 0);
	usleep(1000);
	if (count < 0) {
		perror("Send file information");
		exit(1);
	}

	bzero(buf, BUFFSIZE);
	int file_block_length = 0;
	while ((file_block_length = fread(buf, sizeof(char), BUFFSIZE, file)) > 0) {
		//printf("file_block_length:%d\n", file_block_length);
		if (SEND(sock, buf, file_block_length, 0) < 0) {
			perror("Send");
			exit(1);
		}
		usleep(1000);
		bzero(buf, BUFFSIZE);
	}
	fclose(file);
//	//send one packet to tell tcpd the file has been transfer completely
//	strcpy(buf, "end");
//	//printf("%s\n",buf);
//	if (SEND(sock, buf, BUFFSIZE, 0) < 0) {
//		perror("Send");
//		exit(1);
//	}
	printf("Transfer file finished!\n");
	close(sock);

}
