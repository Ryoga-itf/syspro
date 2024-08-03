#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int file_fd[2];
    const int target_fd[] = {0, 1};
    const int flags[] = {O_RDONLY, O_WRONLY};

    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 2; i++) {
        file_fd[i] = open(argv[i + 1], flags[i]);
        if (file_fd[i] < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        close(target_fd[i]);
        if (dup2(file_fd[i], target_fd[i]) < 0) {
            perror("dup2");
            close(file_fd[i]);
            exit(EXIT_FAILURE);
        }
        close(file_fd[i]);
    }

    execlp("wc", "wc", NULL);

    return EXIT_FAILURE;
}
