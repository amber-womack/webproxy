//TCP Webserver
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

//GLOBALS
char * portnum;
char * rootdoc;
char * docroot;
char * directory;
char * contenttype;
char * URL;
/*-----------------------------------------------------------------------------------------------------*/
//GET FUNCTION
int get(char *method, char *httpVersion, char *connection, char *newFilePath, int new){

	// 200
	// REST
	char *header200 = "HTTP/1.1 200 0K\n";
	send(new, header200, strlen(header200), 0); // send data to client

	// Date and Time
	time_t current_time;
    char *c_time_string;
    current_time = time(NULL); // get current time
	c_time_string = ctime(&current_time); // convert time

	char date[60] = "Date: ";
	strcat(date, c_time_string); // strcat concatenates strings, (pointer to DEST, source)
	send(new, date, strlen(date), 0); // send data to client

	// Content-Length of the document
	struct stat st;
	stat(newFilePath, &st);	// gets the size
	
	char str[256];
	sprintf(str, "%lld", st.st_size); // convert to string
	char contentLength[24];
	strcat(contentLength, "Content-Length: ");
	strcat(contentLength, str);
	send(new, contentLength, strlen(contentLength), 0);
	send(new, "\n", 1, 0);

	// Conection
	if(connection){
		int CONLEN = 12 + strlen(connection);
		char con[CONLEN];
		strcat(con, "Connection: ");
		strcat(con, connection);
		//strcat(con, "\n");
		send(new, con, CONLEN, 0);
		//send(new, "\n\n", 1, 0);
		
	}

	// Connection-Type
	char *ext;
	char *exten;
	ext = strrchr(newFilePath, '.');
	if (!ext) {
    	//no extension
	} else {
    	exten = ext + 1;
	}
	if(!strcmp(exten, "html")){
		send(new, "Content-Type: text/html", 23, 0);
	}
	else if(!strcmp(exten, "htm")){
		send(new, "Content-Type: text/plain", 24, 0);
	}
	else if(!strcmp(exten, "png")){
		send(new, "Content-Type: image/png", 23, 0);
	}
	else if(!strcmp(exten, "gif")){
		send(new, "Content-Type: image/gif", 23, 0);
	}
	else if(!strcmp(exten, "jpg")){
		send(new, "Content-Type: image/jpg", 23, 0);
	}
	else if(!strcmp(exten, "css")){
		send(new, "Content-Type: text/css", 22, 0);
	}
	else if(!strcmp(exten, "js")){
		send(new, "Content-Type: text/javascript", 29, 0);
	}
	else if(!strcmp(exten, "ico")){
		send(new, "Content-Type: image/x-icon", 26, 0);
	}
	
	
	send(new, "\n", 1, 0);
	send(new, "\n", 1, 0);
	//SEND THE WEBSITE BACK

	// get the html
	//char line[1024];
	// char webPage[st.st_size];
	// FILE *fp = fopen(newFilePath, "r");
	// if(fp != NULL){

 //    	size_t bytes = fread(webPage, 1, st.st_size, fp);
 //    	long long cc = send(new, webPage, st.st_size, 0);

	//     fclose (fp);
	//     return cc;
 //    }
 //    else
 //    {
 //    	perror (newFilePath); // why didn't the file open? 
 //    	printf("Didn't open the new file path: %s\n", newFilePath);
 //    	return 0;
 //    }
    return 1;
}

/*-----------------------------------------------------------------------------------------------------*/
//Parsing WS CONFIG FILE. returning the portnumber
// void parseConf(){	 
//     char * line;
//     char * piece;

//     int length;
// 	int indexing1 = 0;
// 	int indexing2 = 0;

// 	char * source = malloc(MAX_DATA);

// 	// get ws.conf
// 	FILE *fp = fopen("ws.conf", "r");
// 	if (fp != NULL){
// 	    size_t newLen = fread(source, sizeof(char), MAX_DATA, fp);
// 	    if (newLen == 0){
// 	        fputs("Error reading file", stderr);
// 	    } 
// 	    else{
// 	        source[++newLen] = '\0'; //Just to be safe add null at end of char array 
// 	    }
// 	    fclose(fp);
// 	}

// 	// parse string by each line and then each space between the line
//     while((line = strsep(&source, "\n")) != NULL){
//     	if(line[0] != '#'){
//     		if(line[0] != '.'){
// 		    	while((piece = strsep(&line, " ")) != NULL){
// 	    			if (indexing1 == 0 && indexing2 == 1){
// 						portnum = piece;
// 						indexing2 = 0;
// 					}
// 	    			else if (indexing1 == 1 && indexing2 == 1){
// 	    				rootdoc = piece;
// 	    				length = strlen(rootdoc);
// 	    				rootdoc[length-1] = '\0';
// 	    				rootdoc++;
// 	    				indexing2 = 0;
// 	    			}
// 	    			else if (indexing1 == 2 && indexing2 == 1){
// 	    				directory = piece;
// 	    			}
// 					indexing2++;
// 	    		}
// 	    		indexing2 = 0;
// 	    		indexing1++;
//     		}
//     		else{
//     			//this section parses the types of file starting with "."
//     			while((piece = strsep(&line, " ")) != NULL){
// 		    		if (indexing2 == 0){
// 	    				contenttype = piece;
// 	    				indexing2 = 2;
// 	    			}
// 	    			else if (indexing2 == 1){
// 	    				indexing2++;
// 	    				strcat(contenttype, piece);
// 	    			}
// 		    	}
// 		    	indexing2 = 1;
//     		}
//     	}
//     }
//     free(source);
// }
/*-----------------------------------------------------------------------------------------------------*/

//FUNCTION FOR SIG CHILD HANDLER, for dead child processes from forking
void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}
/*-----------------------------------------------------------------------------------------------------*/
int main(){
	//struct type server, contains port to which the server is bound locally
	//and client information connecting to us
	struct sockaddr_in server;
	struct sockaddr_in client;
	
	int sock; // server socket
	int new;  // client socket
	int errorCheck;
	char *requestLine;
	char *method;
	char *httpVersion;
	char *errorData;
	char *connection;

	unsigned int sockaddr_len  = sizeof(struct sockaddr_in); //size of the socket calls
	
	// Length of the data
	int data_len;
	//character array that is allocated in memory, defined size at top
	char data[MAX_DATA];
	char dataCopy[MAX_DATA];

	//WS.CONF AND PARSE
	//parseConf();

	// ESTABLISH SOCKET - allows for a physical connection between the client and server
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR){
		perror("socket: ");
		exit(-1);
	}

	// SERVER DATA_STRUCT
	//filling the server data structure
	server.sin_family = AF_INET;
	//The port number from parsing config file and changing to network byte order
	server.sin_port = htons(atoi(portnum));
	//to find what address the server will bind too, and listen on all interfaces
	server.sin_addr.s_addr = INADDR_ANY; 
	//for padding, its 8 bytes long
	bzero(&server.sin_zero, 8);

	// BIND PORT TO SOCKET
	//give the server address, a pointer cast too sockaddr 
	if((bind(sock, (struct sockaddr *) &server, sockaddr_len)) == ERROR){
		perror("bind: ");
		exit(-1);
	}

	// LISTEN ON SOCKET
	//to listen for a number of connections
	if((listen(sock, MAX_CLIENTS)) == ERROR){
		perror("listen: ");
		exit(-1);
	}
	// reap all dead processes
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	while(1){ // WAIT FOR CLIENT CONNECTION

		// BLOCK ON ACCEPT, WAIT FOR CLIENT
		//we block on accept, accept returns when the client comes in
		//recevie data from the client, sends back the data to the client
		//once done, keeps waiting for new connections
		//soket descriptor for which client is talking to 
		//Call access to block, waiting for client
		if((new = accept(sock, (struct sockaddr *)&client, &sockaddr_len)) == ERROR){
			perror("accept: ");
			exit(-1);
		}
		//print that the client is accepted and which port and IP
		printf("New Client connected from port no %d and IP %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));


		data_len = 1;
		//THREADING/FORKING- ability to handle multiple requests
		if(!fork()){
			//child process
			close(sock);
			while(1){
				//TIMER - Set to 10 seconds
				int polltimer;
				//watches sets of file descriptors for events, such as incoming data ready to recv(), 
				//socket ready to send() data to, out-of-band data ready to recv(), errors, etc.
				//Returns the number of elements in the ufds array that have had event occur on them; 
				//this can be zero if the timeout occurred. Also returns -1 on error 
				struct pollfd ufds[1];
				ufds[0].fd = new; //fd = socket descriptor
				ufds[0].events = POLLIN; //Alert me when data is ready to recv() on this socket.
				polltimer = poll(ufds, 1, 10000);
				if(polltimer == -1) {
    				perror("poll"); // error occurred in poll()
				} //child process closes the socket and exit to prevent zombie children(doesn't here other request). 
				else if (polltimer == 0) {
    				printf("Timeout occurred! Process stopped. No requests sent after 10 seconds. \n");
    				break; //break out of the while loop to stop the process
				}
				else{
					//READS DATA FROM CLIENT
					data_len = read(new, data, MAX_DATA);
					//if theres -1, then handles an ERROR 500 for INTERNAL SERVER ERROR
					if(data_len<0){
						fputs("Error reading file", stderr);
						char *err500Internal = "HTTP/1.1 500 Internal Server Error\n\n"; 
						send(new, err500Internal, strlen(err500Internal), 0);
						exit(-1);
					}
					// copy data request, (pointer to DEST where to be copied, string to copy)
					strcpy(dataCopy, data); 

					// PULL AND PARSE REQUEST of GET / HTTP/1.1
					//str -The contents of this string are modified and broken into smaller strings (tokens).
					//delim -This is the C string containing the delimiters. (str, delim)
					requestLine = strtok(data,"\n");
					method = strtok(requestLine," ");
					URL = strtok(NULL," ");
					httpVersion = strtok(NULL," ");

					if (strcmp(method, "GET") == 0){
			            printf("valid GET request\n");
			            if(method == NULL){
			                printf("invalid file for GET\n");
			                exit(-1);
			            } 
			           //get(method, httpVersion, connection, newFilePath, new);    
			        //}
						// CONNECTION TYPE - covering all possible cases
						//haystack - This is the main C string to be scanned
						//needle - This is the small string to be searched with-in haystack string
						if(strstr(dataCopy, "keep-alive")){
							connection = "keep-alive";
							//send(new, connection, strlen(connection), 0);
						}else if(strstr(dataCopy, "Keep-alive")){
							connection = "keep-alive";
							//send(new, connection, strlen(connection), 0);
						}
						else if(strstr(dataCopy, "closed")){
							connection = "close";
							//send(new, connection, strlen(connection), 0);
						}
						else if(strstr(dataCopy, "Closed")){
							connection = "close";
							//send(new, connection, strlen(connection), 0);
						}
						else if(strstr(dataCopy, "close")){
							connection = "close";
							//send(new, connection, strlen(connection), 0);
						}
						else{
							connection = NULL;
						}

						//SET FILE PATH 
						//rootdoc is from parse == documentRoot to index.html, or setting up the file path
						char * newFilePath = calloc(100,1);
						strcat(newFilePath,rootdoc);
						if(strcmp(URL, "/") == 0){
							strcat(newFilePath,"/index.html");
						}
						else{
							strcat(newFilePath, URL);
						}

						// BEGIN ERROR CHECK
						errorCheck = 1;
						
						// ERROR CHECK FOR METHOD
						if(strcmp(method,"GET") && errorCheck == 1){
							char *len400METH = calloc(50,1);
							char *html400METH = calloc(500,1);
							
							char *err400METH = "HTTP/1.1 400 Bad Request\n";
							char *type400METH = "Content-type: text/html\n";
							
							strcat(html400METH,"<html><em> HTTP/1.1 400 Bad Request: Invalid Method: ");
							strcat(html400METH, method);
							strcat(html400METH," </em></html>");

							char str[256];
							sprintf(str, "%ld", strlen(html400METH)); // convert to string
							strcat(len400METH, "Content-length: ");
							strcat(len400METH, str);

							send(new, err400METH, strlen(err400METH), 0);
							send(new, type400METH, strlen(type400METH), 0);
							send(new, len400METH, strlen(len400METH), 0);
							send(new,"\n\n",2,0);
							send(new, html400METH, strlen(html400METH)+1, 0);

							free(len400METH);
							free(html400METH);

							errorCheck = 0;
							data_len = 0;
						}

						// ERROR CHECK FOR VERSION 
						if(((strcmp(httpVersion, "HTTP/1.1\r") != 0) && (strcmp(httpVersion,"HTTP/1.0\r") != 0)) && errorCheck == 1){
							char *len400VER = calloc(50,1);
							char *html400VER = calloc(500,1);
							
							char *err400VER = "HTTP/1.1 400 Bad Request\n";
							char *type400VER = "Content-type: text/html\n";
							
							strcat(html400VER,"<html><em> HTTP/1.1 400 Bad Request: Invalid Version: ");
							strcat(html400VER, httpVersion);
							strcat(html400VER," </em></html>");

							char str[256];
							sprintf(str, "%ld", strlen(html400VER)); // convert to string
							strcat(len400VER, "Content-length: ");
							strcat(len400VER, str);

							send(new, err400VER, strlen(err400VER), 0);
							send(new, type400VER, strlen(type400VER), 0);
							send(new, len400VER, strlen(len400VER), 0);
							send(new,"\n\n",2,0);
							send(new, html400VER, strlen(html400VER)+1, 0);

							free(len400VER);
							free(html400VER);

							errorCheck = 0;
							data_len = 0;
						}
						
						// ERROR CHECK FOR URL
						char * testURL = calloc(100,1);
						strcpy(testURL,newFilePath);
						int index = strlen(testURL);	

						while(testURL[index] != '/'){
							testURL[index] = '\0';
							index--;
						}

						struct stat pathTest;
						int err = stat(testURL, &pathTest);
						// ERROR CHECK FOR 400 , INVALID URL 
						if(err == -1 && errorCheck == 1){
							char *len400URL = calloc(50,1); // zero out the buffer, so nothing is left in it
							char *html400URL = calloc(500,1);
							
							char *err400URL = "HTTP/1.1 400 Bad Request\n";
							char *type400URL = "Content-type: text/html\n";
							
							strcat(html400URL,"<html><em> HTTP/1.1 400 Bad Request: Invalid URL: ");
							strcat(html400URL, testURL);
							strcat(html400URL," </em></html>");

							char str[256];
							sprintf(str, "%ld", strlen(html400URL)); // convert to string
							strcat(len400URL, "Content-length: ");
							strcat(len400URL, str);

							send(new, err400URL, strlen(err400URL), 0);
							send(new, type400URL, strlen(type400URL), 0);
							send(new, len400URL, strlen(len400URL), 0);
							send(new,"\n\n",2,0);
							send(new, html400URL, strlen(html400URL)+1, 0);

							free(len400URL);
							free(html400URL);

							errorCheck = 0;
							data_len = 0;
						}	
						// ERROR CHECK FOR 404 NOT FOUND
						err = 0;
						err = access(newFilePath,F_OK);

						if(err == -1 && errorCheck == 1){
							char *len404 = calloc(50,1);
							char *html404 = calloc(500,1);
							
							char *err404 = "HTTP/1.1 404 Not Found\n";
							char *type404 = "Content-type: text/html\n";
							
							strcat(html404,"<html><em> HTTP/1.1 404 Not Found: ");
							strcat(html404, newFilePath);
							strcat(html404," </em></html>");

							char str[256];
							sprintf(str, "%ld", strlen(html404)); // convert to string
							strcat(len404, "Content-length: ");
							strcat(len404, str);

							send(new, err404, strlen(err404), 0);
							send(new, type404, strlen(type404), 0);
							send(new, len404, strlen(len404), 0);
							send(new,"\n\n",2,0);
							send(new, html404, strlen(html404)+1, 0);

							free(len404);
							free(html404);

							errorCheck = 0;
							data_len = 0;
						}	
						//ERROR CHECK FOR 501
						char * ext = strrchr(newFilePath, '.');
						if(!(strstr(contenttype, ext)) && errorCheck == 1){
							char *len501NotImplemented = calloc(50,1);
							char *html501NotImplemented = calloc(500,1);
							
							char *err501NotImplemented = "HTTP/1.1 501 Not Implemented: "; //HANDLES 501 ERROR
							char *type501NotImplemented = "Content-type: text/html\n";
							
							strcat(html501NotImplemented,"<html><em> HTTP/1.1 501 Not Implemented: ");
							strcat(html501NotImplemented, ext);
							strcat(html501NotImplemented," </em></html>");

							char str[256];
							sprintf(str, "%ld", strlen(html501NotImplemented)); // convert to string
							strcat(len501NotImplemented, "Content-length: ");
							strcat(len501NotImplemented, str);

							send(new, err501NotImplemented, strlen(err501NotImplemented), 0);
							send(new, type501NotImplemented, strlen(type501NotImplemented), 0);
							send(new, len501NotImplemented, strlen(len501NotImplemented), 0);
							send(new,"\n\n",2,0);
							send(new, html501NotImplemented, strlen(html501NotImplemented)+1, 0);

							free(len501NotImplemented);
							free(html501NotImplemented);

							errorCheck = 0;
							data_len = 0;
						}

						// PROCESS GET IF NO ERRORS
						if(errorCheck == 1){
							if(get(method, httpVersion, connection, newFilePath, new) == 0)
								perror("send");
						}
						free(newFilePath);
					}
				}
			}
			//close the client
			close(new);
			exit(0);
		}
		//printf("\nClient Disconnected\n");
		close(new);
	}
}
