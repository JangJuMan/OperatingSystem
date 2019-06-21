#include<unistd.h>
#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>

int main(int argc, char *argv[]){

	// argument passing
	char* IP;
	char* Port;
	int ID;
	//char* PW;
	char PW[10];
	char* file;
	char* buf;
	int option;	
	int check_all = 0;

//	for(int i=0; i<argc; i++){
//		printf("argv[%d] : %s\n",i,argv[i]);
//	}

	while( (option = getopt(argc, argv, "n:u:k:p")) != -1){
		switch(option){
			case 'n':
				buf = malloc(sizeof(char) * 50);
			    strcpy(buf, optarg);
   			 	IP = strtok(buf, ":");
    			Port = strtok(NULL, ":");     
				printf("IP : %s, Port : %s\n",IP, Port);
				check_all ++;
				break;
	
			case 'u':
				ID = atoi(optarg);
				printf("ID : %d\n",ID);
				check_all ++;
				break;

			case 'k':
				strcpy(PW, optarg);
				printf("PW : %s\n", PW);
				check_all ++;
				break;
			
			case 'p':
				printf("prototype : ./submitter -n <IP>:<Port> -u <ID> -k <PW> <file>\n");
				check_all = -1;
				break;
		}
	}

	if(check_all == -1){
		return 0;
	}
	else if(check_all <= 2){
		printf("use -p option to see prototype\n");
		return 0;
	}
	else{
		file = argv[7];
		printf("IP : %s, Port : %s, ID : %d, PW : %s, file : %s\n", IP, Port, ID, PW, file);
	}

	// socket to instagrapd
	struct sockaddr_in serv_addr;
	int sock_fd;
	int s, len;
	char buffer[1024] = {0};
	char* data;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd <= 0){
		perror("socket failed : ");
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8123);
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		perror("inet_pton failed : ");
		exit(EXIT_FAILURE);
	}

	if(connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) <0){
		perror("connect failed : ");
		exit(EXIT_FAILURE);
	}

	// data sending
	//char format[1024];
	char* format;
	format = (char*)malloc(sizeof(char) * strlen(file) + 64);
	sprintf(format, "1`%s`%s`%d`%s`%s", IP, Port, ID, PW, file);
//	printf("format : %s\n", format);

	data = format;	
	len = strlen(format);

	s = 0;
	while(len > 0 && (s = send(sock_fd, data, len, 0)) > 0){
        data += s;
        len -= s;
    }

	shutdown(sock_fd, SHUT_WR);


	// TODO: id 랑 pw를 주면서 다시 연결해 + 연결끊고 대답을 받아
	// 만약 아직이라는 대답이 온다면 계속 반복해

	//다 됐다는 대답이 오면 그거를 프린트해
	
	do{
		// 보내고
		char* request;
		request = (char*)malloc(sizeof(char) * strlen(PW) + 5);
		sprintf(request, "2`%d`%s", ID, PW);
	
		data = request;
		len = strlen(request);
		s = 0;
		while(len > 0 && (s = send(sock_fd, data, len, 0)) > 0){
			data += s;
			len -= s;
		}
		shutdown(sock_fd, SHUT_WR);
	
		// 대답을 받아
		char buffer2[1024];
		data = 0x0;
		len = 0;
		while( (s= recv(sock_fd, buffer2, 1023, 0)) > 0){
			buffer2[s] = 0x0;
			if(data == 0x0){
				data = strdup(buffer2);
				len = s;
			}
			else{
				data = realloc(data, len + s + 1);
				strncpy(data + len, buffer2, s);
				data[len + s] = 0x0;
				len += s;
			}
		}
		printf("> submitter : %s\n", data);
		
		// 대답이 NULL 이면 다시 받아
	}while(data == NULL);

	free(format);
	free(buf);
	return 0;
}
