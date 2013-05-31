/* A simple server to run system commands use multi threads */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORTNO 4444
#define BACKLOG 10
#define BUFLEN 256

void error(const char *msg) {
    perror(msg);
    exit(1);
}

/* 
 * function really do the service in sub thread
 * run cmd geted from client and return the output to client
 */
void *serve(void *arg) {
    int sockfd = *(unsigned int *)arg;
    int n;
    char buffer[BUFLEN];
    FILE *fp;

    printf("start serve...\n");
    bzero(buffer, BUFLEN);
    if ((n = read(sockfd, buffer, BUFLEN)) < 0)
        error("ERROR reading from socket");
    printf("3start serve...\n");
    printf("CMD : %s\n",buffer);

    if ((fp = popen(buffer, "r")) == NULL)
        error("ERROR when popen");
    while (fgets(buffer, BUFLEN, fp) != NULL) {
        if (send(sockfd, buffer, BUFLEN, 0) == -1)
            error("send ERROR");
    }
    pclose(fp);
    close(sockfd);

    printf("end serve...\n");
    return(0);
}

/*
 * Init listen socket and bind it to addr
 */
int init_server() {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORTNO);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    return sockfd;
}
/*
 * Use a sub threads to serve for every connection
 */
int main(int argc, char *argv[]) {
    signal(SIGCHLD,SIG_IGN);

    int sockfd = init_server();
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    listen(sockfd, BACKLOG);

    while (1) {
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0)
            error("ERROR on accept");
        printf("newsockfd=%d\n", newsockfd);
        pthread_t t;
        int fd_in_thread = newsockfd;
        if(pthread_create(&t, NULL, serve, (void *)&fd_in_thread) != 0)
            error("ERROR on pthread_create");
    }
}
