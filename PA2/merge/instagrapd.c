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

ssize_t total = 0;

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

struct sockaddr_in work_addr;
int sock_fd_work;
int WPort;
char* IP_W;

void child_proc(int conn, /*int sock_fd_work,*/ char* Dir){
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
	keyShm = ftok( SHM_PATH, SHM_KEY );
    if( (gnShmID = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){
        if( (gnShmID = shmget( keyShm, SEGSIZE, 0 )) == -1 ){
	        perror("shmget");
            exit(1);
            }
        }
        if( (pstShm = (_ST_SHM *)shmat(gnShmID, 0, 0)) == NULL ){
 	       perror("shmat");
           exit(1);
        }
        if( (pstShm = (_ST_SHM *)shmat(gnShmID, 0, 0)) == NULL ){
	        perror("shmat");
            exit(1);
        }
        keySem = ftok( SHM_PATH, SEM_KEY );
	    if( (gnSemID = semget( keySem, 1, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){
            if( (gnSemID = semget( keySem, 0, 0 )) == -1 ){
	            perror("semget");
                exit(1);
            }
        }

		
	while( (s = recv(conn, buf, 1023, 0)) > 0){
		buf[s] = 0x0;
	//	printf("buf[%d] : %s\n", s, buf);
		if(data == 0x0){
			data = strdup(buf);
			len = s;
	//		printf("1. data : \n%s\n---\n", data);
		}
		else{
			data = realloc(data, len + s + 1);
			strncpy(data + len, buf, s);
			data[len + s] = 0x0;
			len += s;
	//		printf("2. data : \n%s\n----\n",data);
		}

	}
 
	//int z;
//	printf(">> instagrap : \n%s\n>>>\n", data);
//	scanf("%d", &z);

	char* msg;
	char* temp;
	int is_Bag = 0;
	temp = strtok(data, "`");
	char* op = malloc(sizeof(char) * strlen(temp) + 1);
	strcpy(op, temp);

	if(atoi(op) == 1){

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
//		printf("ID int : %d\n", ID_int);

		temp = strtok(NULL, "`");
	    char* PW = malloc(sizeof(char) * strlen(temp) + 1);
	    strcpy(PW, temp);

		temp = strtok(NULL, "`");
	    char* file = malloc(sizeof(char) * (strlen(temp) + 1));
	    strcpy(file, temp);

	
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
			strcpy(pstShm->file[pstShm->number_of_items], file);
         	pstShm->number_of_items++;
         	pstShm->number_of_items = pstShm->number_of_items % 20;
            printf(">>saving ok\n");
		}else{
			printf("<<saving error\n");
		}
		
//		printf("\n[file]>>\n%s\n\n", pstShm->file[pstShm->number_of_items-1]);
//		scanf("%d",&z);
		
//		if(IP != NULL)
//			free(IP);
	}
	else if(atoi(op) == 2){
		temp = strtok(NULL, "`");
		char* ID_2 = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(ID_2, temp);
		int ID_2_int = atoi(ID_2);

		temp = strtok(NULL, "`");
		char* PW_2 = malloc(sizeof(char) * strlen(temp) + 1);
		strcpy(PW_2, temp);
		
		for(int i = 0; i < pstShm->number_of_items; i++){
//			printf("[%d] id_2 : %d, pw_2 : %s / pst_id : %d, pst_pw : %s\n",i,ID_2_int,PW_2,pstShm->ID[i],pstShm->pw[i]);
			if(ID_2_int == pstShm->ID[i]){  // 아이디, pw 일치
				is_Bag++;
				if(strcmp(pstShm->pw[i], PW_2) == 0){
					for(int j=1; j<=10; j++){
						char* input;
						char* data;
						int size;
						int count;

					//	for(j=1; j<=10; j++){	//---------------

						char file_path[128];
						sprintf(file_path, "%s/%d.in", Dir, j);
						printf("[%d]. ",j);
						printf("opening the file >> %s\n", file_path);
						FILE* fp = fopen(file_path, "r");
						if(fp == NULL){								printf("file open error");
							return;
						}
						fseek(fp, 0, SEEK_END);
						size = ftell(fp);
						input = malloc(size + 1);
						memset(input, 0, size + 1);
						fseek(fp, 0, SEEK_SET);
						count = fread(input, size, 1, fp);
						fclose(fp);
	
						msg = (char*)malloc(sizeof(char) * (strlen(input) + strlen(pstShm->file[i]) + 5));
						sprintf(msg, "%s`%s", pstShm->file[i], input);
//							printf("to worker : %s\n", msg);
							// TODO: 워커에 메시지 보내고 받기
	
							// 소켓만들고
						sock_fd_work = socket(AF_INET, SOCK_STREAM, 0);
						if(sock_fd_work <= 0){
							perror("socket failed : ");
							exit(EXIT_FAILURE);
						}
						memset(&work_addr, '0', sizeof(work_addr));
						work_addr.sin_family = AF_INET;
						work_addr.sin_port = htons(WPort);
						if(inet_pton(AF_INET, IP_W, &work_addr.sin_addr) <= 0){
							perror("inet_pton failed : ");
							exit(EXIT_FAILURE);
						}
	
						if(connect(sock_fd_work, (struct sockaddr*) &work_addr, sizeof(work_addr)) < 0){
							perror("connect failed : ");
							exit(EXIT_FAILURE);
						}
					
							// 메시지 보내
						data = msg;
						len = strlen(msg);
						s = 0;
						while(len > 0 && (s = send(sock_fd_work, data, len, 0)) > 0){
							data += s;
							len -= s;
						}
	
							// 닫아 
						shutdown(sock_fd_work, SHUT_WR);
	
							// 대답을 받아 
						data = 0x0;
						len = 0;
						while( (s=recv(sock_fd_work, buf, 1023, 0)) > 0){
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
						printf(" >> from worker : %s\n", data);
						

						msg = (char*)malloc(sizeof(char) * strlen(data) + 1);
					    sprintf(msg, "%s", data);
						
						orig = msg;
						len = strlen(msg);
						s = 0;
						while(len > 0 && (s = send(conn, msg, len, 0)) > 0){
							msg += s;
							len -= s;
						}
						shutdown(conn, SHUT_WR);
					
					// to debug
//					sleep(1);
					}
					//free(input);
					break;
				}
			}
			else{	//permission deny
				char Not_same[] = "permission denied";
			    msg = (char*)malloc(sizeof(char) * strlen(Not_same) + 1);
		        sprintf(msg, "%s", Not_same);
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
		//TODO : msg free
		//free(msg);
	//	free(data);

}	

int main(int argc, char *argv[]){
	int listen_fd, new_socket;
	struct sockaddr_in address;
//	int opt = 1;
	int addrlen = sizeof(address);

	//argument passing
	//char* IP;
	char* WPort_str;
	int Port;
	//int WPort;
	char* Dir;
	char* BUF;
	int op;
	int argument_on = 0;

	while((op = getopt(argc, argv, "p:w:h")) != -1){
		switch(op){
			case 'w':
				BUF = malloc(sizeof(char) * 50);
				strcpy(BUF, optarg);
				IP_W = strtok(BUF, ":");
				WPort_str = strtok(NULL, ":");
				WPort = atoi(WPort_str);
				argument_on++;
				break;

			case 'p':
				Port = atoi(optarg);
				argument_on++;
				break;

			case 'h':
				printf("help message :\n>> please input the type like this\n./instagrapd -p <Port> -w <IP>:<WPort> <Dir>\n");
				argument_on = -1;
				break;
			}
	}

	if(argument_on == -1){
		return 0;
	}
	else if(argument_on < 2){
		printf("use -h option to see help message\n");
		return 0;
	}
	else{
		Dir = argv[5];
	}


		
	// socket connection with submitter-instagrapd to read
//	 char buffer[1024] = {0};

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == 0){
		perror("socket failed(no recieve) : ");
		exit(EXIT_FAILURE);
	}

	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // the local host
	address.sin_port = htons(Port);		//8124
	if(bind(listen_fd, (struct sockaddr*) &address, sizeof(address)) < 0){
		perror("bind failed : ");
		exit(EXIT_FAILURE);
	}

	// shared memory
	key_t keyShm;	// shared memory key
	_ST_SHM *pstShm;	// 공용메모리 구조체
	// Semapore
	key_t keySem;	// semapore key
	union semun sem_union;
	struct sembuf mysem_open = {0, -1, SEM_UNDO}; // 세마포 얻기
	struct sembuf mysem_close = {0, 1, SEM_UNDO}; // 세마포 돌려주기
	keyShm = ftok(SHM_PATH, SHM_KEY);	// shared memory 키값을 생성
    if( (gnShmID = shmget( keyShm, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666 )) == -1 ){
        if( (gnShmID = shmget( keyShm, SEGSIZE, 0 )) == -1 ){
	        perror("shmget");
            exit(1);
		}
	}
	if( (pstShm = (_ST_SHM*)shmat(gnShmID, 0, 0)) == NULL){
		perror("shmat");
	}
	keySem = ftok(SHM_PATH, SEM_KEY);
	if( (gnSemID = semget(keySem, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1){
		if((gnSemID = semget(keySem, 0, 0)) == -1){
			perror("semget");
			exit(-1);
		}
	}
	sem_union.val = 1;
	semctl(gnSemID, 0, SETVAL, sem_union);
    (void)signal(SIGINT, (void (*)())set_shutdown);
	pstShm->number_of_items = 0;


	// listen 
	while(1){
		if(listen(listen_fd, 16 /*size of waiting queue*/) < 0){
			perror("listen failed : ");
		    exit(EXIT_FAILURE);
		}


//		else{
//			printf("instagrapd connected..!");
//		} 

		new_socket = accept(listen_fd, (struct sockaddr*) &address, (socklen_t*)&addrlen);
		if(new_socket < 0){
			perror("accept failed : ");	//accept?
		    exit(EXIT_FAILURE);
		}
		
		if(fork() > 0){
			child_proc(new_socket,/* sock_fd, */Dir);
		}
		else{ // 다른 클라이언트의 접속 요청확인하러 떠납니다
			close(new_socket);
		}
	}
	
	semop(gnSemID, &mysem_close, 1);
	free(BUF);
}
