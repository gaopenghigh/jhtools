/* A simple server to run system commands use poll */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORTNO 4444
#define BACKLOG 10
#define BUFLEN 256
#define MAXCONN 100
#define TRUE 1
#define FALSE 0

void error(const char *msg) {
    perror(msg);
}

/* 
 * function really do the service
 * run cmd geted from client and return the output to client
 */
void serve(int fd) {
    int n;
    char buffer[BUFLEN];
    FILE *fp;

    bzero(buffer, BUFLEN);
    printf("in serve ,fd=%d\n", fd);
    if ((n = read(fd, buffer, BUFLEN)) < 0)
        printf("ERROR reading from socket : %d", n);
    printf("CMD : %s\n",buffer);
    if ((fp = popen(buffer, "r")) == NULL)
        error("ERROR when popen");
    while (fgets(buffer, BUFLEN, fp) != NULL) {
        if (send(fd, buffer, BUFLEN, 0) == -1)
            error("send ERROR");
    }
    printf("serve end, closing %d\n", fd);
    pclose(fp);
}

/*
 * Init listen socket and bind it to addr, return the listen socket
 */
int init_server() {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORTNO);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
        exit(1);
    }

    return sockfd;
}

/*
 * set fd nonblocking, success return 0, fail return -1
 */
int setnonblocking(int fd) {
    if (fd > 0) {
        int flags = fcntl(fd, F_GETFL, 0);
        flags = flags|O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) == 0) {
            printf("setnonblocking success!\n");
            return 0;
        }
    }
    return -1;
}


/*
 * Use poll() to serve for every connection
 */
int main(int argc, char *argv[]) {
    int endserver = FALSE;
    int listen_sock, conn_sock, n;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    /* INIT pollfd structures */
    struct epoll_event ev, events[MAXCONN];
    int nfds;
    int epollfd = epoll_create(10);
    if (epollfd == -1) {
        printf("epoll_create error\n");
        exit(1);
    }

    /* init listen socket, bind, listen */
    listen_sock = init_server();
    setnonblocking(listen_sock);
    listen(listen_sock, BACKLOG);

    /* register listen_sock to epollfd */
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        printf("ERROR: epoll_ctl\n");
        exit(1);
    }

    while (endserver == FALSE) {
        nfds = epoll_wait(epollfd, events, MAXCONN, -1);
        if (nfds == -1) {
            printf("epoll_wait ERROR\n");
            exit(1);
        }

        for (n = 0; n < nfds; n++) {
            if (events[n].data.fd == listen_sock) { /* listen socket */
                do {
                    printf("listen_sock=%d\n", listen_sock);
                    conn_sock = accept(listen_sock, 
                                   (struct sockaddr *)&cli_addr, &clilen);
                    if (conn_sock < 0)
                        continue;
                    setnonblocking(conn_sock);
                    ev.events = EPOLLIN|EPOLLET;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev)
                            == -1) {
                        printf("epoll_ctl for conn_sock ERROR\n");
                        exit(1);
                    }
                } while(conn_sock != -1);
            } else { /* regular connection */
                serve(events[n].data.fd);
                ev.events = EPOLLIN|EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &ev)
                        == -1) {
                    printf("epoll_ctl DEL ERROR\n");
                    exit(1);
                }
                close(events[n].data.fd);
            }
        }
    }

    return 0;
}
