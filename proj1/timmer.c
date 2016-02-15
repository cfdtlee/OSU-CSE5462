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

TimeNode *add_timenode(TimeNode *head, int id, int dtime, int pnum, char * info);
TimeNode *delete_timenode(TimeNode *head, int pnum);
void starttimer(double, int);
void canceltimer(int);
void print_time_list(TimeNode *head);
void stop_timing();
TimeNode *update_time_list(TimeNode *head);

int pfds[2];
time_t lasttime;

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
        TimeNode *head;
        char bufin[BUFFER_SIZE];
        char operation[OPT_SIZE];
        int id;
        double time_interval;
        int t = 8;
        

		while(t--)
		{
			result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);//&timeout); //
			// printf("%d\n", t);
			head = update_time_list(head); //
			switch(result)
			{
				case 0: 
					printf("Timer: timeout\n");
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
					printf("New operation: %s id %d\n:", operation, id); 
					
					if (strcmp(operation, "stop") == 0)
					{
						printf("stop\n");
					}
					if (strcmp(operation, "start") == 0)
					{
						
						head = add_timenode(head, id, time_interval, 0, NULL);
						// printf("head is after %p\n", head);
						// printf("head id = %d dtime = %d\n", head->id, head->delta_time);
						// if(head->next)
						// printf("head id = %d dtime = %d\n", head->next->id, head->next->delta_time);
					}
					if (strcmp(operation, "cancel") == 0)
					{
						// printf("cancel\n");
						head = delete_timenode(head, id);
					}
				} 
				break; 
			}
		}
	} else {
		// char bufout[BUFFER_SIZE];
		// strcpy(bufout, "hello from driver\n");
		// write(pfds[1], bufout, BUFFER_SIZE);
		starttimer(3.0,3);

		starttimer(30.0,2);
		starttimer(22.0,1);
		// sleep(5);
		starttimer(20.0,4);
		canceltimer(2);
		// stop_timing();
		
		sleep(5);
		printf("sleep 5 s\n");
		starttimer(18.0,5);
		canceltimer(4);
		canceltimer(8);

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

void stop_timing()
{
	char operation[OPT_SIZE] = "stop";
	char bufout[BUFFER_SIZE];
	strncpy(bufout, operation, sizeof(operation));
	write(pfds[1], bufout, BUFFER_SIZE);
}

TimeNode *add_timenode(TimeNode *head, int id, int dtime, int pnum, char * info)
{
	// printf("head is %p\n", head);
	TimeNode *node = head;
	TimeNode *node_to_add = (TimeNode *)malloc(sizeof(TimeNode));
	node_to_add->id = id;
	node_to_add->delta_time = dtime;
	node_to_add->port_num = pnum;
	// printf("node_to_add is :%p\n", &node_to_add);
	// strcpy(node_to_add->info, info);
	if (!node)
	{
		lasttime = time(NULL);
		// printf("add first node\n");
		node = node_to_add;
		head = node_to_add;
		// printf("node_to_add is :%p\n", &node_to_add);
		// printf("head is %p\n", head);
		return head;
	}

	// add at first position
	if(node_to_add->delta_time <= node->delta_time)
	{
		// printf("add at first\n");
		node_to_add->next = node;
		node->forward = node_to_add;
		head = node_to_add;
		node->delta_time -= node_to_add->delta_time;
		print_time_list(head);
		return head;
	}

	// find the insert point
	while(node)
	{
		// printf("finding position\n");
		if (!node->next && node_to_add->delta_time > node->delta_time)
		{ // add at the end
			node_to_add->delta_time -= node->delta_time;
			node->next = node_to_add;
			node_to_add->forward = node;
			break;
		}
		if (node_to_add->delta_time <= node->delta_time)
		{ // add before node
			node->forward->next = node_to_add;
			node_to_add->forward = node->forward;
			node_to_add->next = node;
			node->forward = node_to_add;
			node->delta_time -= node_to_add->delta_time;
			break;
		}
		if (node->next && node_to_add->delta_time > node->delta_time)
		{ // move on
			node_to_add->delta_time -= node->delta_time;
			node = node->next;
		}
	}
	print_time_list(head);
	return head;
}

void print_time_list(TimeNode *head)
{
	// printf("call add_timenode\n");
	while(head)
	{
		printf("id: %d, delta_time: %d\n", head->id, head->delta_time);
		head = head->next;
	}
}

TimeNode *delete_timenode(TimeNode *head, int id)
{
	TimeNode *node = head;
	int dtime;
	if (head->id == id)
	{
		dtime = head->delta_time;
		head = head->next;
		if(head) head->delta_time += dtime;
		print_time_list(head);
		return head;
	}
	while(node)
	{
		if (node->id == id)
		{
			node->forward->next = node->next;
			dtime = node->delta_time;
			if(node->next) node->next->delta_time += dtime;
			print_time_list(head);
			return head;
		}
		node = node->next;
	}
	printf("error: id error\n");
	print_time_list(head);
	return head;
	// while()
}
TimeNode *update_time_list(TimeNode *head)
{
	time_t now = time(NULL);
	int dtime = now - lasttime;
	printf("lasttime is %ld, dtime is %d\n", lasttime, dtime);
	lasttime = now;
	// TimeNode *node = head;
	while(head && dtime >= head->delta_time)
	{
		dtime -= head->delta_time;
		head = head->next; // how to destroy node that will not be used anymore?
	}
	if (head)
	{
		head->delta_time -= dtime;
	}
	return head;
}
void syserr(char* msg)
{
	fprintf(stderr, "%s: %s\n", strerror(errno), msg);
	abort();
}