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
#define MAX_DATA 4096 // size of the buffer, passed to send and receive

// --------------- GLOBAL --------------
// SERVER STRUCT
struct sockaddr_in server;

// CLIENT STRUCT
struct sockaddr_in client;

// SOCKET SIZE
unsigned int sockaddr_len;




// ------------- FUNCTIONS -------------
// MD5 HASH
int hash(char * filename){

    int bytes;
    unsigned char data[1024];
    //int i = 0;
    int md5hash;
    unsigned char c[MD5_DIGEST_LENGTH];
    //char *filename="1.txt";
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    //md5 hash of a large file, provided the string 
    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0){
        MD5_Update (&mdContext, data, bytes);
    }
    MD5_Final (c,&mdContext);
    // while(i < MD5_DIGEST_LENGTH) 
    // {
    //     printf("%02x\n", c[i]);
    //     i++;
    // }
    md5hash = c[MD5_DIGEST_LENGTH - 1] % 4;
    //printf ("\n%d\n", md5hash);
    //printf ("%s\n", filename);
    fclose (inFile);
    return md5hash;
}

// SERVER SOCKET
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

// FORKING HANDLER
void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// REAP CHILD PROCESSES
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

// SERVER ACCEPT
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

// POLL TIMER
int timer(char * timeoutCH, int new){
	int timeoutIN = atoi(timeoutCH);

	int polltimer;
	struct pollfd ufds[1];
	ufds[0].fd = new;
	ufds[0].events = POLLIN;
	polltimer = poll(ufds, 1, timeoutIN);
	return polltimer;
}

// PARSE URL TO www.example.com	
char *parseURL(char * URL){
    memmove(URL, URL+7, strlen(URL));
    int check = 0;
    for(int i = 0; i < strlen(URL); i++){
    	if(URL[i] == '/'){
    		check = 1;
    	}
    	if(check == 1){
    		URL[i] = '\0';
    	}
    }
    return URL;
}

// MAKE CONNECTION
int connectport(const char *portnumber, char *URL){
    
    int sock;
    int connectError;
    
    // ESTABLISH SOCKET
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR){
        perror("socket");
        exit(-1);
    }

    // PARSE URL
    parseURL(URL);
    
    // 
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the result

	// INPUT STRUCT getadderinfo()
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;  	 // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	if ((status = getaddrinfo(URL, portnumber, &hints, &servinfo)) != 0) {
	    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	    exit(1);
	}

	// CONNECT
    connectError = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen);

    freeaddrinfo(servinfo); // free the linked-list

    return sock;
}

// FORWARD RESPONSE TO SERVER AND BACK TO CLIENT
int forwarding(char *method, char *httpVersion, char *URL, int new, char *portnumber, char * data){

	int remoteServer;
	char *response = calloc(MAX_DATA, 1);
	
	// MAKE CONNECTION TO REMOTE SERVER
	remoteServer = connectport("80", URL);

	// CHECK HASH
	// if(hash){
		// send to client
	//}
	//else{

	// SEND GET HEADER FROM CLIENT
		send(remoteServer, data, strlen(data), 0);
	//}

	// READ RESPONSE FROM REMOTE SERVER
	read(remoteServer, response, MAX_DATA);

	// HASH THE VALUE

	printf("%s\n", response);

	send(new, response, strlen(response), 0);

	free(response);
	return 0;
}

int main(int argc, char *argv[]){
	// SOCKET SIZE
	sockaddr_len = sizeof(struct sockaddr_in);

	// SOCKET VAR
	int sock; // server socket
	int new;  // client socket

	// HEADER VAR
	char requestLine[256];
	char method[256];
	char httpVersion[256];
	char URL[256];

	int timeout;

	sock = establishSocket(argv[1]);
	
	// CHILD PROCESSES
	reap();
	while(1){// WAIT FOR CLIENT CONNECTION
		new = acceptConnection(sock);
		if(!fork()){// FORK PROCESS
			close(sock);

			while(1){// PROCESS REQUEST
				char *header = calloc(MAX_DATA, 1);
				char *headerCopy = calloc(MAX_DATA, 1);
				
				// TIMEOUT
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

					headerError = recv(new, header, MAX_DATA, 0);
					if(headerError < 0){ // ERROR CHECKING
						fputs("Error reading file", stderr);
						char *err500Internal = "HTTP/1.1 500 Internal Server Error\n\n"; 
						send(new, err500Internal, strlen(err500Internal), 0);
						exit(-1);
					}
					printf("%s\n", header);

					char URLCopy[strlen(URL)];
					strcpy(URLCopy, URL);

					// PARSE HEADER 
					strcpy(headerCopy,header); // COPY FOR STRTOK
					memcpy(requestLine, strtok(headerCopy,"\n"),256);
					memcpy(method, strtok(requestLine," "), 256); 	// GET/PUT/LIST
					memcpy(URL, strtok(NULL," "), 256);				// www.yahoo.com
					memcpy(httpVersion, strtok(NULL," "), 256);		// HTTP/1.0

					// CHECK URL
					int errURL = 1;
					if(strcmp(method, "GET") == 0){
						if(strcmp(httpVersion,"HTTP/1.0\r") != 0){
							// error
							printf("Wrong Version\n");
						}
						else if(errURL == 0){
							// error
							printf("Wrong URL\n");
						}
						else{
							forwarding(method, httpVersion, URL, new, argv[1], header);
						}
					}
					else{
						printf("Wrong Method\n");
					}
				}
				free(header);
				free(headerCopy);
			}
			close(new);
		}
		close(new);
	}
	return 0;
}