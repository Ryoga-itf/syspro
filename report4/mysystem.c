#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int mysystem(const char *command) {
    int child, status;

    if ((child = fork()) < 0) {
        perror("fork");
        return -1;
    }

    if (child == 0) {
        // child
        if (execl("/bin/sh", "sh", "-c", command, (char *)NULL) < 0) {
            perror("execl");
            return -1;
        }
    } else {
        // parent
        if (wait(&status) < 0) {
            perror("wait");
            return -1;
        }
    }
    return 0;
}

int main(void) {
    puts("TEST1:");
    mysystem("ls -1");

    puts("TEST2:");
    mysystem("ls -1 | cat -n");

    puts("TEST3:");
    mysystem("ls -1 | cat -n | wc");

    puts("TEST4:");
    mysystem("ls /usr/include/**/*.h | cat -n | wc > /tmp/syspro.txt");

    return 0;
}
