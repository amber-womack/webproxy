//PROXY SERVER
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for the sockets
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h> // for atoi
#include <unistd.h> //for closing
#include <strings.h>
#include <errno.h> //for perror
#include <time.h>
#include <sys/stat.h>
#include <netinet/in.h> // for the structures
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h> //for forking
#include <sys/poll.h> //for the timer function

#define ERROR -1 //the error
#define MAX_CLIENTS 10 // max clients to have in wait queue, listen call used by kernel
#define MAX_DATA 1024 // size of the buffer, passed to send and receive

// SERVER STRUCT
struct sockaddr_in server;

// CLIENT STRUCT
struct sockaddr_in client;

// SOCKET SIZE
unsigned int sockaddr_len;

int establishSocket(char * portnum){
	int sock; // server socket

	// ESTABLISH SOCKET
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR){
		perror("socket: ");
		exit(-1);
	}

	// SERVER DATA_STRUCT
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(portnum));
	server.sin_addr.s_addr = INADDR_ANY; 
	bzero(&server.sin_zero, 8);

	// BIND PORT TO SOCKET
	if((bind(sock, (struct sockaddr *) &server, sockaddr_len)) == ERROR){
		perror("bind: ");
		exit(-1);
	}

	// LISTEN ON SOCKET
	if((listen(sock, MAX_CLIENTS)) == ERROR){
		perror("listen: ");
		exit(-1);
	}
	return sock;
}

void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

void reap(){
	// REAP
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}
}

int acceptConnection(int sock){
	int new;
	// ACCEPT NEW SOCKET
	if((new = accept(sock, (struct sockaddr *)&client, &sockaddr_len)) == ERROR){
		perror("accept: ");
		exit(-1);
	}
	printf("New Client connected from port no %d and IP %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
	return new;
}

int timer(char * timeoutCH, int new){
	int timeoutIN = atoi(timeoutCH);

	int polltimer;
	struct pollfd ufds[1];
	ufds[0].fd = new;
	ufds[0].events = POLLIN;
	polltimer = poll(ufds, 1, timeoutIN);
	return polltimer;
}

int main(int argc, char *argv[]){
	// SOCKET SIZE
	sockaddr_len = sizeof(struct sockaddr_in);

	// SOCKET VAR
	int sock; // server socket
	int new;  // client socket
		
	// HEADER VAR
	char *requestLine;
	char *method;
	char *httpVersion;
	char *connection;
	char *URL;

	int timeout;
	char data[MAX_DATA];

	sock = establishSocket(argv[1]);
	
	// CHILD PROCESSES
	reap();
	while(1){// WAIT FOR CLIENT CONNECTION

		new = acceptConnection(sock);
		if(!fork()){// FORK PROCESS
			close(sock);

			while(1){// PROCESS REQUEST

				timeout = timer(argv[2], new);
				if(timeout == -1){ // ERROR IN TIMER
					perror("poll");
				}
				else if(timeout == 0){ // TIMEOUT
					printf("Timeout occurred! Process stopped. No requests sent after %s seconds. \n", argv[2]);
    				break;
				}
				else{ // HANDLE REQUEST

					// READ HEADER
					int headerError;
					headerError = read(new, data, MAX_DATA);
					if(headerError < 0){ // ERROR CHECKING
						fputs("Error reading file", stderr);
						char *err500Internal = "HTTP/1.1 500 Internal Server Error\n\n"; 
						send(new, err500Internal, strlen(err500Internal), 0);
						exit(-1);
					}

					printf("%s\n", data);

					// PARSE HEADER
					requestLine = strtok(data,"\n");
					method = strtok(requestLine," ");
					URL = strtok(NULL," ");
					httpVersion = strtok(NULL," ");

				}
			}
			close(new);
		}
		close(new);
	}
	return 0;
}