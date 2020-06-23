#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define SERVER_PORT 8000
#define LISTEN_ENQ 5
#define MAXSZ 100
#define MAXCLIENTS 4

int count=0;
int clientCount = 0;
int serverSocket, newSocket[MAXCLIENTS];


void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

void *chatServer(void *arg){
  int newSockID = *((int *) arg);
  size_t bytes = 0;
  char buffer[256];
  memset(buffer, '0', 256);
  int destID;
  while(1){
      bytes = recv(newSockID, buffer, 255, 0);
      if(bytes > 0){
        if(strcmp(buffer, "bye\n") == 0)
          pthread_exit(NULL);
        destID = (int) buffer[0] - '0';
        buffer[0] = '0' + newSockID;
        send(destID, buffer, 255, 0);
      }
	}
}


int main(int argc, char* argv[])
{
	int listen_fd, sock_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;
	pid_t child_pid; /* pid of child process */
	char print_addr [INET_ADDRSTRLEN];
	int n;
	char s[12];
 	char msg[MAXSZ];
 	int clientAddressLength;
	pthread_t ThIds[MAXCLIENTS];
	char online[20];

	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);

	if ( (listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	/* bind to created socket */
	if( bind(listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0 ){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	printf("Listening on port number %d ...\n", ntohs(server_addr.sin_port));
	if( listen(listen_fd, LISTEN_ENQ) < 0 ) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	signal (SIGCHLD, sig_chld);

	while(1)
	{

		client_len = sizeof(client_addr);
		printf ("Waiting for a client to connect...\n\n");
		if ( (sock_fd = accept(listen_fd, (struct sockaddr*) &client_addr,&client_len)) < 0 )
		{
			perror("accept error");
		}
		inet_ntop(AF_INET, &(client_addr.sin_addr), print_addr, INET_ADDRSTRLEN);
		printf("Client connected from %s:%d\n", print_addr, ntohs(client_addr.sin_port) );
		count++;
		sprintf(s, "Hello Client %d", sock_fd);
		n=strlen(s)+1;
		send(sock_fd,s,n,0);
		int i,j=0;
		/*for(i=0;i<clientCount;i++){
			if(i!=clientCount-1){
				sprintf(&online[j], "%d", newSocket[i]);
				sprintf(&online[++j], ",");
				++j;
			}
			else{
					sprintf(&online[j], "%d", newSocket[i]);
			}
		}
		printf("Online Clients: %s\n",online);*/
    //sprintf(msg, "Online Clients: %s",online);
		//send(sock_fd,online,20,0);
		if(sock_fd>0){
				newSocket[clientCount] = sock_fd ;
				printf("Client %d is online.\n", newSocket[clientCount]);
				pthread_create(&ThIds[clientCount], NULL, chatServer, &newSocket[clientCount]);
				++clientCount;
			}
		}
		for(int i = 0; i < MAXCLIENTS; ++i){
	    pthread_join(ThIds[i], NULL);
	    close(newSocket[i]);
	  }
	close(sock_fd);
	close(listen_fd);
	return 0;
}
