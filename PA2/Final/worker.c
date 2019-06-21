#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<signal.h>

int WPort;

void child_proc(int conn){
	char buf[1024];
	char* data = 0x0;
	int len = 0;
	int s;

	while( (s = recv(conn, buf, 1023, 0)) > 0){
		buf[s] = 0x0;
//		printf("buf[%d] : %s\n", s, buf);
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

//	printf(" >> worker : %s\n ", data);


	// strtok로 인자 구별하여 저장하기
	char* tmp;
	tmp = strtok(data, "`");
	char* source = malloc(sizeof(char) * (strlen(tmp) + 1));
	strcpy(source, tmp);

	tmp = strtok(NULL, "`");
	char* IN = malloc(sizeof(char) * (strlen(tmp) + 1));
	strcpy(IN, tmp);

	printf(">> source : \n%s\n", source);
	// 빌드하고 계산하기

	// 데이터 결과값 돌려주기? 여기서? 
	// 소켓 만들고 데이터	보내
	

	
	char* orig = 0x0;

	char temp[] = "return to insta";
	orig = temp;
	len = strlen(temp);
	s = 0;
	printf(">> orig : %s\n",orig);
	while(len > 0 && (s = send(conn, orig, len, 0)) > 0){
		orig += s;
		len -= s;
	}

	shutdown(conn, SHUT_WR);
//	if(orig != 0x0){
//		free(orig);
//	}

}


int main(int argc, char* argv[]){
	int listen_fd, insta_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	//argument passing
	//int WPort;
	int op;
	int argument_on = 0;

	while((op = getopt(argc, argv, "p:h")) != -1){
		switch(op){
			case 'p':
				WPort = atoi(optarg);
				argument_on++;
				break;

			case 'h':
				printf("help message : \n>> please input the type like this\n./worker -p <Port>\n");
				argument_on = -1;
				break;
		}
	}

	if(argument_on == -1){
		return 0;
	}
	else if(argument_on < 1){
		printf("use -h option to see help message\n");
		return 0;
	}
	
	// socket connection with instagrapd-worker to read
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == 0){
		perror("socket failed(no recieve) : ");
		exit(EXIT_FAILURE);
	}

	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;	// the local host
	address.sin_port = htons(WPort);
	if(bind(listen_fd, (struct sockaddr*) &address, sizeof(address)) < 0){
		perror("bindfailed : ");
		printf(">[%d]\n", WPort);
		exit(EXIT_FAILURE);
	}

	while(1){
		if(listen(listen_fd, 16) < 0 ){
			perror("listen failed : ");
			exit(EXIT_FAILURE);
		}
//		else{
//		    printf("worker connected..!\n");
//	    }
		insta_socket = accept(listen_fd, (struct sockaddr*) &address, (socklen_t*)&addrlen);
		if(insta_socket < 0){
			perror("accept failed : ");
			exit(EXIT_FAILURE);
		}
		
		if(fork() > 0){
			child_proc(insta_socket);
		}
		else{
			close(insta_socket);
		}
	}

		// TODO: 계산하기(파이프 연결하기 + 포크뜨기 + 포크 뜬거에서 dup2해서 다시 빼오기 + 
		//.		 소켓으로 다시 보내주기.
		// if(fork() > 0){


}

