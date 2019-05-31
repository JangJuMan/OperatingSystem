#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<dlfcn.h>
#include<execinfo.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>
static pthread_mutex_t MUTE = PTHREAD_MUTEX_INITIALIZER;

static unsigned long int tid_log[10] = {0x0, };
static pthread_mutex_t* mutex_log[10][100] = {0, };
static pthread_mutex_t* adj_matrix[100][100] ={0, };
static int number_of_node = 0;
static int edge_tail[100] = {0, };
static int new_check = 0;
static int adj_col=0;
static int thread_count=0;	
static int curr[10] = {0, };
int visited[100] = {0, };
int d[100] = {0,};
int f[100] = {0, };
//static int finished[100][100] = {0, };
//int findCycleAlgorithm(int );

//for debug--------------------------------
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
		for(int j=0; j<10; j++){
			fprintf(stderr, "[%p] ", mutex_log[i][j]);
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n\n");
}

void print_adj_mat(){
	fprintf(stderr, "[adj_matrix]---------\n");
		for(int i=0; i<10; i++){
			for(int j=0; j<10; j++){
				fprintf(stderr, "[%p] ", adj_matrix[i][j]);
			}
			fprintf(stderr, "\n");
		}
	fprintf(stderr, "\n");
}

void print_edge_tail(){
	fprintf(stderr, "[edge_tail]----------\n");
	for(int i=0; i<10; i++){
		fprintf(stderr, "[%d] ", edge_tail[i]);
	}
	fprintf(stderr, "\n");
}


void print_all(){
	print_curr();
	print_tid_log();
	print_mutex_log();
	print_adj_mat();
	print_edge_tail();
}

//----------------------------------------------

//해당 쓰레드 아이디의 인덱스를 리턴/ 없다면 -1 리턴 
int check_tid(unsigned long int tid){
	for(int i=0; i<10; i++){
		if(tid_log[i] == tid)
			return i;
	}
	return -1;
}

// tid_log에서 가장 가까운 빈 공간 리턴
int get_empty_tid_log(){
	for(int i=0; i<10; i++){
		if(tid_log[i] == 0){
			return i;
		}
	}
	return -1;
}

// adj_matrix에서 첫째 줄에 노드 추가(lock)
void node_check(pthread_mutex_t *mutex){
	new_check = 0;
	for(int i=0; i<100; i++){
		if(adj_matrix[i][0] == mutex){
			new_check++;
			break;
		}
	}
	if(new_check == 0){
		for(int i=0; i<100; i++){
			if(adj_matrix[i][0] == NULL){
				adj_matrix[i][0] = mutex;
				break;
			}
		}
	}
}

// adj_matrix에 엣지 부분 추가하는 것 (lock)
void edge_add(pthread_mutex_t *before, pthread_mutex_t *mutex){
	for(int i=0; i<100; i++){
		if(adj_matrix[i][0] == before){
			adj_matrix[i][++edge_tail[i]] = mutex;
			break;
		}
	}
}

// adj_matrix에서 첫째 줄에 노드 제거 (unlock)
void remove_node(pthread_mutex_t *mutex){
	for(int i=0; i<100; i++){
//		printf("adj_matrix[%d][0] == %p == mutex(%p)\n",i, adj_matrix[i][0], mutex);
		if(adj_matrix[i][0] == mutex){
			adj_matrix[i][0] = 0;
			break;
		}
	}
}


// adj_matrix에서 엣지 제거 (unlock)
void remove_edge(pthread_mutex_t *before, pthread_mutex_t *mutex){
	for(int i=0; i<100; i++){
		if(adj_matrix[i][0] == before){
//			fprintf(stderr, "<< is same?\n");
			if(adj_matrix[i][edge_tail[i]] == mutex){
//				fprintf(stderr, ">> same\n");
				adj_matrix[i][edge_tail[i]--] = 0;
			}
			break;
		}
	}
	remove_node(mutex);
}

// tid_log 리셋 (unlock)
void reset_tid_log(){
	for(int i=0; i<10; i++){
		if(mutex_log[i][0] == NULL){
			tid_log[i] = 0;
		}
	}
}
// cycle check;
int Time=0;
int answer=-2;
int present_idx[100] ={0, };

int isCyclicUnit(int v){
//	if(visited[v] == 0){
	if(adj_matrix[v][0] == NULL){
		return -1;
	}
	if(f[v] != 0){
		return -1;
	}
	answer = -1;

	if(visited[v] == 0){
		visited[v] = 1;
		Time++;
		d[v] = Time;
		fprintf(stderr, "d[%d] : %d\n",v, Time);
		for(int i=1; i<=edge_tail[v]; i++){
			for(int col = 0; col<100; col++){
				if(adj_matrix[v][i] == adj_matrix[col][0]){
					if(f[col] == 0 && d[col] < d[v]){
						fprintf(stderr,"\n\n>>>>this is cycle adj[%d][%d] vs adj[%d][%d]! : %d < %d | f[%d] : %d?\n\n", v, i, col, 0,d[col], d[v], col, f[col]);
						return 1;	//isCyclicUnit(col);
					}
					else{
						answer = isCyclicUnit(col);
						break;
					}
				}
			}
		}
	}
	Time++;
	f[v] = Time;
	printf("f[%d] : %d\n", v, f[v]);
	
	//}
	return answer;
}


int isCycle(){
	for(int i=0; i<100; i++){
		visited[i] = 0;
		f[i] = 0;
		d[i] = 10001;
	}
	Time = 0;

	for(int i=0; i<100; i++){
		if(isCyclicUnit(i) == 1)
				return 1;
	}
	return -1;
}



// lock을 overriding
int pthread_mutex_lock(pthread_mutex_t *mutex){

	memset(visited, 0, sizeof(visited));
	int (*pthread_mutex_lock_p)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	char* error;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0)
		exit(1);

	//LOCKㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜ
	pthread_mutex_lock_p(&MUTE);
	pthread_t tid = pthread_self();
	fprintf(stderr, "Starting of pthread_mutex_lock(%p)==================================\n", mutex);

		// 처음오는 스레드이면 새롭게 추가하고/ 아니면 기존 거에 추가한다.
		int now = check_tid(tid);
		if(now == -1){
			now = get_empty_tid_log();
			tid_log[now] = tid;

		}
		mutex_log[now][curr[now]++] = mutex;

		// add at first colum
		node_check(mutex);
		for(int i=0; i< curr[now]; i++){
			if(mutex_log[now][i] != mutex)
				edge_add(mutex_log[now][i], mutex);
		}

		print_all();

//		fprintf(stderr, "\t\tresult = %d\n\n", isCycle());

/*		for(int i = 0; i < 100; i++)
			if(visited[i] != 1)
				if(findCycleAlgorithm(i))
					fprintf(stderr, "\n\n\n\ndeadlock\n\n\n\n");
*/
//		printf("if cycle? : %d\n",findCycleAlgorithm(0));
//		if(findCycleAlgorithm(0))
//			fprintf(stderr, "\n\n\n\t>>>> [DEAD LOCK] <<<<\n\n\n\n");
		// 아닙니다		if(isCycle(i));

	//UNLOCKㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗ
	pthread_mutex_unlock_p(&MUTE);
	fprintf(stderr, "Ending of pthread_mutex_lock(%p)===================================\n\n\n", mutex);

	if(isCycle() == 1){
		printf("\n\n\n\n\n\n\n\n\n\n\t>>> [ DEAD LOCK ] <<<\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

	return pthread_mutex_lock_p(mutex);
}

int k=0;

// unlock 을 overriding
int pthread_mutex_unlock(pthread_mutex_t *mutex){
	memset(visited, 0, sizeof(visited));
	int erase_check = 0;
	int (*pthread_mutex_lock_p)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_p)(pthread_mutex_t *mutex);
	char* error;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0)
		exit(1);

	//LOCKㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜㅜ
	pthread_mutex_lock_p(&MUTE);
	pthread_t tid = pthread_self();
	fprintf(stderr, "Starting of pthread_mutex_unlock(%p)+++++++++++++++++++++++++++++++++++\n", mutex);

		// 일치하는 스레드에서 해당 뮤택스를 지운다
		int now = check_tid(tid);
		fprintf(stderr,"now : %d\n", now);
		if(now == -1){
		}
		else{	
			mutex_log[now][--curr[now]] = 0;

			for(int i=0; i<10; i++){
				for(int j=1; j<100; j++){
				/*	for(k=0; k<100; k++){
						if(mutex == adj_matrix[k][0]){
							break;
						}
					}
*/
					if(i == now){	
					}
					else{
						if(mutex_log[i][j] == mutex){	//adj_matrix[i][j] == mutex){
							erase_check++;
						}
					}
				}
			}
			fprintf(stderr, "erase ch : %d, curr[now] : %d, k : %d\n",erase_check, curr[now], k);
			if(erase_check == 0){
				if(curr[now] == 0){
					remove_node(mutex);
				}
				else{
					fprintf(stderr, "\t\t >>>i : %d | i < %d -- %d ++ %d\n",0, curr[now]-1,curr[now], curr[now]+1);
					for(int i=0; i<curr[now]; i++){
						remove_edge(mutex_log[now][i],mutex);
					}
				}
			}
		

		}
		reset_tid_log();
		print_all();
//		fprintf(stderr, "\t\tunlock result = %d\n\n", isCycle());
/*		for(int i = 0; i < 100; i++)
			if(visited[i] != 1)
				if(findCycleAlgorithm(i))
						fprintf(stderr,"\n\n\n\n\ndeadlock\n\n\n\n");
*/		//////////////// new one/////////
		//if(isCycling){
		//	printf("[unlock] : is cycle!\n");
		//}

//		if(findCycleAlgorithm(0)) 
//			fprintf(stderr, "\n\n\n\n\n\ndeadlock\n\n\n\n\n\n");


		print_all();

	//UNLOCKㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗㅗ
	pthread_mutex_unlock_p(&MUTE);
	fprintf(stderr, "Ending of pthread_mutex_unlock(%p)+++++++++++++++++++++++++++++++++++++\n\n\n", mutex);

	if(isCycle() == 1){
		printf("\n\n\n\n\n\n\n\n\n\n\t>>> [ free DEAD LOCK ] <<<\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

	return pthread_mutex_unlock_p(mutex);
}
/*
int findCycleAlgorithm(int here){
	if(visited[here] != 0){
		if(visited[here] == -1)
			return 1;
		else
			return 0;
	}
	visited[here] = -1;
	for(int there = 1; there < 100; there++){
		if(adj_matrix[here][there] != NULL && findCycleAlgorithm(there))
			return 1;
	}

	visited[here] = 1;
	return 0;
}
*/
