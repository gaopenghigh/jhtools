/* A simple server to run system commands use poll */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
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
void serve(struct pollfd *pfd) {
    int n;
    char buffer[BUFLEN];
    FILE *fp;

    bzero(buffer, BUFLEN);
    printf("in serve ,fd=%d\n", pfd->fd);
    if (pfd->revents & POLLIN) {                /* read */
        if ((n = read(pfd->fd, buffer, BUFLEN)) < 0)
            printf("ERROR reading from socket : %d", n);
        printf("CMD : %s\n",buffer);
        if ((fp = popen(buffer, "r")) == NULL)
            error("ERROR when popen");
        while (fgets(buffer, BUFLEN, fp) != NULL) {
            if (send(pfd->fd, buffer, BUFLEN, 0) == -1)
                error("send ERROR");
        }
        printf("serve end, closing %d\n", pfd->fd);
        close(pfd->fd);
        pfd->fd = -1;
        pclose(fp);
    }
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

void printfd(struct pollfd array[], int n) {
    int i;
    printf("array = ");
    for(i = 0; i < n; i++)
        printf("[%d]:%d ", i, array[i].fd);
    printf("\n");
}

/*
 * Use poll() to serve for every connection
 */
int main(int argc, char *argv[]) {
    int endserver = FALSE;
    int listen_sock, conn_sock, pos, i, j;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    /* INIT pollfd structures */
    struct pollfd allfds[MAXCONN];
    printf("sizeof(allfds)=%d\n", (int)sizeof(allfds));
    memset(allfds, 0, sizeof(allfds));
    int nfds = 0;    /* number of fds in allfds */
    int currentsize; /* current size of allfds */
    int poll_result;

    /* init listen socket, bind, listen */
    listen_sock = init_server();
    setnonblocking(listen_sock);
    listen(listen_sock, BACKLOG);

    /* add listen_socket to allfds array */
    allfds[0].fd = listen_sock;
    allfds[0].events = POLLIN;
    nfds++;
    

    while (endserver == FALSE) {
        printfd(allfds, nfds);
        printf("nfds=%d\n", nfds);
        /* wait for events on sockets, timeout = -1 means waite forever */
        poll_result = poll(allfds, nfds, -1);
        if (poll_result == -1) {
            error("poll ERROR");
            break;
        } else if (poll_result == 0) {
            printf("poll round timeout, enther another poll...\n");
            continue;
        }
        /*******************************************************************/
        /* One or more descriptors are readable                            */
        /*******************************************************************/
        currentsize = nfds;
        printf("correntsize=%d\n", currentsize);
        for (pos = 0; pos < currentsize; pos++) {
            if (allfds[pos].revents & POLLIN &&
                    allfds[pos].fd == listen_sock) {  /* listen socket */ 
                printf("event on listen sock\n");
                /* Accept all incomming connections */
                do {
                    printf("listen_sock=%d\n", listen_sock);
                    conn_sock = accept(listen_sock, 
                                   (struct sockaddr *)&cli_addr, &clilen);
                    if (conn_sock < 0)
                        continue;
                    setnonblocking(conn_sock);
                    if (nfds >= MAXCONN) {
                        error("nfds >= MAXCONN");
                        break;
                    }
                    allfds[nfds].fd = conn_sock;
                    allfds[nfds].events = POLLIN;
                    nfds++;
                } while(conn_sock != -1);

            /* regular socket */
            } else {                                       
                serve(&allfds[pos]);
            }

            /* comparess allfds, delete the items which fd = -1 */
            for (i = 0; i < nfds; i++) {
                if (allfds[i].fd == -1) {
                    for (j = i; j < nfds; j++) {
                        allfds[j] = allfds[j+1];
                    }
                    nfds--;
                    i--;
                }
            }
        }

    }

    /* end server */
    for (i = 0; i < nfds; i++) {
        if (allfds[i].fd >= 0)
            close(allfds[i].fd);
    }

    return 0;

}
