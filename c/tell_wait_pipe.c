/* tell_wait_pipe.c
 * Use PIPE to sync between child process and parent process
 * From APUE
 * Create 2 PIPEs, fd1[2], fd2[2]
 * fd1 : parent --> child, write 'p' if parent is OK
 * fd2 : child --> parent, write 'c' if child is OK
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define ERRORCODE 1

static int fd1[2], fd2[2];

void err(char *s) {
    printf("ERROR: %s\n", s);
    exit(ERRORCODE);
}

void TELL_WAIT(void) {
    if (pipe(fd1) < 0 || pipe(fd2) < 0)
        err("pipe failed");
}

void TELL_PARENT(pid_t pid) {
    if (write(fd2[1], "c", 1) != 1)
        err("write error");
}

void WAIT_PARENT(pid_t pid) {
    char c;
    if (read(fd1[0], &c, 1) != 1)
        err("read error");
    if (c != 'p')
        err("WAIT_PARENT: incorrect data");
}

void TELL_CHILD(pid_t pid) {
    if (write(fd1[1], "p", 1) != 1)
        err("write error");
}

void WAIT_CHILD(pid_t pid) {
    char c;
    if (read(fd2[0], &c, 1) != 1)
        err("read error");
    if (c != 'c')
        err("WAIT_CHILD: incorrect data");
}

