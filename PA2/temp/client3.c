#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

struct sockaddr_in c_addr;
char fname[100];

void gotoxy(int x,int y)
 {
 printf("%c[%d;%df",0x1B,y,x);
 }


 void SendFileToServer(int arg)
{
      int connfd = arg;
      printf("Connection accepted and id: %d\n",connfd);
      //printf("Connected to Clent: %s:%d\n",inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
      write(connfd,fname,256);

        FILE *fp = fopen(fname,"rb");
        if(fp==NULL)
        {
            printf("File open error");
            return 1;   
        }   

        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);
            //printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
            if(nread > 0)
            {
                //printf("Sending \n");
                write(connfd, buff, nread);
            }
            if (nread < 1024)
            {
                if (feof(fp))
		{
                    printf("End of file\n");
		    printf("File transfer completed for id: %d\n",connfd);
		}
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }
printf("Closing Connection for id: %d\n",connfd);
close(connfd);
shutdown(connfd,SHUT_WR);
sleep(2);
}



int main(int argc, char *argv[])
{
    system("clear");
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    /* Create a socket first */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); // port
    char ip[50];

if (argc < 2) 
{
    printf("Enter IP address to connect: ");
    gets(ip);
}
else
  strcpy(ip,argv[1]);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    /* Attempt a connection */
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connected to ip: %s : %d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));


    printf("Enter file name to send: ");
    gets(fname);

    printf("Waiting...\n");
    
    SendFileToServer(sockfd);
   	 /* Create file where data will be stored */
    // 	FILE *fp;
	// char fname[100];
	// read(sockfd, fname, 256);
	// //strcat(fname,"AK");
	// printf("File Name: %s\n",fname);
	// printf("Receiving file...");
   	//  fp = fopen(fname, "ab"); 
    // 	if(NULL == fp)
    // 	{
    //    	 printf("Error opening file");
    //      return 1;
    // 	}
    // long double sz=1;
    /* Receive data in chunks of 256 bytes */
    // while((bytesReceived = read(sockfd, recvBuff, 1024)) > 0)
    // { 
    //     sz++;
    //     gotoxy(0,4);
    //     printf("Received: %llf Mb",(sz/1024));
	// fflush(stdout);
    //     // recvBuff[n] = 0;
    //     fwrite(recvBuff, 1,bytesReceived,fp);
    //     // printf("%s \n", recvBuff);
    // }

    // if(bytesReceived < 0)
    // {
    //     printf("\n Read Error \n");
    // }
    // printf("\nFile OK....Completed\n");
    return 0;
}
