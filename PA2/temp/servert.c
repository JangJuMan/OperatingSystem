#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>

ssize_t total = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *writefile(int *arg)
{
	pthread_mutex_lock(&m);
	int sockfd = (int)*arg;
	printf("inside the function: %d\n", sockfd);

	FILE *fp;
        char fname[100];
        read(sockfd, fname, 256);
        printf("File Name: %s\n",fname);//////////
        printf("Receiving file...");
        fp = fopen(fname, "wb");

        if(NULL == fp)
        {
         printf("Error opening file");
         return 1;
        }
	

        ssize_t n;

        char buff[4096] = {0};
        while((n = read(sockfd, buff, 4096)) > 0)
        {
                total += n;

        if(n == -1)
        {
                perror("Receive File Error");
                exit(1);
        }

        if(fwrite(buff, sizeof(char), n, fp) != n)
        {
                perror("Write File Error");
                exit(1);
        }

        memset(buff, 0, 4096);
        }
	pthread_mutex_unlock(&m);
	sleep(1);
}


void gotoxy(int x,int y)
 {
 printf("%c[%d;%df",0x1B,y,x);
 }

struct sockaddr_in c_addr;
//char fname[100];

// void* SendFileToClient(int *arg)
// {
//       int connfd=(int)*arg;
//       printf("Connection accepted and id: %d\n",connfd);
//       printf("Connected to Clent: %s:%d\n",inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
//       write(connfd,fname,256);

//         FILE *fp = fopen(fname,"rb");
//         if(fp==NULL)
//         {
//             printf("File open error");
//             return 1;   
//         }   

//         /* Read data from file and send it */
//         while(1)
//         {
//             /* First read file in chunks of 256 bytes */
//             unsigned char buff[1024]={0};
//             int nread = fread(buff,1,1024,fp);
//             //printf("Bytes read %d \n", nread);        

//             /* If read was success, send data. */
//             if(nread > 0)
//             {
//                 //printf("Sending \n");
//                 write(connfd, buff, nread);
//             }
//             if (nread < 1024)
//             {
//                 if (feof(fp))
// 		{
//                     printf("End of file\n");
// 		    printf("File transfer completed for id: %d\n",connfd);
// 		}
//                 if (ferror(fp))
//                     printf("Error reading\n");
//                 break;
//             }
//         }
// printf("Closing Connection for id: %d\n",connfd);
// close(connfd);
// shutdown(connfd,SHUT_WR);
// sleep(2);
// }

int main(int argc, char *argv[])
{
    int connfd = 0,err;
    pthread_t tid; 
    struct sockaddr_in serv_addr;
    int listenfd = 0,ret;
    char sendBuff[1025];
    int numrv;
    size_t clen=0;
    int bytesReceived = 0;
    char recvBuff[1024];

    memset(recvBuff, '0', sizeof(recvBuff));

	/*socket creation*/
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd<0)
	{
	  printf("Error in socket creation\n");
	  exit(2);
	}

    printf("Socket retrieve success\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

	/*binding*/
    ret=bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(ret<0)
    {
      printf("Error in bind\n");
      exit(2);
    }

	/*listening*/
    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }

   
while(1){   
 /*accept*/
     connfd = accept(listenfd, (struct sockaddr*)&c_addr,&clen);
        if(connfd<0)
        {
			printf("Error in accept\n");
			exit(2);	
		}
	printf("accept success:%d\n", connfd);

	err = pthread_create(&tid, NULL, writefile, &connfd);
	if(err != 0)
	printf("\n error in thread creation: [%s]", strerror(err));
}
	printf("asdf\n");
	pthread_join(err, NULL);
      

 /* Create file where data will be stored */
 /*   FILE *fp;
	char fname[100];
	read(connfd, fname, 256);
	//strcat(fname,"AK");
	printf("File Name: %s\n",fname);
	printf("Receiving file...");
   	 fp = fopen(fname, "wb"); 
    	if(NULL == fp)
    	{
       	 printf("Error opening file");
         return 1;
    	}

	printf("start receiving file \n");
	writefile(connfd, fp);
	printf("success \n");
}*/

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
	exit(2);
    }
    printf("\nFile OK....Completed\n");

    printf("Closing Connection for id: %d\n",connfd);
    close(connfd);
    shutdown(connfd,SHUT_WR);
    sleep(2);

    return 0;
}
