#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>
#include<execinfo.h>
#include<pthread.h>
#include<unistd.h>

pthread_mutex_t MUTE = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t MUTE2 = PTHREAD_MUTEX_INITIALIZER;

static unsigned long int tid_log[10] = {0x0};
static char* mutex_log[10][100] = {0x0};
static char* adj_matrix[100][100] = {0};
static int number_of_node = 0;
static int edge_tail[100] = {0};
static int new_check = 0;
//static int adj_matrix[100][100] = {0};
//static int adj_low=0;
//static int adj_mid[100] = {0};
static int adj_col=0;
static int thread_count=0;	// mutex_count=0;
static int curr[10] = {0};

void print_curr(){
	fprintf(stderr, "[curr]---------\n");
	for(int i=0; i<10; i++){
		fprintf(stderr, "[%d] ", curr[i]);
	}
	fprintf(stderr, "\n\n");
}

void print_tid_log(){
	fprintf(stderr, "[tid_log]----------\n");
	for(int i=0; i<10; i++){
		fprintf(stderr, "[%lu] ", tid_log[i]);
	}
	fprintf(stderr, "\n\n");
}

void print_mutex_log(){
	fprintf(stderr, "[mutex_log]---------\n");
	for(int i=0; i<5; i++){
		for(int j=0; j<5; j++){
			fprintf(stderr, "[%p] ", mutex_log[i][j]);
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n\n");
}

void print_adj_mat(){
	fprintf(stderr, "[adj_matrix]---------\n");
		for(int i=0; i<5; i++){
			for(int j=0; j<5; j++){
				fprintf(stderr, "[%p] ", adj_matrix[i][j]);
			}
			fprintf(stderr, "\n");
		}
	fprintf(stderr, "\n\n");
}

void print_all(){
	print_curr();
	print_tid_log();
	print_mutex_log();
	print_adj_mat();
}


int check_tid(unsigned long int tid){
	for(int i=0; i<10; i++){
		if(tid_log[i] == tid)
			return i;
	}
	return -1;
}

void node_check(pthread_mutex_t *mutex){
	new_check = 0;
	for(int i=0; i<100; i++){
		if(adj_matrix[i][0] == mutex){
			new_check++;
			break;
		}
	}
	if(new_check == 0){
	//	fprintf(stderr,"add node part\n");
		for(int i=0; i<100; i++){
			if(adj_matrix[i][0] == NULL){
	//			fprintf(stderr, "it is nil : [%d][0] : %p \n", i, adj_matrix[i][0]);
				adj_matrix[i][0] = mutex;
				break;
			}
		}
	}
}

void edge_add(pthread_mutex_t *before, pthread_mutex_t *mutex){
	for(int i=0; i<100; i++){
		if(adj_matrix[i][0] == before){
			adj_matrix[i][++edge_tail[i]] = mutex;
			break;
		}
	}
}


int pthread_mutex_lock(pthread_mutex_t *mutex){
	
	int (*pthread_mutex_lock_p)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	char* error;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0)
		exit(1);

	//LOCK+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	pthread_mutex_lock_p(&MUTE);
	pthread_t tid = pthread_self();
	//fprintf(stderr, "Starting of pthread_mutex_lock()==================================\n");

		// 처음오는 스레드이면 새롭게 추가하고/ 아니면 기존 거에 추가한다.
		int now = check_tid(tid);
		if(now == -1){
			tid_log[thread_count] = tid;
			now = thread_count++;
		}
		mutex_log[now][curr[now]++] = mutex;

		int ptr = pthread_mutex_lock_p(mutex);

		// add at first colum
		node_check(mutex);
		for(int i=0; i< curr[now]; i++){
			if(mutex_log[now][i] != mutex)
				edge_add(mutex_log[now][i], mutex);
		}


	//	print_all();
	//UNLOCK+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	pthread_mutex_unlock_p(&MUTE);
	//fprintf(stderr, "Ending of pthread_mutex_lock()===================================\n");

	return ptr;
}


int pthread_mutex_unlock(pthread_mutex_t *mutex){

	int (*pthread_mutex_lock_p)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	char* error;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0)
		exit(1);

	//LOCK+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	pthread_mutex_lock_p(&MUTE);
	pthread_t tid = pthread_self();
	//fprintf(stderr, "Starting of pthread_mutex_unlock()+++++++++++++++++++++++++++++++++++\n");

		// 일치하는 스레드에서 해당 뮤택스를 지운다
		int now = check_tid(tid);
		mutex_log[now][--curr[now]] = 0;

		int ptr = pthread_mutex_unlock_p(mutex);

	//	print_all();

	//UNLOCK+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	pthread_mutex_unlock_p(&MUTE);
	//fprintf(stderr, "Ending of pthread_mutex_unlock()+++++++++++++++++++++++++++++++++++++\n");

	return ptr;
}

