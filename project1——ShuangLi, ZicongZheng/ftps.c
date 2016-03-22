/* server.c using TCP */
#include "head.h"
/* server program called with no argument */
main(int argc, char*argv[]) {
	int sock; /* initial SOCKET descriptor */
	struct sockaddr_in sin_addr; /* structure for SOCKET name setup */
	char buf[BUFFSIZE]; /* buffer for holding read data */
	int port;

	if (argc < 2) {
		printf("usage: ftps <local-port>\n");
		exit(1);
	}

	printf("TCP server waiting for remote connection from clients ...\n");

	/*initialize SOCKET connection in unix domain*/
	if ((sock = SOCKET(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("error openting datagram SOCKET");
		exit(1);
	}

	/* construct name of SOCKET to send to */
	memcpy((char*)&port, argv[1],sizeof(int));
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY;
	sin_addr.sin_port = htons(FTPS_PORT);

	/* bind SOCKET name to SOCKET */
	if (bind(sock, (struct sockaddr *) &sin_addr, sizeof(struct sockaddr_in))< 0) {
		perror("error binding stream SOCKET");
		exit(1);
	}

//	/* listen for SOCKET connection and set max opened SOCKET connetions to 5 */
//	listen(sock, 5);
//
//	/* accept a (1) connection in SOCKET msgSOCKET */
//	if ((msgsock = accept(sock, (struct sockaddr *) NULL, (int *) NULL))== -1) {
//		perror("error connecting stream SOCKET");
//		exit(1);
//	}

	//receive file imformation
	char filename[FILE_NAME_SIZE];
	int count;
	int size;
	bzero(buf, BUFFSIZE);
	bzero(filename, FILE_NAME_SIZE);
	//printf("test point 0:\n");
	count = RECV(sock, buf, BUFFSIZE, 0);
	if (count < 0) {
		perror("recv");
		exit(1);
	}
	//retrieve the file size
	memcpy((char*)&size, buf , FILE_BYTES_SIZE);
	memcpy(filename, buf+FILE_BYTES_SIZE,strlen(buf+FILE_BYTES_SIZE) > FILE_NAME_SIZE ? FILE_NAME_SIZE : strlen(buf+FILE_BYTES_SIZE));
	printf("Preparing receive file %s, the file size is %d Bytes \n", filename, size);

	//create a different directory temp
	char dir[30] = "./receive";
	DIR *mydir = NULL;
	//if the dir is not exist
	if ((mydir = opendir(dir)) == NULL) {
		int status = mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (status == -1) {
			perror("create directory");
			exit(1);
		}
	}

	//receive file
	strcat(dir,"/");
	strcat(dir,filename);
	FILE *file = fopen(dir, "wb+");
	if (NULL == file) {
		perror("open");
		exit(1);
	}
	bzero(buf, BUFFSIZE);

	int length = 0;

	while (size>0) {
		//printf("test point 1:\n");

		length = RECV(sock, buf, BUFFSIZE, 0);
		//printf("length:%d\n",length);
		if (length < 0) {
			perror("recv");
			exit(1);
		}
		//printf("size:%d\n",size);
		int actual = (size>length)?length:size;
		int writelen = fwrite(buf, sizeof(char),actual , file);
		if (writelen < actual) {
			perror("write");
			exit(1);
		}
		size -= length;
		bzero(buf, BUFFSIZE);
	}
	printf("Receieved file %s finished!\n", filename);
	/* close all connections and remove SOCKET file */
	//close(msgsock);
	close(sock);
}

