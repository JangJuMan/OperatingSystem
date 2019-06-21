#include<unistd.h>
#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>

struct ID_SET{
	int id;
	char passwd[10];
};

int number_of_item = 0;

void child_proc(int conn, struct ID_SET* id_set){
	char buf[1024];
	char* data = 0x0;
	char* orig = 0x0;
	int len =0;
	int s;

	while( (s = recv(conn, buf, 1023, 0)) > 0){
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
 
	char* temp;
	temp = strtok(buf, "`");
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
	printf("IP : %s, Port : %s, ID : %s, PW : %s, file : %s\n", IP, Port, ID_char, PW, file);

	// ID check--------------------------------not completed... ----------------------
	int access_ok = 0;
	int empty_id = 0;
	printf("\t<in> -- number of item:%d\n", number_of_item);
	
	for(int i=0; i < number_of_item; i++){
		printf("\t<Bag> [%d] id : %d, pw : %s\n",i, (id_set+i)->id, (id_set+i)->passwd);
		// 만약 해당 아이디가 있다면
		if((id_set+i)->id == ID_int){
			empty_id++;
			// 패스워드 체크
			if(strcmp((id_set+i)->passwd, PW) == 0){
				access_ok = 1;
			}
			else{
				access_ok = -1;
				printf(">> wrong password.!\n");
				// TODO : send error message to submitter -> set orig as error msg
			}
		}		
	}
	printf(" empty id : %d\n", empty_id);
	// 해당 아이디가 없다면
	if(empty_id == 0){
		// 새로할당하고 저장하기
		id_set = (struct ID_SET*)malloc(sizeof(struct ID_SET));
		(id_set + number_of_item)->id = ID_int;
		strcpy((id_set + number_of_item)->passwd , PW);
		printf("\t\t<new>id : %d , pw : %s\n", (id_set + number_of_item)->id, (id_set + number_of_item)->passwd);
		number_of_item++;
	}

	printf("\t<after> -- numberofitem:%d\n",number_of_item);

	// not yet.. ----------------------------------------------------------------------

	orig = data;
	while(len > 0 && (s = send(conn, data, len, 0)) > 0){
		data += s;
		len -= s;
	}
	shutdown(conn, SHUT_WR);
	if(orig != 0x0)
		free(orig);
	if(IP != 0x0)
		free(IP);
	access_ok = 0;
	empty_id = 0;
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

	// TODO : ID set , passwd checking
	struct ID_SET *id_set = malloc(sizeof(struct ID_SET));
	number_of_item = 0;

	while(1){
		if(listen(listen_fd, 16 /*size of waiting queue*/) < 0){
			perror("listen failed : ");
			exit(EXIT_FAILURE);
		}
	
		new_socket = accept(listen_fd, (struct sockaddr*) &address, (socklen_t*)&addrlen);
		if(new_socket < 0){
			perror("accept failed : ");	//accept?
		    exit(EXIT_FAILURE);
		}
		
		if(fork() > 0){
			child_proc(new_socket, id_set);
		}
		else{ // 다른 클라이언트의 접속 요청확인하러 떠납니다
			close(new_socket);
		}
	}

	free(id_set);
}
