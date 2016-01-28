/* server.c using TCP */
#include "head.h"
/* server program called with no argument */
main(int argc, char*argv[]) {
	int sock; /* initial socket descriptor */
	int msgsock; /* accepted socket descriptor,
	 * each client connection has a
	 * unique socket descriptor*/
	int rval = 1; /* returned value from a read */
	struct sockaddr_in sin_addr; /* structure for socket name setup */
	char buf[BUFFSIZE]; /* buffer for holding read data */
	int port;

	if (argc < 2) {
		printf("usage: ftps <local-port>\n");
		exit(1);
	}

	printf("TCP server waiting for remote connection from clients ...\n");

	/*initialize socket connection in unix domain*/
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("error openting datagram socket");
		exit(1);
	}

	/* construct name of socket to send to */
	memcpy((char*)&port, argv[1],sizeof(int));
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY;
	sin_addr.sin_port = htons(port);

	/* bind socket name to socket */
	if (bind(sock, (struct sockaddr *) &sin_addr, sizeof(struct sockaddr_in))< 0) {
		perror("error binding stream socket");
		exit(1);
	}

	/* listen for socket connection and set max opened socket connetions to 5 */
	listen(sock, 5);

	/* accept a (1) connection in socket msgsocket */
	if ((msgsock = accept(sock, (struct sockaddr *) NULL, (int *) NULL))== -1) {
		perror("error connecting stream socket");
		exit(1);
	}

	//receive file imformation
	char filename[FILE_NAME_SIZE];
	int count;
	int size;
	bzero(buf, BUFFSIZE);

	count = recv(msgsock, buf, BUFFSIZE, MSG_WAITALL);
	// count = read()
	if (count < 0) {
		perror("recv");
		exit(1);
	}
	//retrieve the file size
	memcpy((char*)&size, buf , FILE_BYTES_SIZE);
	strncpy(filename, buf+FILE_BYTES_SIZE,strlen(buf+FILE_BYTES_SIZE) > FILE_NAME_SIZE ? FILE_NAME_SIZE : strlen(buf+FILE_BYTES_SIZE));

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
	while (size > 0) {
		length = recv(msgsock, buf, BUFFSIZE, 0);
		size -= length;
		if (length < 0) {
			perror("recv");
			exit(1);
		}
		int writelen = fwrite(buf, sizeof(char), length, file);
		if (writelen < length) {
			perror("write");
			exit(1);
		}
		bzero(buf, BUFFSIZE);
	}
	printf("Receieved file %s finished!\n", filename);	

	/* write message back to client */
	char buf2[40] = "Transfer file finished!\n";
	if(write(msgsock, buf2, 1024) < 0) {
		perror("error writing on stream socket");
		exit(1);
	}
	
	fclose(file);

	/* close all connections and remove socket file */
	close(msgsock);
	close(sock);
}

