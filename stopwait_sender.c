/*STOP AND WAIT ARQ PROTOCOL-SERVER SIDE*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include <time.h>
#define PORT 8080
#define SIZE 256

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
  int addrlen=sizeof(address);
  int socket_id,new_socket,opt=1;

  if((socket_id=socket(AF_INET,SOCK_STREAM,0))==0)
  {
    perror("Socket Failed!\n");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }  

  memset(&address,'\0',addrlen);
  address.sin_family=AF_INET;
  address.sin_port=htons(PORT);
  address.sin_addr.s_addr=INADDR_ANY;
 
  if(bind(socket_id,(struct sockaddr*)&address,addrlen)<0)
  {
    perror("Bind Failed!\n");
    exit(EXIT_FAILURE);
  } 
  
  if(listen(socket_id,7)<0)
  {
    perror("Listen Failed!\n");
    exit(EXIT_FAILURE);
  } 
 
    int r,bytes_read=0,bytes_received=0;
    char arr[2]={0};
    char filename[25]={'\0'};
    char buffer[SIZE]={'\0'};
    struct timeval tv;
    memset(&buffer,'\0',SIZE); 
    memset(&filename,'\0',sizeof(filename));    

    Packet packet;
    Frame frame;
    Frame recv_frame;
  
    if((new_socket=accept(socket_id,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0)
    {
      perror("accept failed!");
      exit(EXIT_FAILURE);
    }
   
      printf("\n\nConnected to client : %s",inet_ntoa(address.sin_addr));
      r=recv(new_socket,filename,sizeof(filename),0);
      printf("\n\nThe requested data from client : %s\n\n",filename);
    
      FILE *f=fopen(filename,"r");
      if(f==NULL)
      {
        printf("Data requested not available!!\n");
        printf("\n----------Client disconnected--------------\n");
        close(new_socket);
      }
     else
     {
       memset(arr, '\0', 2);
       memset(arr, 'y', 1);
       send(new_socket,arr,sizeof(arr),0);
      
       tv.tv_sec=1; //1 sec timeout
       tv.tv_usec=0;

       setsockopt(socket_id,SOL_SOCKET,SO_RCVTIMEO,(char*)&tv,sizeof(struct timeval));
       
       int frame_count=0;
       int sqno=1;
       int waiting_ack;
       while((bytes_read=fread(buffer,sizeof(char),SIZE,f))>0)
       { 
         memset(&packet.data,'\0',SIZE);
         memset(&frame.packet,'\0',sizeof(Packet));
         strcpy(packet.data,buffer);
         memcpy(&(frame.packet),&packet,sizeof(Packet));
         waiting_ack=sqno;
         sqno=1-sqno;
         frame.sq_no=sqno;
         while(1)
         {
            send(new_socket,&frame,sizeof(frame),0);
            printf("Frame sent: %d\n",frame.sq_no);
            bytes_received=recv(new_socket,&recv_frame,sizeof(recv_frame),0);
            
           if(bytes_received>0 && recv_frame.sq_no==waiting_ack)
           {
              printf("\nAcknowledgement received: %d   ",recv_frame.sq_no);
              memset(&buffer,'\0',SIZE);
              break;
           }
           else
           {
              printf("\nFrame time expired: %d",frame.sq_no);
           }
         }
         frame_count++;
       }
       printf("\n\nTotal no. of frames sent: %d",frame_count);
       printf("\n\nThe data was sent successfully\n"); 
       close(new_socket);
       printf("\n----------Client disconnected--------------\n");
    }   
  return 0;
}
