#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void do_child(int write_pipe, int read_pipe, int count) {

    close(STDIN_FILENO);
    if (dup2(read_pipe, STDIN_FILENO) < 0) {
        perror("dup2 (child)");
        exit(EXIT_FAILURE);
    }
    close(read_pipe);

    for (int i = 0; i < count; i += 2) {
        // print
        putchar('0' + (i / 2) % 10);
        fflush(NULL);

        // send to parent
        int stdout_backup;
        if ((stdout_backup = dup(STDOUT_FILENO)) < 0) {
            perror("dup (child)");
            exit(EXIT_FAILURE);
        }

        close(STDOUT_FILENO);
        if (dup2(write_pipe, STDOUT_FILENO) < 0) {
            perror("dup2 (child)");
            exit(EXIT_FAILURE);
        }

        putchar('!');
        fflush(NULL);

        // wait for parent
        char c = getchar();

        close(STDOUT_FILENO);
        if (dup2(stdout_backup, STDOUT_FILENO) < 0) {
            perror("dup2 (child)");
            exit(EXIT_FAILURE);
        }
        close(stdout_backup);
    }
}

void do_parent(int write_pipe, int read_pipe, int count) {

    close(STDIN_FILENO);
    if (dup2(read_pipe, STDIN_FILENO) < 0) {
        perror("dup2 (parent)");
        exit(EXIT_FAILURE);
    }
    close(read_pipe);

    for (int i = 1; i < count; i += 2) {
        // wait for child
        char c = getchar();

        // print
        putchar('a' + ((i - 1) / 2) % 26);
        fflush(NULL);

        // send to child
        int stdout_backup;
        if ((stdout_backup = dup(STDOUT_FILENO)) < 0) {
            perror("dup (parent)");
            exit(EXIT_FAILURE);
        }

        close(STDOUT_FILENO);
        if (dup2(write_pipe, STDOUT_FILENO) < 0) {
            perror("dup2 (parent)");
            exit(EXIT_FAILURE);
        }

        putchar('!');
        fflush(NULL);

        close(STDOUT_FILENO);
        if (dup2(stdout_backup, STDOUT_FILENO) < 0) {
            perror("dup2 (parent)");
            exit(EXIT_FAILURE);
        }
        close(stdout_backup);
    }

    // wait
    if (wait(NULL) < 0) {
        perror("wait");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int child;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s count\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int count = atoi(argv[1]);

    int pipe_fds1[2]; // parent to child
    int pipe_fds2[2]; // child to parent

    if (pipe(pipe_fds1) < 0 || pipe(pipe_fds2) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((child = fork()) < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (child == 0) {
        // child
        do_child(pipe_fds2[1], pipe_fds1[0], count);
    } else {
        // parent
        do_parent(pipe_fds1[1], pipe_fds2[0], count);
        putchar('\n');
    }
    return EXIT_SUCCESS;
}
