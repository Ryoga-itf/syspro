#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <line_counts>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat fs;
    if (fstat(fd, &fs) < 0) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // The size of a mapped area must be a multiple of pagesize
    // ceil(x / m) * m = (x + (m - 1)) / m * m
    size_t maplen =
        (fs.st_size + _SC_PAGE_SIZE - 1) / _SC_PAGE_SIZE * _SC_PAGE_SIZE;

    char *contents = mmap(NULL, maplen, PROT_READ, MAP_PRIVATE, fd, 0);
    if (contents == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int lines = 0;
    int max_lines = atoi(argv[2]);
    for (char *ptr = contents; *ptr && lines < max_lines; ++ptr) {
        char c = *ptr;
        putchar(c);
        if (c == '\n') {
            lines++;
        }
    }

    if (munmap(contents, maplen)) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return 0;
}
