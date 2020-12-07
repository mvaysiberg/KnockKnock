#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <ctype.h>
#include <math.h>

int isPunctuation(char c);
char*  handleMessage(char* message, int i, int length);
char* makeJoke(int i);
char* makeError(char* error);
char* makeError(char* error);void chat(int connfd);


int main(int argc, char* argv[]){
	
	if (argc != 2){
	    printf("Must specify one port\n");
	    exit(0);
    }
	int sockfd, connfd, len;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed\n"); 
		exit(0); 
	}
	else {
		printf("Socket successfully created..\n"); 
    }
	//zero out socket address info struct
	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons((unsigned short)atoi(argv[1])); 

    if ((bind(sockfd, (struct sockaddr )&servaddr, sizeof(servaddr))) != 0) { 
		printf("Socket failed to bind\n"); 
		exit(0); 
	} 
	else {
		printf("Socket successfully binded..\n");
    }
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed\n"); 
		exit(0);
	} 
	else {
		printf("Server listening..\n");
    }
	len = sizeof(servaddr); 
	
    while(1) {
        connfd = accept(sockfd, NULL, &len); 
	    if (connfd < 0) { 
		    printf("Server failed to accept\n"); 
		    exit(0); 
	    } 
	    else {
		    printf("server accept the client...\n");
        }
        chat(connfd);
        close(connfd);
    }
    close(sockfd);
}

int isPunctuation(char c){
    return c == '.' || c == '?' || c == '!';
}

char*  handleMessage(char* message, int i, int length) {
	if (i == 0 && strstr(message, "|") != NULL){
		return "M1FT";
	}
	else if (i == 0 && strlen(message) !=length){
		return "M1LN";
	}
	else if(i == 0 && strcmp(message, "Who's there?") != 0) {
		return "M1CT";
	}

	else if (i == 1 && strstr(message, "|") != NULL){
		return "M3FT";
	}
	else if (i == 1 && strlen(message) !=length){
		return "M3LN";
	}
	else if(i == 1 && strcmp(message, "Dijkstra, who?") != 0) {
		return "M3CT";
	}

	else if (i == 2 && strstr(message, "|") != NULL){
		return "M5FT";
	}
	else if (i == 2 && strlen(message) !=length){
		return "M5LN";
	}
	else if(i == 2 && !isPunctuation(message[length -1]) {
		return "M5CT";
	}
	else {
		return "good";
	}

} 

char* makeJoke(int i){
	char* jokeMessage;
	if (i == 0){
		jokeMessage = "Knock, knock.";
	}else if (i == 1){
		jokeMessage = "Dijkstra.";
	}else if (i == 2){
		jokeMessage = "That path was taking too long, so I let myself in.";
	}
	int jokeLen = strlen(jokeMessage);
	char* formatMessage = malloc(3 + 1+ (floor(log10(jokeLen)) + 1) + 1 +  jokeLen + 1 + 1);
	strcpy(formatMessage, "REG|");
	itoa(jokeLen, formatMessage + 4, 10);
	strcat(formatMessage, "|");
	strcat(formatMessage, jokeMessage);
	strcat(formatMessage, "|");
	//maybe need null terminator for strcat (check for makeError malloc as well)
	return formatMessage;
	}

char* makeError(char* error){
    char* formatError = malloc(10);
    strcpy(formatError,"ERR|");
    strcat(formatError, error);
    strcat(formatError, "|");
    return formatError;
}

void chat(int connfd){
	char header[4];
	header[3] = '\0';

	for(int i =  0; i < 3;++i) {
		//parse header
		char* jokeMessage = makeJoke(i);
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
			printf("Received error message: %s\n", errMsg + 1);
			return;
		}else if (strcmp(header, "REG") == 0)){
			int length = 0
			char digit;
			int gotPipe = 0;
			while (1){
				int got = read(connfd, &digit, 1);
				if (got == 1){
					if (!gotPipe && digit != '|'){
						//format error
						handleFormatError(connfd, i, 0);
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
					}
				} else if (got == 0 || got == -1){
					//connection closed
					return;
				}
			}
			char* message = malloc(length + 1);
			received = 0;
			while (1){
				int status = read(connfd, message + received, length - received); 
				if (status == -1){
					//connection closed
					return;
				}else if (message[received] == '|'){
					break;
				}
				received += status;
			}
			message[received] = '\0';
			char* status = handleMessage(message, i);
			if(strcmp(status, "good") != 0) {
				char* error = makeError(status);
				write(connfd, error, 9);
				free(error);
			}

		}else{
			//first three chars not message type, format error
    		handleErrorFormat(connfd, i,0);

		}
	}
}
