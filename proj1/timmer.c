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

int pfds[2];
struct timeval lasttime;

int main(int argc, char *argv[])
{
	pid_t pid;
	// int rc;
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
        // int t = 8;
        

		while(1)
		{
			result = select(FD_SETSIZE, &inputs, (fd_set *)0, (fd_set *)0, &timeout);//&timeout); //
			// printf("%d\n", t);
			head = update_time_list(head); //
			if (result == 0)
			{
				printf("Timer: timeout\n");
				break;
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
					nread = read(pfds[0], bufin, BUFFER_SIZE);
					strncpy(operation, bufin, sizeof(operation)); //operation
					memcpy((char*)&id, bufin + OPT_SIZE, sizeof(int));
					// strncpy((char *)&id, bufin + OPT_SIZE, ID_SIZE); //ID
					memcpy((char*)&time_interval, bufin + OPT_SIZE + ID_SIZE, sizeof(double));
					// strncpy((char *)&time_interval, bufin + OPT_SIZE + ID_SIZE, sizeof(double)); //TIME
					printf("New operation: %s id %d\n:", operation, id); 
					
					if (strcmp(operation, "stop") == 0)
					{
						printf("stop\n"); //
						break;
					}
					if (strcmp(operation, "start") == 0)
					{
						
						head = add_timenode(head, id, time_interval, 0, NULL);
						// printf("head is after %p\n", head);
						// printf("head id = %d dtime = %ld.%d\n", head->id, head->delta_time.tv_sec, head->delta_time.tv_usec);
						// if(head->next)
						// 	printf("head id = %d dtime = %ld.%d\n", head->next->id, head->next->delta_time.tv_sec,  head->next->delta_time.tv_usec);
						// else printf("no next\n");
					}
					if (strcmp(operation, "cancel") == 0)
					{
						// printf("cancel\n");
						head = delete_timenode(head, id);
					}
				}
				 
			}
		}
	} else { // driver
		starttimer(3.123,3); // (dtime, id)
		starttimer(3.123,3); // (dtime, id)
		starttimer(30.456,2);
		starttimer(22.0,1);
		// sleep(5);
		starttimer(20.0,4);
		canceltimer(2);
		sleep(5);
		printf("sleep 5 s\n");
		starttimer(18.0,5);
		canceltimer(4);
		canceltimer(8);
		stop_timing();
	}
	return 0;
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
	// char buf[INFO_SIZE] = "This is a test string";
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

int find_timenode(TimeNode *head, int id)
{
	while(head)
	{
		if (head->id == id)
		{
			return 1;
		}
		else
			head = head->next;
	}
	return 0;
}

TimeNode *add_timenode(TimeNode *head, int id, double dtime, int pnum, char * info)
{
	// printf("head is %p\n", head);
	if (find_timenode(head, id))
	{
		printf("Error in adding the duplicate id.\n");
		return head;
	}
	TimeNode *node = head;
	// printf("in add head id = %d dtime = %ld.%d\n", head->id, head->delta_time.tv_sec, head->delta_time.tv_usec);
	TimeNode *node_to_add = (TimeNode *)malloc(sizeof(TimeNode));
	node_to_add->id = id;
	node_to_add->delta_time.tv_sec = (time_t)dtime;
	node_to_add->delta_time.tv_usec = dtime * 1000000 - (time_t)dtime * 1000000;

	// printf("read time:%ld.%d\n", node_to_add->delta_time.tv_sec, node_to_add->delta_time.tv_usec);

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
		printf("head is %p\n", head);
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
		print_time_list(head);
		return head;
	}

	// find the insert point
	while(node)
	{
		// printf("finding position\n");
		if (!node->next && !timeval_subtract(&r, &node_to_add->delta_time, &node->delta_time))
		{ // add at the end
			// node_to_add->delta_time -= node->delta_time;
			timeval_subtract(&node_to_add->delta_time, &node_to_add->delta_time, &node->delta_time);
			node->next = node_to_add;
			node_to_add->forward = node;
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
	print_time_list(head);
	return head;
}

void print_time_list(TimeNode *head)
{
	// printf("call add_timenode\n");
	while(head)
	{
		// printf("id: %d, delta_time: %d\n", head->id, head->delta_time);
		if (head->delta_time.tv_usec < 0)
		{
			head->delta_time.tv_sec -= 1;
			head->delta_time.tv_usec += 1000000;
		}
		printf("id: %d, delta_time: %ld.%d\n", head->id, head->delta_time.tv_sec, (int)head->delta_time.tv_usec);
		head = head->next;
	}
}

TimeNode *delete_timenode(TimeNode *head, int id)
{
	TimeNode *node = head;
	struct timeval dtime;
	if (head->id == id)
	{
		dtime = head->delta_time;
		head = head->next;
		// if(head) head->delta_time += dtime;
		if(head) timeval_add(&head->delta_time, &head->delta_time, &dtime);
		print_time_list(head);
		return head;
	}
	while(node)
	{
		if (node->id == id)
		{
			node->forward->next = node->next;
			dtime = node->delta_time;
			// if(node->next) node->next->delta_time += dtime;
			if(node->next) timeval_add(&node->next->delta_time, &node->next->delta_time, &dtime);
			print_time_list(head);
			return head;
		}
		node = node->next;
	}
	printf("Error: id error, trying to delete id that is not exist\n");
	print_time_list(head);
	return head;
	// while()
}

TimeNode *update_time_list(TimeNode *head)
{
	struct timeval now, dtime, r;
	gettimeofday(&now, NULL);
	// int dtime = now - lasttime;
	if (lasttime.tv_sec == 0 && lasttime.tv_usec == 0)
	{
		dtime = lasttime;
	}
	else
		timeval_subtract(&dtime, &now, &lasttime);
	printf("update: lasttime is %ld.%d, dtime is %ld.%d\n", lasttime.tv_sec, (int)lasttime.tv_usec, dtime.tv_sec, (int)dtime.tv_usec);
	lasttime = now;
	// TimeNode *node = head;
	// while(head && dtime >= head->delta_time)
	while(head && !timeval_subtract(&r, &dtime, &head->delta_time))
	{
		// printf("head move\n");
		// dtime -= head->delta_time;
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

void syserr(char* msg)
{
	fprintf(stderr, "%s: %s\n", strerror(errno), msg);
	abort();
}