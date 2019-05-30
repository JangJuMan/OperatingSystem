#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void 
noise()
{
	usleep(rand() % 1000) ;
}

void* thread2(void *arg){
	pthread_mutex_lock(&mutex2); noise();
	pthread_mutex_lock(&mutex); noise();
	pthread_mutex_unlock(&mutex); noise();
	pthread_mutex_unlock(&mutex2); noise();
}

void *
thread(void *arg) 
{
		pthread_mutex_lock(&mutex);	 noise() ;
		pthread_mutex_lock(&mutex2); noise() ;
		pthread_mutex_unlock(&mutex2); noise() ;
		pthread_mutex_unlock(&mutex); noise() ;

		pthread_t tid2;
		pthread_create(&tid2, NULL, thread2, NULL);
		pthread_join(tid2, NULL);

		return NULL;
}

int 
main(int argc, char *argv[]) 
{
	pthread_t tid;

	pthread_create(&tid, NULL, thread, NULL);

	pthread_join(tid, NULL);
	return 0;
}

