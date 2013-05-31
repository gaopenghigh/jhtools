#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 

#define BUFLEN 256

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);

    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    while (1) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            error("ERROR opening socket");

        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR connecting");
        printf(">>> ");
        bzero(buffer, BUFLEN);
        fgets(buffer, BUFLEN, stdin);
        if((n = send(sockfd, buffer, strlen(buffer), 0)) <= 0)
             error("ERROR writing to socket");
        bzero(buffer, BUFLEN);
        while ((n = recv(sockfd, buffer, BUFLEN, 0)) > 0) {
            printf("%s",buffer);
        }
    }
    return 0;
}

