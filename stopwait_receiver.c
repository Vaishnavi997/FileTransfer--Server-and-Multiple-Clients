/*STOP AND WAIT ARQ PROTOCOL- CLIENT SIDE*/
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 256
#define PORT 8080
#define IP_ADDRESS "" //enter server IP here

typedef struct 
{
  char data[SIZE];
}Packet;

typedef struct
{
  int sq_no;
  Packet packet;  
}Frame;

int main()
{
  struct sockaddr_in address;
  
  int socket_id,bytes_received;
  int addrlen=sizeof(address);
  char arr[2];
  char filename[70];
  char buffer[SIZE];
  struct timeval tv;

  memset(&buffer,'\0',SIZE); 
  memset(&filename,'\0',sizeof(filename)); 
  Frame ack_frame, recv_frame;
 
  if((socket_id=socket(AF_INET,SOCK_STREAM,0))<0)
  {
    perror("\nSocket Failed!\n");
    exit(EXIT_FAILURE);
  }
   
  memset(&address,'\0',addrlen); 
  address.sin_family=AF_INET;
  address.sin_port=htons(PORT);

  if(inet_pton(AF_INET,IP_ADDRESS,&address.sin_addr)<=0)
  {
    printf("\nInvalid IP address\n");
    return -1;
  }

  if(connect(socket_id,(struct sockaddr*)&address,addrlen))
  {
    printf("\nConnection failed\n");
    return -1;
  }

  printf("\nEnter the file name : ");
  scanf("%s",filename);
  
  send(socket_id,filename,strlen(filename),0);
  
  printf("\nRequest sent to server\n");
  
  recv(socket_id,arr,sizeof(arr),0);

  if(strcmp(arr,"y")!=0)
  {
    printf("\nData requested not available in the server!!\n\n");
    return -1;
  }
  else
  {
    FILE *fp=fopen("received_file.txt", "w+"); 
    if(NULL==fp)
    {
        printf("\nData requested not available!!\n");
        return -1;
    }
    
    ack_frame.sq_no=0;
    while((bytes_received=recv(socket_id, &recv_frame, sizeof(recv_frame), 0))>0)
    {   
      memset(&ack_frame.packet.data,'\0',SIZE);   
      while(1)
      {
        if(bytes_received>0 && recv_frame.sq_no==ack_frame.sq_no)
        {
            fprintf(fp, "%s", recv_frame.packet.data);
            printf("\nFrame received: %d", recv_frame.sq_no);
            ack_frame.sq_no=1-ack_frame.sq_no;
            send(socket_id, &ack_frame, sizeof(ack_frame), 0);
            printf("  Ack sent: %d\n", ack_frame.sq_no);
            break;
        }
        else
        {
          send(socket_id, &ack_frame, sizeof(ack_frame), 0);
        }
      }   
    }
    printf("\nData was received sucessfully");
    printf("\n\nData is copied in the file received_file.txt\n\n");
  }
 return 0;
}



