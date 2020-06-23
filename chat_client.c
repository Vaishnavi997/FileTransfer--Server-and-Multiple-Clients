#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_PORT 8000
#define MAXSZ 100

char sendBuffer[256], recvBuffer[256];
int sock_fd, exitState = 0;

void *sendThread(void *arg){
	while(1){
		fgets(sendBuffer, 255, stdin);
		if(strcmp(sendBuffer, "bye\n") == 0){
			exitState = 1;
			exit(0);
			pthread_exit(NULL);
		}
		send(sock_fd, sendBuffer, 255, 0);
	}
}

void *recvThread(void *arg){
	size_t bytes = 0;
	while(1){
		bytes = recv(sock_fd, recvBuffer, 255, 0);
		if(bytes > 0)
			printf("%s", recvBuffer);
		if(exitState == 1)
			exit(0);
	}
}

int main(int argc, char* argv[])
{
 	int n;
 	char msg1[MAXSZ];
 	char msg2[MAXSZ];
  pthread_t sendThd, recvThd;
	struct sockaddr_in server_addr;
	if (argc < 2)
	{
		printf("Usage: %s <IP address> \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	memset(&server_addr, 0, sizeof(server_addr));

	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server_addr.sin_family = AF_INET;
	if ( inet_pton(AF_INET, argv[1], &(server_addr.sin_addr)) < 1 ) /* convert commandi line argument to numeric IP */
	{
		printf("Invalid IP address\n");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_port = htons(SERVER_PORT);

	if( connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0 )
	{
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	printf("connected to ..%d Port \n",SERVER_PORT);
	n=recv(sock_fd,msg2,MAXSZ,0);
 	printf("Received message from  server::%s\n",msg2);

  //n=recv(sock_fd,msg1,20,0);
 	//printf("Received message from  server::%s\n",msg1);
  pthread_create(&recvThd, NULL, recvThread, NULL);
  pthread_create(&sendThd, NULL, sendThread, NULL);
  pthread_join(sendThd, NULL);
  pthread_join(recvThd, NULL);

	if(close(sock_fd) < 0) /* close socket*/
	{
		perror("socket close error");
		exit(EXIT_FAILURE);
	}
	return 0;
}
