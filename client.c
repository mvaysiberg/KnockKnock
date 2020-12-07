#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>

void chat(int sockfd) {
	char message[80];
	for(int i =  0; i < 3; ++i) {
		bzero(message, sizeof(message));
		read(sockfd, message, sizeof(message)); 
		printf("Received: %s\n", message);
		if (strncmp(message, "ERR", 3) == 0){
            return;
        }
        /*if(i == 0) {
			write(sockfd, “REG|11|Who’s there|”, 19);
		}*/	
		char input[80];
		scanf("%s", input);
		write(sockfd, input, strlen(input));
	}

}


int main(int argc, char* argv[]){
	if(argc != 2) {
	    printf("Must specify one port\n");
	    exit(0);
    }
	int sockfd, len;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("Socket creation failed\n"); 
		exit(0); 
	}
	else {
		printf("Socket successfully created..\n"); 
    }
    //zero out socket address info struct
	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons((unsigned short)atoi(argv[1])); 
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else {
		printf("connected to the server..\n"); 
    }
    chat(sockfd);
    close(sockfd);
}
