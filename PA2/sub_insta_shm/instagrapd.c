#include<unistd.h>
#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>

//for shared memory
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include "comm.h"
#include<signal.h>

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short int* array;
};

int gnShmID;	// shared memory indicator
int gnSemID;	// Semapore indicator

void* set_shutdown (){

	shmctl( gnShmID, IPC_RMID, 0 );
    semctl( gnSemID, IPC_RMID, 0 );

 	exit (1);
}


void child_proc(int conn){
	char buf[1024];
	char* data = 0x0;
	char* orig = 0x0;
	int len =0;
	int s;

	/* Shared Memory */
    key_t keyShm;       /* Shared Memory Key */
	_ST_SHM *pstShm;      /* 공용 메모리 구조체 */

	/* Semapore */
    key_t keySem;       /* Semapore Key */

    struct sembuf mysem_open  = {0, -1, SEM_UNDO}; // 세마포어 얻기
	struct sembuf mysem_close = {0, 1, SEM_UNDO};  // 세마포어 돌려주기

    /* Shared Memory의 키값을 생성한다. */
	keyShm = ftok( SHM_PATH, SHM_KEY );

    /* 공유 메모리 세그먼트를 연다 - 필요하면 만든다. */
    if( (gnShmID = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){
        if( (gnShmID = shmget( keyShm, SEGSIZE, 0 )) == -1 ){
	        perror("shmget");
            exit(1);
            }
        }

 	  	/* 현재 프로세스에 공유 메모리 세그먼트를 연결한다. */
        if( (pstShm = (_ST_SHM *)shmat(gnShmID, 0, 0)) == NULL ){
 	       perror("shmat");
           exit(1);
        }

        /* 현재 프로세스에 공유 메모리 세그먼트를 연결한다. */
        if( (pstShm = (_ST_SHM *)shmat(gnShmID, 0, 0)) == NULL ){
	        perror("shmat");
            exit(1);
        }

 	    /* Semapore 키값을 생성한다. */
        keySem = ftok( SHM_PATH, SEM_KEY );

	    /* 공유 Semapore 세그먼트를 연다 - 필요하면 만든다. */
	    if( (gnSemID = semget( keySem, 1, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){

            /* Segment probably already exists - try as a client */
            if( (gnSemID = semget( keySem, 0, 0 )) == -1 ){
	            perror("semget");
                exit(1);
            }
        }

	while( (s = recv(conn, buf, 1023, 0)) > 0){
		printf(">> s : %d\n",s);
		buf[s] = 0x0;
		if(data == 0x0){
			data = strdup(buf);
			len = s;
		}
		else{
			data = realloc(data, len + s + 1);
			strncpy(data + len, buf, s);
			data[len + s] = 0x0;
			len += s;
		}

	}
	printf(">server : %s\n", data);
 
	char* msg;
	char* temp;
	int is_Bag = 0;
	temp = strtok(data, "`");
	char* op = malloc(sizeof(char) * strlen(temp) + 1);
	strcpy(op, temp);

	// op check / 1 : first time / 2: after...
	if(atoi(op) == 1){
	//	printf("it is first time\n");

		temp = strtok(NULL, "`");
		char* IP = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(IP, temp);
	
		temp = strtok(NULL, "`");
		char* Port = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(Port, temp);

		temp = strtok(NULL, "`");
	    char* ID_char = malloc(sizeof(char) * strlen(temp) + 1);
	    strcpy(ID_char, temp);
		int ID_int = atoi(ID_char);
		printf("ID int : %d\n", ID_int);

		temp = strtok(NULL, "`");
	    char* PW = malloc(sizeof(char) * strlen(temp) + 1);
	    strcpy(PW, temp);

		temp = strtok(NULL, "`");
	    char* file = malloc(sizeof(char) * strlen(temp) + 1);
	    strcpy(file, temp);

		//for check
//		printf(">> server : IP : %s, Port : %s, ID : %s, PW : %s, file : %s\n", IP, Port, ID_char, PW, file);
	
		//TODO 공유메모리에 ID와 PW 의 쌍을 저장해야 해.
		int inBag = 0;
		for(int i = 0; i < pstShm->number_of_items; i++){
			if(ID_int == pstShm->ID[i]){
				inBag = 1;
				break;
			}	
        }
		if(inBag == 0){
			pstShm->ID[pstShm->number_of_items] = ID_int;
            strcpy(pstShm->pw[pstShm->number_of_items], PW);  
         	pstShm->number_of_items++;
         	pstShm->number_of_items = pstShm->number_of_items % 40;
            printf(">>saving ok\n");
		}else{
			printf("<<saving error\n");
		}

		if(IP != NULL)
			free(IP);
	}
	else if(atoi(op) == 2){
		// TODO: ID CHECK and pw check in shared memory
		temp = strtok(NULL, "`");
		char* ID_2 = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(ID_2, temp);
		int ID_2_int = atoi(ID_2);

		temp = strtok(NULL, "`");
		char* PW_2 = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(PW_2, temp);
		
//		printf("Bag[%d] : ID : %d, PW : %s\n", pstShm->number_of_items, pstShm->ID[pstShm->number_of_items - 1], pstShm->pw[pstShm->number_of_items - 1]);
//		printf("ID : %d\n", pstShm->ID[pstShm->number_of_items - 1]);
//		printf("PW : %s\n", pstShm->pw[pstShm->number_of_items - 1]);
//		printf("#of items : %d\n", pstShm->number_of_items);


		for(int i = 0; i < pstShm->number_of_items; i++){
			printf("[%d] id_2 : %d, pw_2 : %s / pst_id : %d, pst_pw : %s\n",i,ID_2_int,PW_2,pstShm->ID[i],pstShm->pw[i]);
			if(ID_2_int == pstShm->ID[i]){  // 아이디, pw 일치
				is_Bag++;
				if(strcmp(pstShm->pw[i], PW_2) == 0){
					msg = (char*)malloc(sizeof(char) * strlen(data) + 1);
	    	        sprintf(msg, "%s", data);
					break;
				}
				else{	//permission deny
					char Not_same[] = "permission denied";
 		            msg = (char*)malloc(sizeof(char) * strlen(Not_same) + 1);
			        sprintf(msg, "%s", Not_same);
				}
			}
		}
	
		orig = msg;
   		len = strlen(msg);
    	s = 0;
    	while(len > 0 && (s = send(conn, msg, len, 0)) > 0){
    	    msg += s;
    	    len -= s;
    	}
    	shutdown(conn, SHUT_WR);

	}	
}

int main(int argc, char *argv[]){
	int listen_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	
	char buffer[1024] = {0};

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == 0){
		perror("socket failed(no recieve) : ");
		exit(EXIT_FAILURE);
	}

	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // the local host
	address.sin_port = htons(8123);
	if(bind(listen_fd, (struct sockaddr*) &address, sizeof(address)) < 0){
		perror("bind failed : ");
		exit(EXIT_FAILURE);
	}

	// TODO : ID set , passwd checking in shared memory
	// shared memory
	key_t keyShm;	// shared memory key
	_ST_SHM *pstShm;	// 공용메모리 구조체
	
	// Semapore
	key_t keySem;	// semapore key
	union semun sem_union;
	struct sembuf mysem_open = {0, -1, SEM_UNDO}; // 세마포 얻기
	struct sembuf mysem_close = {0, 1, SEM_UNDO}; // 세마포 돌려주기

	keyShm = ftok(SHM_PATH, SHM_KEY);	// shared memory 키값을 생성

	/* 공유 메모리 세그먼트를 연다 - 필요하면 만든다. */
    if( (gnShmID = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){

        /* Segment probably already exists - try as a client */
        if( (gnShmID = shmget( keyShm, SEGSIZE, 0 )) == -1 ){
	        perror("shmget");
            exit(1);
		}
	}

	/* 현재 프로세스에 공유 메모리 세그먼트를 연결한다 */
	if( (pstShm = (_ST_SHM*)shmat(gnShmID, 0, 0)) == NULL){
		perror("shmat");
	}

	/* 세마포 키값을 생성한다*/
	keySem = ftok(SHM_PATH, SEM_KEY);

	/* 공유 세마포 세그먼트를 연다 - 필요하면 만든다*/
	if( (gnSemID = semget(keySem, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1){
		
		/* Segment probably already exists - try as a client*/
		if((gnSemID = semget(keySem, 0, 0)) == -1){
			perror("semget");
			exit(-1);
		}
	}

	/* 세마포 초기화*/
	sem_union.val = 1;
	semctl(gnSemID, 0, SETVAL, sem_union);

	/* Signal 등록 */
    (void)signal(SIGINT, (void (*)())set_shutdown);
 
//    pstShm->member2 = 100;
	pstShm->number_of_items = 0;

	if(listen(listen_fd, 16 /*size of waiting queue*/) < 0){
        perror("listen failed : ");
        exit(EXIT_FAILURE);
    }

	// listen 
	while(1){
//		if(listen(listen_fd, 16 /*size of waiting queue*/) < 0){
//			perror("listen failed : ");
//			exit(EXIT_FAILURE);
//		}
	
		new_socket = accept(listen_fd, (struct sockaddr*) &address, (socklen_t*)&addrlen);
		if(new_socket < 0){
			perror("accept failed : ");	//accept?
		    exit(EXIT_FAILURE);
		}
		
		if(fork() > 0){
			child_proc(new_socket);
		}
		else{ // 다른 클라이언트의 접속 요청확인하러 떠납니다
			close(new_socket);
		}
	}
	
	// 세마포 사용을 해제한다
	semop(gnSemID, &mysem_close, 1);
}
