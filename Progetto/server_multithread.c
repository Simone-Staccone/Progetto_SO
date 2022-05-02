#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void thread_fun(void *arg)
{
	char *st = (char *)arg;
	puts("Entered the trhead");
	execlp("./server",st);
}

int main(int argc,char **argv)
{
	if(argc != 2)
	{
		puts("Error, too few or too much argument, ./server <port_number> -pthread");
		exit(EXIT_FAILURE);
	}
	
	//int port = strtol(argv[1],0,10);
	pthread_t thr;
	
	while(1)
	{
		if(pthread_create(&thr,NULL,(void *)thread_fun,(void *)argv[1]) == -1)
		{
			puts("Error creating thread");
			exit(EXIT_FAILURE);
		}
	}
	
	
	
	return 0;
}