#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

typedef struct _node{
   char* setup;
   char* punchline;
   struct _node* next;
} node;


char* getLine(FILE* fp);
int isPunctuation(char c);
char* handleMessage(char* message, int i, int length, node* randomJoke);
char* makeJoke(int i, node* randomJoke);
char* makeError(char* error);
void chat(int connfd, node* randomJoke);
void handleFormatError(int connfd, int i, int errType);
node* getJoke(node* head, int i);

//This function receives a port and joke file, creates a server socket, and chooses a random joke to send to a client.
//It is assumed that the joke file is formatted correctly.
//The following errors are checked for: correct number of inputs, a valid port number, non-empty file, and errors in the socket creation process.
int main(int argc, char* argv[]){
	
	if (argc != 3){
	    printf("Must specify one port and one joke file\n");
	    exit(0);
    }

	for (int i = 0; i < strlen(argv[1]); ++i){
		if (!isdigit(argv[1][i])){
			printf("Port must be numeric\n");
			exit(0);
		}
	}
	int port = atoi(argv[1]);
	if (port <= 5000 || port >= 65536){
		printf("Port must be in valid range\n");
		exit(0);
	}

	FILE* fp = fopen(argv[2], "r");
	if(fp == NULL) {
        printf("error\n");
        exit(0);
    }
	node* head = NULL;
	int jokes = 0;
	//add jokes to linked list
	while(!feof(fp)){
		++jokes;
		node* newNode = malloc(sizeof(node));
		newNode->setup = getLine(fp);
		newNode->punchline = getLine(fp);
		//skip empty line
		fgetc(fp);
		if(head == NULL) {
			head = newNode;
			head->next = NULL;
		}
		else {
			newNode->next = head;
			head = newNode;
		}
	}
	fclose(fp);
	if (jokes == 0){
		printf("Empty jokes file\n");
		exit(0);
	}
	srand(time(NULL));
	//create server socket
	int sockfd, connfd;
	socklen_t len;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("Socket creation failed\n"); 
		exit(0); 
	}
	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons((unsigned short)atoi(argv[1])); 

    if ((bind(sockfd, (struct sockaddr* )&servaddr, sizeof(servaddr))) != 0) { 
		printf("Socket failed to bind\n"); 
		exit(0); 
	} 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed\n"); 
		exit(0);
	} 
	len = sizeof(servaddr); 
	//continue accepting clients
    while(1) {
		int jokeNum = rand() % jokes;
		node* randomJoke = getJoke(head, jokeNum);
        connfd = accept(sockfd, NULL, &len); 
	    if (connfd < 0) { 
		    printf("Server failed to accept\n"); 
		    exit(0); 
	    } 
        chat(connfd,randomJoke);
        close(connfd);
    }
    close(sockfd);
}
//This function reads a line from the joke file
char* getLine(FILE* fp) {
	int maxSize = 50;
  	char* token = malloc(maxSize);
	char c = fgetc(fp);
	int i = 0;
	while(c != '\n'){
    	if (i == maxSize-1){
	        token = realloc(token, 2*maxSize);
	        maxSize *= 2;            
       	}
		token[i++] = c;
        c = fgetc(fp);
		if(feof(fp)) {
			break;
		}
	}
 	token[i] = '\0';
	return token;
}

//This function finds the random joke in the jokes linked list
//It is assumed that head != NULL (guaranteed by main)
//i represents the random joke number
node* getJoke(node* head, int i){
	for (int j = 0; j < i && head != NULL; ++j){
		head = head->next;
	}
	if (head != NULL){
		return head;
	}else{
		return NULL;
	}
}

int isPunctuation(char c){
    return c == '.' || c == '?' || c == '!';
}
//This function determines if the content of a message is formatted properly
//It is assumed that randomJoke != NULL (guaranteed by main)
//length is the length specified by the received message, i is the received message number
char*  handleMessage(char* message, int i, int length, node* randomJoke) {
	if (i == 0 && strlen(message) !=length){
		return "M1LN";
	}
	else if(i == 0 && strcmp(message, "Who's there?") != 0) {
		return "M1CT";
	}

	if (i == 1 && strlen(message) !=length){
		return "M3LN";
	}
	else if(i == 1 && !(strncmp(message,randomJoke->setup,strlen(randomJoke->setup)-1) == 0 && strcmp(message + strlen(randomJoke->setup)-1, ", who?") == 0)) {
		return "M3CT";
	}

	if (i == 2 && strlen(message) !=length){
		return "M5LN";
	}
	else if(i == 2 && !isPunctuation(message[length -1])) {
		return "M5CT";
	}
	else {
		return "good";
	}

} 
//This function constructs a joke according to the KKJ application protocol.
//It is assumed that randomJoke != NULL (guaranteed by main)
//i is the sent message number
char* makeJoke(int i, node* randomJoke){
	char* jokeMessage;
	if (i == 0){
		jokeMessage = "Knock, knock.";
	}else if (i == 1){
		jokeMessage = randomJoke->setup;
	}else if (i == 2){
		jokeMessage = randomJoke->punchline;
	}
	int jokeLen = strlen(jokeMessage);
	char* formatMessage = malloc(3 + 1+ (floor(log10(jokeLen)) + 1) + 1 +  jokeLen + 1 + 1);
	strcpy(formatMessage, "REG|");
	sprintf(formatMessage + 4, "%d", jokeLen);
	strcat(formatMessage, "|");
	strcat(formatMessage, jokeMessage);
	strcat(formatMessage, "|");
	return formatMessage;
}
//This function constructs an error message according to the KKJ application protocol.
char* makeError(char* error){
    char* formatError = malloc(10);
    strcpy(formatError,"ERR|");
    strcat(formatError, error);
    strcat(formatError, "|");
    return formatError;
}
//This function receives and sends messages to the client.
//It is assumed that randomJoke != NULL (guaranteed by main).
//Checks the following errors: read() returns 0 or -1.
void chat(int connfd, node* randomJoke){
	char header[4];
	header[3] = '\0';

	for(int i =  0; i < 3;++i) {
		bzero(header, 3);
		//parse header
		char* jokeMessage = makeJoke(i, randomJoke);
		int jokeLen = strlen(jokeMessage);
		write(connfd, jokeMessage, jokeLen);
		free(jokeMessage);
		int received = 0;
		while (received < 3){
			int status = read(connfd, header + received, 3 - received);
			if (status == -1 || status == 0){
				//connection closed
				return;
			}
			received += status;

		}
		if(strcmp(header, "ERR") == 0){
			//handle error
			char* errMsg = malloc(6);
			errMsg[5] = '\0';
			received = 0;
			while (received < 5){
				int status = read(connfd, errMsg + received, 5 - received);
				if (status == -1 || status == 0){
					return;
				}
				received += status;
			}
			printf("Received message %c ", errMsg[2]);
			if (errMsg[3] == 'C'){
				printf("content error\n");
			}else if (errMsg[3] == 'L'){
				printf("length error\n");
			}else if (errMsg[3] == 'F'){
				printf("format error\n");
			}
			free(errMsg);
			return;
		}else if (strcmp(header, "REG") == 0){
			int length = 0;
			char digit;
			int gotPipe = 0;
			//parse length
			while (1){
				int got = read(connfd, &digit, 1);
				if (got == 1){
					if (!gotPipe && digit != '|'){
						//format error
						handleFormatError(connfd, i, 0);
						return;
					}else if (!gotPipe && digit == '|') {
						gotPipe = 1;
						//received pipe ending the length
					}else if (digit == '|'){
						break;
					}else if (isdigit(digit)){
						length *= 10;
						length += digit - '0';
					}else{
						//did not receive integer data in length
						//format error
						handleFormatError(connfd, i, 0);
						return;
					}
				} else if (got == 0 || got == -1){
					//connection closed
					return;
				}
			}
			char* message = malloc(length + 1);
			bzero(message, length + 1);
			received = 0;
			//parse message
			while (1){
				int status = read(connfd, message + received, 1); 
				if (status == -1 || status == 0){
					//connection closed
					return;
				}else if (message[received] == '|'){
					break;
				}
				else if(length == received) {
					handleFormatError(connfd, i, 1);
					return;
				}
				received += status;
			}
			message[received] = '\0';
			char* status = handleMessage(message, i,length,randomJoke);
			free(message);
			if(strcmp(status, "good") != 0) {
				char* error = makeError(status);
				write(connfd, error, 9);
				free(error);
				return;
			}

		}else{
			//first three chars not message type, format error
    		handleFormatError(connfd, i,0);
			return;

		}
	}
}
//This function constructs and sends an error message according to the KKJ application protocol.
//It is assumed that errType is either 0 or 1 (guaranteed by chat)
//i is the received message number
void handleFormatError(int connfd, int i, int errType) {
    int errNum = 2* i + 1;
    char errChar = errNum + '0';
    char errCode[5];
    errCode[0] = 'M';
    errCode[1] =errChar;
    errCode[2] = '\0';
    //format error
    if(errType == 0) {
        strcat(errCode, "FT");

    }
    //length error
    else if(errType == 1) {
        strcat(errCode, "LN");
    }
    char* error = makeError(errCode);
    write(connfd, error, 9);
    free(error);
}

