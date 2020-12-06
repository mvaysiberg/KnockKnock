#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <ctype.h>
#include <math.h>

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
