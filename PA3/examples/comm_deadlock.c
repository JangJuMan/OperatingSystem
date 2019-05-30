#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void noise(){
	usleep(rand() % 1000);
}

void noise2(){
	usleep(rand() % 100);
}


void* thread(void *arg){
	for(int i=0; i<10; i++){
		pthread_mutex_unlock(&mutex);
		printf("%d번째 unlock 통과\n", i);
		noise2();
	}
}

int main(int argc, char* argv[]){
	pthread_t tid;
	
	pthread_create(&tid, NULL, thread, NULL);

	for(int i=0; i<10; i++){
		pthread_mutex_lock(&mutex);
		printf("%d번째 lock 통과\n", i);
		noise();
	}
}

