#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/time.h> 
#include <fcntl.h>
#include <sys/ioctl.h> 
#include <unistd.h>

#define BUFFER_SIZE 1200
#define INFO_SIZE 1000
#define OPT_SIZE 10
#define ID_SIZE 4
#define TIME_SIZE 8

extern int errno;
void syserr(char*);

typedef struct tnode{ 
	int id;
	int delta_time;
	int port_num;
	char info[INFO_SIZE];
	struct tnode *forward;
	struct tnode *next;
} TimeNode;

void add_timenode(TimeNode head, int id, int dtime, int pnum, char * info);
TimeNode delete_timenode(TimeNode head, int pnum);
void starttimer(double, int);
void canceltimer(int);

int pfds[2];

int main(int argc, char *argv[])
{
	pid_t pid;
	int rc;
	char buf[BUFFER_SIZE];
	pid = getpid();
	sprintf(buf, "process pid is %d\n", pid);
	// sprintf(buf, "This line is from pid %d, value = %d\n", pid, i);
	write(1, buf, strlen(buf));
	
	
	pipe(pfds);

	if (!fork()) { // timer
		int result, nread;
		fd_set inputs; 
    	struct timeval timeout; 
    	FD_ZERO(&inputs);//用select函数之前先把集合清零  
    	FD_SET(pfds[0], &inputs);//把要检测的句柄——标准输入（0），加入到集合里
    	timeout.tv_sec = 5; 
        timeout.tv_usec = 500000;
        TimeNode head;
        char bufin[BUFFER_SIZE];
        char operation[OPT_SIZE];
        int id;
        double time_interval;
        int t = 4;

		while(t--)
		{
			result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);//&timeout); //
			printf("%d\n", t);
			switch(result)
			{
				case 0: 
					printf("Timer: timeout\n");
					// update_time_list(head, ); //
					break;
				case -1: 
					perror("select"); 
					exit(1); 
				default:
				if(FD_ISSET(pfds[0], &inputs)) 
				{
					nread = read(pfds[0], bufin, BUFFER_SIZE);
					strncpy(operation, bufin, sizeof(operation)); //operation
					memcpy((char*)&id, bufin + OPT_SIZE, sizeof(int));
					// strncpy((char *)&id, bufin + OPT_SIZE, ID_SIZE); //ID
					memcpy((char*)&time_interval, bufin + OPT_SIZE + ID_SIZE, sizeof(double));
					// strncpy((char *)&time_interval, bufin + OPT_SIZE + ID_SIZE, sizeof(double)); //TIME
					printf("operation: %s in %d time_interval: %lf", operation, id, time_interval); 
					
				} 
				break; 
			}
		}
		// printf(" CHILD: writing to the pipe\n");
		// sleep(5);
		// write(pfds[1], "test", 5);
		// printf(" CHILD: exiting\n");
		// exit(0);
	} else {
		char bufout[BUFFER_SIZE];
		strcpy(bufout, "hello from driver\n");
		// write(pfds[1], bufout, BUFFER_SIZE);
		starttimer(20.0,4);
		starttimer(10.0,2);
		starttimer(30.0,3);
		sleep(5);
		// canceltimer(2);
		// starttimer(20.0,4);
		// sleep(5);
		// starttimer(18.0,5);
		// canceltimer(4);
		// canceltimer(8);

		// printf("PARENT: reading from pipe\n");
		// read(pfds[0], buf, 5);
		// printf("PARENT: read \"%s\"\n", buf);
		// wait(NULL);
	}
}

void starttimer(double time_interval, int id)
{
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
}
void canceltimer(int id)
{
	char operation[OPT_SIZE] = "cancel";
	char buf[INFO_SIZE] = "This is a test string";
	char bufout[BUFFER_SIZE];
	bzero(bufout, BUFFER_SIZE);
	strncpy(bufout, operation, sizeof(operation));
	strncpy(bufout + OPT_SIZE, (char*) &id, ID_SIZE);
	write(pfds[1], bufout, BUFFER_SIZE);
}

void add_timenode(TimeNode head, int id, int dtime, int pnum, char * info)
{
	TimeNode node = head;

	// find the insert point
	// while(node && dtime > node.delta_time)
	{

	}

	// change delta_time of points afterwards
	// while()
	{

	}
}

TimeNode delete_timenode(TimeNode head, int id)
{
	TimeNode node = head;
	return node;
	// while()
}

void syserr(char* msg)
{
	fprintf(stderr, "%s: %s\n", strerror(errno), msg);
	abort();
}