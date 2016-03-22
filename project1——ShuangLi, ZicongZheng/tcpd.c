/*
 * tcpd.c
 *
 *  Created on: Feb 1, 2016
 *      Author: Zeller
 */
#include "head.h"

typedef unsigned int u32;
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#define TCP_RTO_MIN 2000
struct tcp_sock {  
	/* RTT measurement */  
	u32 srtt; /* smoothed round trip time << 3 */  
	u32 mdev; /* medium deviation */  
	u32 mdev_max; /* maximal mdev for the last rtt period */  
	u32 rttvar; /* smoothed mdev_max */  
	u32 rtt_seq; /* sequence number to update rttvar */  
	u32     snd_una; 		/* First byte we want an ack for        */
	u32     snd_nxt;        /* Next sequence we send                */
};

static inline int after(u32 seq1, u32 seq2)
{
    return (seq1-seq2) > 0;
}

// RTO的计算
static inline u32 __tcp_set_rto(const struct tcp_sock *tp)
{
    return (tp->srtt >> 3) + tp->rttvar;
}
/* Called to compute a smoothed rtt estimate. The data fed to this  
 * routine either comes from timestamps, or from segments that were 
 * known _not_ to have been retransmitted [see Karn/Partridge Proceedings 
 * SIGCOMM 87]. The algorithm is from the SIGCOMM 88 piece by Van 
 * Jacobson. 
 * NOTE : the next three routines used to be one big routine. 
 * To save cycles in the RFC 1323 implementation it was better to break it 
 * up into three procedures. ——erics 
 */  
  
void tcp_rtt_estimator (struct tcp_sock *tp, const u32 mrtt)  
{  
    // struct tcp_sock *tp = tcp_sk(sk);  
    long m = mrtt; /*此为得到的新的RTT测量值*/  
  
    /* The following amusing code comes from Jacobson's article in 
     * SIGCOMM '88. Note that rtt and mdev are scaled versions of rtt and 
     * mean deviation. This is designed to be as fast as possible 
     * m stands for "measurement". 
     *  
     * On a 1990 paper the rto value is changed to : 
     * RTO = rtt + 4 * mdev 
     * 
     * Funny. This algorithm seems to be very broken. 
     * These formulae increase RTO, when it should be decreased, increase 
     * too slowly, when it should be increased quickly, decrease too quickly 
     * etc. I guess in BSD RTO takes ONE value, so that it is absolutely does 
     * not matter how to calculate it. Seems, it was trap that VJ failed to  
     * avoid. 8) 
     */  
    if (m == 0)  
        m = 1; /* RTT的采样值不能为0 */  
  
    /* 不是得到第一个RTT采样*/  
    if (tp->srtt != 0) {  
        m -= (tp->srtt >> 3); /* m is now error in rtt est */  
        tp->srtt += m; /* rtt = 7/8 rtt + 1/8 new ，更新srtt*/  
  
        if (m < 0) { /*RTT变小*/  
            m = -m; /* m is now abs(error) */  
            m -= (tp->mdev >> 2); /* similar update on mdev */  
  
            /* This is similar to one of Eifel findings. 
             * Eifel blocks mdev updates when rtt decreases. 
             * This solution is a bit different : we use finer gain 
             * mdev in this case (alpha * beta). 
             * Like Eifel it also prevents growth of rto, but also it 
             * limits too fast rto decreases, happening in pure Eifel. 
             */  
             if (m > 0) /* |err| > 1/4 mdev */  
                 m >>= 3;  
  
        } else { /* RTT变大 */  
            m -= (tp->mdev >> 2); /* similar update on mdev */  
        }  
  
        tp->mdev += m; /* mdev = 3/4 mdev + 1/4 new，更新mdev */  
  
        /* 更新mdev_max和rttvar */  
        if (tp->mdev > tp->mdev_max) {  
            tp->mdev_max = tp->mdev;  
            if (tp->mdev_max > tp->rttvar )  
                tp->rttvar = tp->mdev_max;  
        }  
  
       /* 过了一个RTT了，更新mdev_max和rttvar */  
        if (after(tp->snd_una, tp->rtt_seq)) {  
            if (tp->mdev_max < tp->rttvar)/*减小rttvar */  
                tp->rttvar -= (tp->rttvar - tp->mdev_max) >> 2;   
            // tp->rtt_seq = tp->snd_nxt;  
            tp->mdev_max = TCP_RTO_MIN; /*重置mdev_max */  
        }  
  
    } else {   
    /* 获得第一个RTT采样*/  
        /* no previous measure. */  
        tp->srtt = m << 3; /* take the measured time to be rtt */  
        tp->mdev = m << 1; /* make sure rto = 3 * rtt */  
        tp->mdev_max = tp->rttvar = max(tp->mdev, TCP_RTO_MIN);  
        // tp->rtt_seq = tp->snd_nxt; /*设置更新mdev_max的时间*/  
    }  
}

void init(struct tcp_sock *tp)
{
	// printf("finding bugs_2\n");
	fflush(stdout);

	tp->srtt = 0;

	tp->mdev = 0; /* medium deviation */
	// printf("finding bugs_3\n");
	fflush(stdout);
	tp->mdev_max = 0; /* maximal mdev for the last rtt period */  
	tp->rttvar = 0; /* smoothed mdev_max */  

	tp->rtt_seq = 0; /* sequence number to update rttvar */  
	tp->snd_una = 0; 		/* First byte we want an ack for        */
	tp->snd_nxt = 0; 
}


typedef struct tnode{ 
	int id;
	struct timeval delta_time;
	int port_num;
	char info[INFO_SIZE];
	struct tnode *forward;
	struct tnode *next;
} TimeNode;

TimeNode *add_timenode(TimeNode *head, int id, double dtime, int pnum, char * info);
TimeNode *delete_timenode(TimeNode *head, int pnum);
void starttimer(double, int);
void canceltimer(int);
void print_time_list(TimeNode *head);
void stop_timing();
TimeNode *update_time_list(TimeNode *head);
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
void timeval_add (struct timeval *result, struct timeval *x, struct timeval *y);
int find_timenode(TimeNode *head, int id);
void resend(int id);
u32 calculate_rtt (struct timeval x, struct timeval y);

int pfds[2];
int pfds_back[2];
struct timeval lasttime;
TimeNode *head;

crc  crcTable[256];

crc	crcFast(uint8_t const message[], int nBytes)
{
    uint8_t data;
    crc remainder = 0;


    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    int byte;
    for (byte = 0; byte < nBytes; ++byte)
    {
        data = message[byte] ^ (remainder >> (WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }

    /*
     * The final remainder is the CRC.
     */
    return (remainder);

}   /* crcFast() */

int checkEmpty(char buffer[],int length){
	int i = 0;
	char temp = 0xff;
	for(i =0;i<length;i++){
		if(buffer[i]&temp)
			//if it is not empty, return 0
			return 0;
	}
	//if it is empty, return 1
	return 1;
}

struct Packet {
		struct sockaddr_in header;
		struct tcphdr tcp_header;
		struct timeval time_stamp;
		char body[BUFFSIZE];
	};

int header_size = sizeof(struct sockaddr_in)+sizeof(struct tcphdr)+sizeof(struct timeval)+4;
void client() {
	pid_t pid;
	// int rc;
	char buf[BUFFER_SIZE];
	char bufin[BUFFER_SIZE];
	pid = getpid();
	// sprintf(buf, "process pid is %d\n", pid);
	// sprintf(buf, "This line is from pid %d, value = %d\n", pid, i);
	// write(1, buf, strlen(buf));
	pipe(pfds); // write to timer at pfds[1], timer receive at pfds[0]
	pipe(pfds_back); // write back to driver at pfds_back[1], driver reveive at pfds[0]

	if (fork()) { // timer
		int result, nread;
		fd_set inputs; 
    	struct timeval * timeout1 = malloc(sizeof(struct timeval));
    	timeout1 = NULL;
    	FD_ZERO(&inputs);//用select函数之前先把集合清零  
    	// FD_SET(pfds[0], &inputs);//把要检测的句柄——标准输入（0），加入到集合里
//    	 timeout1.tv_sec = 2;
//         timeout.tv_usec = 0;
        // timeout = NULL;
        // char bufin[BUFFER_SIZE];
        char operation[OPT_SIZE];
        int id;
        double time_interval;
        // int t = 8;
        gettimeofday(&lasttime, NULL); /////////////////////////////////////////////////////////

		while(1)
		{
			// printf("BBBBBBBefore select\n");
			fflush(stdout);
			FD_SET(pfds[0], &inputs);
//			if(timeout1->tv_sec == 0 && timeout1->tv_usec ==0)
//			{
//				printf("timeout = NULL\n");
//				result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, NULL);
//				printf("timeout end");
//
//			}
//			else
			if (timeout1 == NULL)
			{
				
			}
			// else {printf("Timer:timeout = %ld.%ld\n", timeout1->tv_sec, timeout1->tv_usec);}
			result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, timeout1);//&timeout); //

			// printf("AAAAfter select\n");
			fflush(stdout);
			
			if (result == 0)
			{
				head = update_time_list(head); //
				printf("Timer: timeout, update time list \n");
				fflush(stdout);
				if (!head)
				{
					printf("Timer:set timeout1 = NULL\n");
					timeout1 = NULL;
				}
				// break;
			}
			else if (result == -1)		
			{
				perror("select"); 
				exit(1); 
			}
			else
			{
				if(FD_ISSET(pfds[0], &inputs)) 
				{
					// printf("In timer, pfds is ready\n");
					nread = read(pfds[0], bufin, BUFFER_SIZE);
					strncpy(operation, bufin, sizeof(operation)); //operation
					memcpy((char*)&id, bufin + OPT_SIZE, sizeof(int));
					// strncpy((char *)&id, bufin + OPT_SIZE, ID_SIZE); //ID
					memcpy((char*)&time_interval, bufin + OPT_SIZE + ID_SIZE, sizeof(double));
					if (strcmp(operation, "stop") == 0)
					{
						printf("Timer:stop\n"); //
						fflush(stdout);
						break;
					}
					if (strcmp(operation, "start") == 0)
					{
						// printf("Timer:prepare to call add_timenode, id = %d, time_interval = %lf\n", id, time_interval);
						// fflush(stdout);
						head = add_timenode(head, id, time_interval, 0, NULL);
						// printf("DDDebuging000!\n");
						// timeout1->tv_sec = head->delta_time.tv_sec;
						// timeout1->tv_usec = head->delta_time.tv_usec;
						timeout1 = &head->delta_time;
						// printf("DDDebuging!\n");
					}
					if (strcmp(operation, "cancel") == 0)
					{
						struct timeval now_test;
						gettimeofday(&now_test,NULL);
						// printf("Timer:before delete %d: at %ld.%d\n", id, now_test.tv_sec, now_test.tv_usec);
						// printf("Timer:before delete %d:", id);
						// print_time_list(head);
						head = delete_timenode(head, id);
						// printf("Timer:after delete %d:", id);
						// print_time_list(head);
						
					}
				}
				 
			}
		}
	} 
	else 
	{ // driver
	printf("in driver\n");
	fflush(stdout);
	// set variable for rtp calculation
	struct tcp_sock *tp = malloc(sizeof(struct tcp_sock));
//	tp->srtt = 0;
	//printf("finding bugs\n");
	init(tp);

	char sendbuf[SEND_RECV_BUF_SIZE*BUFFSIZE];

	// last ACK received
	int lar = -1;
	// last frame send
	int lfs = -1;
	//current pos for write in the buffer
	int bufpos = 0;

	//use a struct to contains first 16byte which use to clarify the destination to troll and the data buffer
	struct sockaddr_in troll;
	struct Packet packet;

//	printf("tcp_header length:%ld\n",sizeof(packet.tcp_header));

	struct hostent *hp = gethostbyname(LOCALHOST);
	if (hp == 0) {
		fprintf(stderr, "%s:unknown host\n", LOCALHOST);
		exit(3);
	}

	troll.sin_family = AF_INET;
	bcopy((char *) hp->h_addr, (char *)&troll.sin_addr, hp->h_length);
	troll.sin_port = htons(TROLLC_PORT);

	struct hostent *hps = gethostbyname("sl5");
		if (hp == 0) {
			fprintf(stderr, "%s:unknown host\n", LOCALHOST);
			exit(3);
		}

	bcopy((char *) hps->h_addr, (char *)&packet.header.sin_addr, hps->h_length);
	packet.header.sin_port = htons(TCPDS_REMOTE_PORT);
	packet.header.sin_family = htons(AF_INET);

	int sock_ftpc;
	int sock_troll_server;

	char buf[BUFFSIZE]; /* buffer for holding read data */

	//for ftps
	struct sockaddr_in sin_addr_ftpc; /* structure for socket name setup */
	sin_addr_ftpc.sin_family = AF_INET;
	sin_addr_ftpc.sin_addr.s_addr = INADDR_ANY;
	sin_addr_ftpc.sin_port = htons(TCPDC_LOCAL_PORT);

	//for receive ack from troll runing on server
	struct sockaddr_in sin_addr_troll_server; /* structure for socket name setup */
	sin_addr_troll_server.sin_family = AF_INET;
	sin_addr_troll_server.sin_addr.s_addr = INADDR_ANY;
	sin_addr_troll_server.sin_port = htons(TCPDC_REMOTE_PORT);

	//addresss for receive ack from troll running on server side
	struct sockaddr_in troll_server;
	troll_server.sin_family = AF_INET;
	//change here if need to receive from a specific address
	troll_server.sin_addr.s_addr = INADDR_ANY;
	troll_server.sin_port = htons(TCPDC_REMOTE_PORT);

	/*initialize socket connection in unix domain*/
	if ((sock_ftpc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("error openting datagram socket");
		exit(1);
	}
	if ((sock_troll_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("error openting datagram socket");
			exit(1);
	}

	/* bind socket name to socket */
	if (bind(sock_ftpc, (struct sockaddr *) &sin_addr_ftpc, sizeof(struct sockaddr_in))< 0) {
		perror("error binding socket");
		exit(1);
	}
	if (bind(sock_troll_server, (struct sockaddr *) &sin_addr_troll_server, sizeof(struct sockaddr_in))< 0) {
			perror("error binding socket");
			exit(1);
	}

	/* Find assigned port value and print it for client to use */
	int namelen=sizeof(struct sockaddr_in);
	if (getsockname(sock_ftpc, (struct sockaddr *) &sin_addr_ftpc, &namelen) < 0) {
		perror("getting sock name");
		exit(3);
	}
	printf("Driver:tcpd waiting on port # %d\n", ntohs(sin_addr_ftpc.sin_port));
	fflush(stdout);
	packet.tcp_header.check = 0;
	bzero(packet.body, BUFFSIZE);

	int length = 0;

	char ackbuf[header_size];

	struct timeval timeout;
	//set timeout one second
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	u32 rtt = 0;
	u32 rto = TCP_RTO_MIN;
	int ttt=0;
	//receive and send data
	while (1) {

		//set of sockets descriptors
		fd_set set;
		FD_ZERO(&set);
		//set the sockets into set
		FD_SET(sock_ftpc,&set);
		FD_SET(sock_troll_server,&set);
		FD_SET(pfds_back[0], &set);
		int res = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

		if(res<0){
			perror("select error");
			exit(1);
		}

		else{
			//timeout resend
			if (FD_ISSET(pfds_back[0], &set)) {
					printf("Driver:recv resend request from timer\n");
					int resend_id; //
					char operation[OPT_SIZE];
					int nread = read(pfds_back[0], bufin, BUFFER_SIZE);
					strncpy(operation, bufin, sizeof(operation));
					printf("Driver:operation is :%s\n", operation);
					if (strcmp(operation, "resend") == 0) {

						memcpy((char*) &resend_id, bufin + OPT_SIZE,
								sizeof(int));
						// resend(resend_id);
						packet.tcp_header.seq = resend_id;
						printf("Driver:resend: %d\n", resend_id);
						memcpy((void*) &packet.body,
								(const void *) &sendbuf
										+ (packet.tcp_header.seq) * BUFFSIZE,
								BUFFSIZE);

						// rtt was initialated with 0;
						// estimate rto
						tcp_rtt_estimator(tp, rtt);
						// printf("tp->srtt: %d\n", tp->srtt);
						// printf("rto is else: %d, ", __tcp_set_rto(tp));
						fflush(stdout);
						rto = __tcp_set_rto(tp);
						// set send time
						struct timeval now;
						gettimeofday(&now, NULL);
						// store send_time in urg
						// packet.tcp_header.urg = now; //??
						//memcpy((void *)&packet+34, (void*)&now, sizeof(struct timeval));
						packet.time_stamp = now;
						// printf("resend time = %ld.%d\n", now.tv_sec, now.tv_usec);
						fflush(stdout);
						//calculate checksum
						int nBytes = sizeof(packet) - sizeof(packet.header);
						uint8_t const message[nBytes];
						memcpy((void*) &message, (const void *) &packet + 16,
								nBytes);
						crc checksum = crcFast(message, nBytes);
						// printf("packet checksum:%d\n", checksum);
						fflush(stdout);
						packet.tcp_header.check = checksum;

						int result = sendto(sock_ftpc, (char *) &packet, sizeof(packet), 0, (struct sockaddr *) &troll, sizeof(troll));
						// start a timer for id = packet.tcp_header.seq
						printf("Driver:starttimer in resend\n");
						fflush(stdout);
						///////////////////////////////////////
						starttimer((double) rto/1000000,packet.tcp_header.seq);

						//printf("result length: %d\n",result);
						if (result < 0) {
							perror("send to troll");
							exit(1);
						}

						bzero((void *) &packet.tcp_header, 20);
						bzero((void *) &packet.time_stamp,
								sizeof(struct timeval));
						bzero(packet.body, BUFFSIZE);
					}
					else {
						printf("Driver:miss send operation: %s\n", operation);
						fflush(stdout);
					}
			}

			//recv from ftpc
			if(FD_ISSET(sock_ftpc,&set)){

				ttt++;
				//if (ttt > 10) exit(0);
				printf("stay in ftpc\n");
				//  printf("lar before:%d\n",lar);
				//  printf("lfs before:%d\n",lfs);
				//  printf("bufpos before:%d\n",bufpos);
				//if current bufpos is out of the window, then we can add the packet in the buffer
				if( ((lfs>=lar)&& ((bufpos>lfs) || bufpos<=lar)) || ((lfs<lar)&& (bufpos<=lar) && (bufpos>lfs) )){
				{
					printf("Driver:recv from ftpc\n");
					ttt = 0;
				}
				
				fflush(stdout);
					int size = sizeof(sin_addr_ftpc);
					length = recvfrom(sock_ftpc,sendbuf+bufpos*BUFFSIZE,BUFFSIZE,0,(struct sockaddr *)&sin_addr_ftpc, &size);
					//printf("test point 1:\n");
					if (length < 0) {
						perror("recv from ftpc");
						exit(1);
					}

					bufpos++;
					//wrap around buffer
					if(bufpos== SEND_RECV_BUF_SIZE)
						bufpos = 0;
				}
			}

			if(FD_ISSET(sock_troll_server,&set)){
				printf("recv ack from troll running on the server\n");
				int size = sizeof(sin_addr_troll_server);
				//receive ack from troll running on the server side
				int acklength = recvfrom(sock_troll_server, ackbuf, header_size, 0,(struct sockaddr *) &sin_addr_troll_server, &size);
				//printf("test point 1:\n");
				if (acklength != header_size) {
					perror("recv ack from troll running on the server");
					exit(1);
				}

				crc original;
				memcpy((void *)&original,(void*)&ackbuf+32,2);

				bzero((void*)&ackbuf+32,2);
				uint8_t const ackcheck[header_size-16];
				memcpy((void*)&ackcheck,ackbuf+16,header_size-16);
				crc remainder = crcFast(ackcheck,header_size-16);
				//printf("original ack checksum:%d\n",original);
				//printf("packet ack checksum:%d\n",remainder);
				if(original==remainder){
					int ack = 0;
					//get ack number
					memcpy((void *)&ack,(void*)&ackbuf+24,4);

					// struct timeval now_test;
					// gettimeofday(&now_test,NULL);
					printf("Driver:recv ack number: %d \n",ack);
					// fflush(stdout);

					// cancel timer id = ack
					canceltimer(ack);
					printf("test point 000");
					struct timeval recv_time, send_time;
					printf("test point 00");
					gettimeofday(&recv_time, NULL);
printf("test point 0");
					// get send_time back from packet
					struct {
						struct sockaddr_in header;
						struct tcphdr tcp_header;
						struct timeval time_stamp;
					} ack_pac;
					memcpy((void*)&ack_pac,(const void *)&ackbuf,sizeof(ack_pac));
					send_time = ack_pac.time_stamp;
					// printf("send time2  = %ld.%d\n", send_time.tv_sec, send_time.tv_usec);
					// printf("recv_time = %ld.%d\n", recv_time.tv_sec, recv_time.tv_usec);
					fflush(stdout);
					// recv_time - send_time
					rtt = calculate_rtt(recv_time, send_time);
					
					
					//bzero the location of the ack number
					bzero(sendbuf+BUFFSIZE*ack,BUFFSIZE);
					//find the correct pos to update the lar
					//find the first non-empty block
					printf("test point 1");
					int i;
					for(i = lar+1;i<=(lfs>=lar)?lfs:SEND_RECV_BUF_SIZE-1;i++){
						int empty = checkEmpty(sendbuf+i*BUFFSIZE,BUFFSIZE);
						if(empty==0)
							break;
					}
					lar = i-1;
					//wrap around
					if (i == SEND_RECV_BUF_SIZE) {
						for (i = 0; i <= lfs; i++) {
							int empty = checkEmpty(sendbuf + i * BUFFSIZE,BUFFSIZE);
							if (empty == 0)
								break;
						}
						if(i!=0)
							lar = i-1;
					}
					printf("lar after ack:%d\n",lar);
					printf("lfs after ack:%d\n",lfs);
					printf("bufpos after ack:%d\n",bufpos);
					//add the code here to delete node in timer
				}
				else{
					printf("Driver:Ack packet is garbled!\n");
					fflush(stdout);
				}
			}
			//add another if here to detect timeout from timer and retransmission
		}

		//if lfs-lar<sws, then we can send the next packet in the window
		if(((lfs>=lar) && (lfs-lar< WINDOW) && ((lfs+1<bufpos)||(bufpos<=lar))) || ((lfs<lar)&& (63-lar+lfs<WINDOW) && (lfs+1)<bufpos)){
			//send the next packet
			packet.tcp_header.seq = lfs+1;
			memcpy((void*)&packet.body,(const void *)&sendbuf+(lfs+1)*BUFFSIZE,BUFFSIZE);
			/* fill buffer with data */
			//printf("message length: %d\n",length);


			// rtt was initialated with 0;
			// estimate rto
			tcp_rtt_estimator(tp, rtt);
			// printf("tp->srtt: %d\n", tp->srtt);
			// printf("rto is: %d, ", __tcp_set_rto(tp));
			fflush(stdout);
			rto = __tcp_set_rto(tp);
			// set send time
			struct timeval now;
			gettimeofday(&now, NULL);
			// store send_time in urg
			// packet.tcp_header.urg = now; //??
			//memcpy((void *)&packet+34, (void*)&now, sizeof(struct timeval));
			packet.time_stamp = now;
			
			// printf("send time = %ld.%d\n", now.tv_sec, now.tv_usec);
			//calculate checksum
			int nBytes = sizeof(packet) - sizeof(packet.header);
			uint8_t const message[nBytes];
			memcpy((void*)&message,(const void *)&packet+16,nBytes);
			crc checksum = crcFast(message,nBytes);
			// printf("packet checksum:%d\n",checksum);
			fflush(stdout);
			//checksum = checksum ^ 0xffff;
			packet.tcp_header.check = checksum;

			int result = sendto(sock_ftpc, (char *) &packet,sizeof(packet), 0,(struct sockaddr *)&troll, sizeof(troll));
			// start a timer for id = packet.tcp_header.seq
			printf("Driver:starttimer after send %d\n", lfs+1);
			fflush(stdout);
			starttimer((double)rto/1000000, packet.tcp_header.seq);

			//printf("result length: %d\n",result);
			if (result < 0) {
				perror("send to troll");
				exit(1);
			}

			//increase lfs;
			lfs++;
			if(lfs==SEND_RECV_BUF_SIZE-1)
				lfs=-1;
			printf("lar:%d\n",lar);
				printf("lfs:%d\n",lfs);
				printf("bufpos:%d\n",bufpos);
		}
//		//printf("%s\n",buf);
//		if (strcmp(packet.body, "end") == 0)
//			break;
		//packet.tcp_header.check = 0;
		bzero((void *)&packet.tcp_header, 20);
		bzero((void *)&packet.time_stamp, sizeof(struct timeval));
		bzero(packet.body, BUFFSIZE);
	}
	close(sock_ftpc);
	close(sock_troll_server);
	exit(0);
	}
}

void server() {

	struct {
		struct sockaddr_in header;
		struct tcphdr tcp_header;
		struct timeval time_stamp;
	} ack;

	char recvbuf[SEND_RECV_BUF_SIZE*BUFFSIZE];
	char buf[sizeof(struct Packet)]; /* buffer for holding data from troll*/

	//largest frame received
	int lfr = -1;
	//largetst acceptable frame
	int laf = lfr+WINDOW;

	struct hostent *hp = gethostbyname("sl1");
	if (hp == 0) {
		fprintf(stderr, "%s:unknown host\n", LOCALHOST);
		exit(3);
	}

		struct hostent *hps = gethostbyname("sl1");
		if (hp == 0) {
			fprintf(stderr, "%s:unknown host\n", LOCALHOST);
			exit(3);
		}

	//use a struct to contains first 16 byte which use to clarify the destination to troll
	struct sockaddr_in troll;
	troll.sin_family = AF_INET;
	bcopy((char *) hps->h_addr, (char *)&troll.sin_addr, hps->h_length);
	troll.sin_port = htons(TROLLS_PORT);

	struct sockaddr_in ftps;
	ftps.sin_family = AF_INET;
	bcopy((char *) hp->h_addr, (char *)&ftps.sin_addr, hp->h_length);
	ftps.sin_port = htons(FTPS_PORT);

	int sock;
	struct sockaddr_in sin_addr; /* structure for socket name setup */
	sin_addr.sin_family = AF_INET;
	sin_addr.sin_addr.s_addr = INADDR_ANY;
	sin_addr.sin_port = htons(TCPDS_REMOTE_PORT);

	/*initialize socket connection in unix domain*/
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("error openting datagram socket");
		exit(1);
	}

	/* bind socket name to socket */
	if (bind(sock, (struct sockaddr *) &sin_addr, sizeof(struct sockaddr_in))< 0) {
		perror("error binding stream socket");
		exit(1);
	}

	// set ack header for troll
	bcopy((char *) hp->h_addr, (char *)&ack.header.sin_addr, hp->h_length);
	ack.header.sin_port = htons(TCPDC_REMOTE_PORT);
	ack.header.sin_family = htons(AF_INET);

	int len = sizeof(sin_addr);
	//printf("test point 0:\n");
	bzero(buf, header_size+BUFFSIZE);
	bzero(recvbuf,SEND_RECV_BUF_SIZE*BUFFSIZE);
	//receive and send data

	while (1) {
		int length = recvfrom(sock, buf, sizeof(struct Packet), 0,(struct sockaddr *) &sin_addr, &len);
		//printf("test point 1:\n");
		//printf("packet length:%d\n",length);
		if (length < 0) {
			perror("recv from troll");
			exit(1);
		}

		//calculate checksum
		int nBytes = sizeof(struct Packet) - sizeof(struct sockaddr_in);
		int tim = sizeof(struct timeval);
		printf("nBytes:%d\n",nBytes);
//		printf("timeval:%d\n",tim);
//		printf("Packet:%ld\n",sizeof(struct Packet));
//		printf("header size:%d\n",header_size);
		uint8_t const message[nBytes];
		memcpy((void*)&message,buf+sizeof(struct sockaddr_in),nBytes);

		//get the seqNum
		int seqNum = 0;
		memcpy((void *)&seqNum,(void*)&message+4,4);

		//get time_stamp
		// struct timeval time_stamp;
		// memcpy((void *)&time_stamp,(void*)&message+sizeof(struct tcphdr),sizeof(struct timeval));

		struct Packet pac;
		struct timeval time_stamp;
		memcpy((void *)&pac,(void*)&buf,sizeof(struct Packet));
		time_stamp = pac.time_stamp;

		crc original;
		memcpy((void *)&original,(void*)&message+16,2);
		//printf("original checksum:%d\n",original);
		bzero((void*)&message+16,2);
		//bzero((void*)&message+sizeof(struct tcphdr),16);
		crc remainder = crcFast(message,nBytes);
		//printf("packet checksum:%d\n",remainder);

		if(remainder!=original)
			printf("%s\n","This packet was garbled");
		//if checksum is correct
		else{
			//only if seqNum is >lfr and <= laf, we accept the packet, store in the recv buffer
			printf("laf:%d\n",laf);
			printf("lfr:%d\n",lfr);
			printf("seqNum:%d\n",seqNum);
			if(((laf>=lfr && seqNum>lfr && seqNum <=laf)) || ((laf<lfr) && (seqNum>lfr || seqNum <= laf))) {
				//printf("test point 1:\n");
				//skip duplicate
				char temp[BUFFSIZE];
				memcpy((void *)&temp,(void*)&recvbuf+seqNum*BUFFSIZE,BUFFSIZE);
				int isEmpty = checkEmpty(temp,BUFFSIZE);
				printf("isEmpty:%d\n",isEmpty);
				if(isEmpty){
					memcpy((void*)&recvbuf+BUFFSIZE*seqNum,(void*)&buf+header_size,length-header_size);

//					/* send data to ftps */
//					int result = sendto(sock, buf+header_size, length-header_size, 0,(struct sockaddr *)&ftps, sizeof(ftps));
//					if (result < 0) {
//						perror("send to ftps");
//						exit(1);
//					}

					//should keep order, only send lfr+1
					//find the correct pos to update the lfr
					//find the first empty block
					if(laf>lfr){
						int i =0;
						for(i = lfr+1;i<=laf;i++){
							int num = checkEmpty(recvbuf+i*BUFFSIZE,BUFFSIZE);
							if(num==1)
								break;
							int result = sendto(sock, recvbuf+i*BUFFSIZE,length - header_size, 0,(struct sockaddr *) &ftps, sizeof(ftps));
							if (result < 0) {
								perror("send to ftps");
								exit(1);
							}
						}
						if(i != lfr+1){
							bzero(recvbuf+(lfr+1)*BUFFSIZE,(i-lfr-1)*BUFFSIZE);
							//update lfr
							lfr = i-1;
							if(lfr>63)
								lfr = -1;
							//update laf
							laf = lfr + WINDOW;
							if(laf>63){
								laf = laf-64;
							}
						}
					}
					else if(laf<lfr){
						int i =0;
						for(i = lfr+1;i< SEND_RECV_BUF_SIZE;i++){
							int num = checkEmpty(recvbuf+i*BUFFSIZE,BUFFSIZE);
							if (num == 1)
								break;
							int result = sendto(sock, recvbuf+i*BUFFSIZE,length - header_size, 0,(struct sockaddr *) &ftps, sizeof(ftps));
							if (result < 0) {
								perror("send to ftps");
								exit(1);
							}
						}
						if(i != lfr+1){
							bzero(recvbuf+(lfr+1)*BUFFSIZE,(i-lfr-1)*BUFFSIZE);

							if(i<SEND_RECV_BUF_SIZE){
								laf += (i-lfr-1);
								lfr = i-1;
							}
							//wrap around
							else{
								for (i = 0; i <= laf; i++) {
									int num = checkEmpty(recvbuf+i*BUFFSIZE,BUFFSIZE);
									if (num == 1)
										break;
									int result = sendto(sock, recvbuf+i*BUFFSIZE,length - header_size, 0,(struct sockaddr *) &ftps, sizeof(ftps));
									if (result < 0) {
										perror("send to ftps");
										exit(1);
									}
								}
								if(i!=0){
									bzero(recvbuf,i*BUFFSIZE);
									lfr = i-1;
								}
								else
									lfr = -1;
								laf = lfr + WINDOW;
							}
						}
					}

				}
			}
				//printf("seqNum:%d\n",seqNum);
				ack.tcp_header.ack_seq = seqNum;
				ack.time_stamp = time_stamp;

				uint8_t const ackcheck[header_size - 16];
				memcpy((void*) &ackcheck, &ack.tcp_header, header_size - 16);
				crc checksum = crcFast(ackcheck, header_size - 16);
				ack.tcp_header.check = checksum;
				//printf("ack packet size: %ld\n",sizeof(ack));
				printf("send ack number: %d\n", seqNum);
				//printf("ack checksum:%d\n",checksum);

				//sent ack to troll running on server
				int res = sendto(sock, (char *) &ack, header_size, 0,
						(struct sockaddr *) &troll, sizeof(troll));
				if (res < 0) {
					perror("tcpd on server send to troll");
					exit(1);
				}
		}
		//		//printf("%s\n",buf+16);
		//		if (strcmp(buf+16, "end") == 0)
		//			break;
		bzero(buf, sizeof(struct Packet));
		bzero((void*)&ack.tcp_header,20);
		bzero((void*)&ack.time_stamp,sizeof(struct timeval));
	}
	close(sock);
	exit(0);
}


//http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
void crcInit()
{
    crc  remainder;
    /*
     * Compute the remainder of each possible dividend.
     */
    int dividend;
    for (dividend = 0; dividend < 256; ++dividend)
    {
        /*
         * Start with the dividend followed by zeros.
         */
        remainder = dividend << (WIDTH - 8);

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        uint8_t bit;
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }

        /*
         * Store the result into the table.
         */
        crcTable[dividend] = remainder;
        //if(remainder==6704)
        	//printf("%d\n",crcTable[dividend]);
    }

}   /* crcInit() */

main(int argc, char *argv[]) {
	crcInit();
	if (strcmp(argv[1], "client") == 0)
		client();
	else if (strcmp(argv[1], "server") == 0)
		server();
	else {
		printf("usage: ftpd client/server\n");
		exit(1);
	}
}

void resend(int id)
{
	printf("resend is: %d\n", id);
	fflush(stdout);
}

void starttimer(double time_interval, int id)
{
	printf("\nstarttimer: id=%d, time_interval=%lf\n", id, time_interval);
	fflush(stdout);
	//print_time_list(head);
	fflush(stdout);
	char operation[OPT_SIZE] = "start"; 
	char buf[INFO_SIZE] = "This is a test string";
	char bufout[BUFFER_SIZE];
	bzero(bufout, BUFFER_SIZE);
	strncpy(bufout, operation, sizeof(operation)); //operation
	memcpy(bufout + OPT_SIZE, (char*)&id, sizeof(int));
	// strncpy(bufout + OPT_SIZE, (char*) &id, ID_SIZE); //ID
	memcpy(bufout + OPT_SIZE + ID_SIZE, (char*)&time_interval, sizeof(double));
	// strncpy(bufout + OPT_SIZE + ID_SIZE, (char*) &time_interval, sizeof(double)); //TIME
	strncpy(bufout + OPT_SIZE + ID_SIZE + sizeof(double), buf, INFO_SIZE);
	write(pfds[1], bufout, BUFFER_SIZE);
	// printf("call write at starttimer\n");
	// char bufin[BUFFER_SIZE];
	// read(pfds[0], bufin, BUFFER_SIZE);
	// char operation_in[OPT_SIZE];
	// strncpy(operation_in, bufin, sizeof(operation)); //operation
	// printf("read after write%s\n", operation_in);
	// fflush(stdout);
}

void canceltimer(int id)
{
	printf("canceltimer: id=%d\n", id);
	fflush(stdout);
	fflush(stdout);
	char operation[OPT_SIZE] = "cancel";
	// char buf[INFO_SIZE] = "This is a test string";
	char bufout[BUFFER_SIZE];
	bzero(bufout, BUFFER_SIZE);
	strncpy(bufout, operation, sizeof(operation));
	strncpy(bufout + OPT_SIZE, (char*) &id, ID_SIZE);
	printf("call write at canceltimer to cancel %d\n", id);
	fflush(stdout);
	write(pfds[1], bufout, BUFFER_SIZE);
	printf("end cancel");
}

void stop_timing()
{
	char operation[OPT_SIZE] = "stop";
	char bufout[BUFFER_SIZE];
	strncpy(bufout, operation, sizeof(operation));
	write(pfds[1], bufout, BUFFER_SIZE);
}

int find_timenode(TimeNode *head, int id)
{
	TimeNode *node = head;
	while(node)
	{
		if (node->id == id)
		{
			return 1;
		}
		else
			node = node->next;
	}
	return 0;
}

TimeNode *add_timenode(TimeNode *head, int id, double dtime, int pnum, char * info)
{
	// printf("head is %p at the beginning of add_timenode\n", head);
	// fflush(stdout);
	if (find_timenode(head, id))
	{
		printf("Error in adding the duplicate id.\n");
		fflush(stdout);
		return head;
	}
	TimeNode *node = head;
	// printf("in add head id = %d dtime = %ld.%d\n", head->id, head->delta_time.tv_sec, head->delta_time.tv_usec);
	TimeNode *node_to_add = (TimeNode *)malloc(sizeof(TimeNode));
	node_to_add->id = id;
	node_to_add->delta_time.tv_sec = (time_t)dtime;
	node_to_add->delta_time.tv_usec = dtime * 1000000 - (time_t)dtime * 1000000;

	// printf("read time:%ld.%ld\n", node_to_add->delta_time.tv_sec, node_to_add->delta_time.tv_usec);

	node_to_add->port_num = pnum;
	// printf("node_to_add is :%p\n", &node_to_add);
	// strcpy(node_to_add->info, info);
	if (!node)
	{
		gettimeofday(&lasttime, NULL);
		// printf("add first node\n");
		node = node_to_add;
		head = node_to_add;
		// printf("node_to_add is :%p\n", &node_to_add);
		// printf("head is %p\n", head);
		return head;
	}

	// add at first position
	struct timeval r;
	if(timeval_subtract(&r, &node_to_add->delta_time, &node->delta_time))
	{
		// printf("add at first\n");
		node_to_add->next = node;
		node->forward = node_to_add;
		head = node_to_add;
		timeval_subtract(&node->delta_time, &node->delta_time, &node_to_add->delta_time);
		//print_time_list(head);
		return head;
	}

	// find the insert point
	while(node)
	{

		// printf("finding position\n");
		fflush(stdout);
		// printf("node_to_add = %ld.%d\n", node_to_add->delta_time.tv_sec, node_to_add->delta_time.tv_usec);
		if (!node->next && !timeval_subtract(&r, &node_to_add->delta_time, &node->delta_time))
		{ // add at the end
			// node_to_add->delta_time -= node->delta_time;
			timeval_subtract(&node_to_add->delta_time, &node_to_add->delta_time, &node->delta_time);
			node->next = node_to_add;
			node_to_add->forward = node;
			// printf("node_to_add = %ld.%d\n", node_to_add->delta_time.tv_sec, node_to_add->delta_time.tv_usec);
			break;
		}
		if (timeval_subtract(&r, &node_to_add->delta_time, &node->delta_time))
		{ // add before node
			node->forward->next = node_to_add;
			node_to_add->forward = node->forward;
			node_to_add->next = node;
			node->forward = node_to_add;
			// node->delta_time -= node_to_add->delta_time;
			timeval_subtract(&node->delta_time, &node->delta_time, &node_to_add->delta_time);
			break;
		}
		if (node->next && !timeval_subtract(&r, &node_to_add->delta_time, &node->delta_time))
		{ // move on
			// node_to_add->delta_time -= node->delta_time;
			timeval_subtract(&node_to_add->delta_time, &node_to_add->delta_time, &node->delta_time);
			node = node->next;
		}
	}
	//print_time_list(head);
	return head;
}

void print_time_list(TimeNode *head)
{
	// printf("call add_timenode\n");
	if (!head)
	{
		printf("()\n");
		fflush(stdout);
	}
	while(head)
	{
		// printf("id: %d, delta_time: %d\n", head->id, head->delta_time);
		if (head->delta_time.tv_usec < 0)
		{
			head->delta_time.tv_sec -= 1;
			head->delta_time.tv_usec += 1000000;
		}
		double t = 0.0;
		t += (double)head->delta_time.tv_sec + (double)head->delta_time.tv_usec / 1000000;
		// printf("(%d: %ld.%d)", head->id, head->delta_time.tv_sec, (int)head->delta_time.tv_usec);
		printf("(%d: %lf)", head->id, t);
		fflush(stdout);
		head = head->next;
	}
	printf("\n");
	fflush(stdout);
}

TimeNode *delete_timenode(TimeNode *head, int id) // 30 32 46 ->44
{
	
	printf("Before delete time node:");
	printf("delete %d\n", id);
	//print_time_list(head);
	fflush(stdout);
	if (!find_timenode(head, id))
	{
		printf("Error in deleting the noexist id.\n");
		fflush(stdout);
		
		printf("after delete time node:");
		//print_time_list(head);
		
		return head;
	}
	TimeNode *node = head;
	struct timeval dtime;
	if (head->id == id)
	{
		dtime = head->delta_time;
		head = head->next;
		// head -> forward = NULL;
		// free(node);
		// if(head) head->delta_time += dtime;
		if(head) timeval_add(&head->delta_time, &head->delta_time, &dtime);
		// print_time_list(head);

		printf("after delete time node:");
		//print_time_list(head);

		return head;
	}
	while(node)
	{
		if (node->id == id)
		{
			node->forward->next = node->next;
			if(node->next)
				node->next->forward = node->forward;
			dtime = node->delta_time;
			// if(node->next) node->next->delta_time += dtime;
			if(node->next) timeval_add(&node->next->delta_time, &node->next->delta_time, &dtime);
			//print_time_list(head);
			// free(node);

			printf("after delete time node:");
			//print_time_list(head);

			return head;
		}
		node = node->next;
	}
	printf("Error: id error, trying to delete id that is not exist\n");
	fflush(stdout);
	// print_time_list(head);
	return head;
	// while()
}

TimeNode *update_time_list(TimeNode *head)
{
	// printf("in update_time_list\n");
	fflush(stdout);
	struct timeval now, dtime, r;
	gettimeofday(&now, NULL);
	// int dtime = now - lasttime;
	if (lasttime.tv_sec == 0 && lasttime.tv_usec == 0)
	{
		dtime = lasttime;
	}
	else
		timeval_subtract(&dtime, &now, &lasttime);
	// printf("update: lasttime is %ld.%d, dtime is %ld.%d\n", lasttime.tv_sec, (int)lasttime.tv_usec, dtime.tv_sec, (int)dtime.tv_usec);
	lasttime = now;
	// TimeNode *node = head;
	// while(head && dtime >= head->delta_time)
	if (head)
	{
		// printf("in if(head)\n");
		// print_time_list(head);
		fflush(stdout);
	}
	while(head && !timeval_subtract(&r, &dtime, &head->delta_time))
	{
		// printf("head move\n");
		// dtime -= head->delta_time;

		char bufout[BUFFER_SIZE];
		char operation[OPT_SIZE] = "resend";
		bzero(bufout, BUFFER_SIZE);
		// strncpy(bufout, operation, sizeof(operation)); //operation

		strncpy(bufout, operation, sizeof(operation));
		memcpy(bufout + OPT_SIZE, (char*)&head->id, sizeof(int));
		// memcpy(bufout + OPT_SIZE + ID_SIZE, (char*)&time_interval, sizeof(double));
		// strncpy(bufout + OPT_SIZE + ID_SIZE + sizeof(double), buf, INFO_SIZE);
		write(pfds_back[1], bufout, BUFFER_SIZE); // send resend-request back to driver
		printf("write back to request resend\n");
		fflush(stdout);

		timeval_subtract(&dtime, &dtime, &head->delta_time);
		head = head->next; // how to destroy node that will not be used anymore?
	}
	if (head)
	{
		// head->delta_time -= dtime;
		timeval_subtract(&head->delta_time, &head->delta_time, &dtime);
	}
	return head;
}
/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void timeval_add (struct timeval *result, struct timeval *x, struct timeval *y)
{
  result->tv_sec = x->tv_sec + y->tv_sec;
  result->tv_usec = x->tv_usec + y->tv_usec;
}

// calculate x - y
u32 calculate_rtt (struct timeval x, struct timeval y)
{
	u32 s = 0;
	if (x.tv_usec < y.tv_usec) 
	{
		s = x.tv_usec + 1000000 - y.tv_usec + 1000000 * (x.tv_sec - 1 - y.tv_sec);
	}
	else
		s = x.tv_usec - y.tv_usec + 1000000 * (x.tv_sec - y.tv_sec);
	// printf("rtt = %ld.%d - %ld.%d = %d\n", x.tv_sec, x.tv_usec, y.tv_sec, y.tv_usec, s);
	// fflush(stdout);
	return s;
}

void syserr(char* msg)
{
	fprintf(stderr, "%s: %s\n", strerror(errno), msg);
	abort();
}
