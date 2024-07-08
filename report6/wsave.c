#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFERSIZE 1024

extern int tcp_connect(char *server, int portno);
extern int fdopen_sock(int sock, FILE **inp, FILE **outp);

void parse_header(FILE *in, int *status_code, int *content_length) {
    char buffer[BUFFERSIZE];
    char *status_line = fgets(buffer, BUFFERSIZE, in);
    sscanf(status_line, "HTTP/%*s %d", status_code);

    char *res;
    while ((res = fgets(buffer, BUFFERSIZE, in)) > 0) {
        if (strncmp(res, "\r\n", 2) == 0) {
            break;
        }
        if (strncmp(res, "Content-Length:", 15) == 0) {
            char *content_length_str = res + 15;
            *content_length = atoi(content_length_str);
        }
    }
}

int read_and_save_body(FILE *in, int content_length, char *dst) {
    FILE *file = fopen(dst, "wb");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    char *buffer = malloc(content_length);
    if (buffer == NULL) {
        perror("malloc");
        fclose(file);
        return 1;
    }

    while (1) {
        int read_num = fread(buffer, sizeof(char), content_length, in);
        if (read_num) {
            fwrite(buffer, sizeof(char), read_num, file);
        } else {
            if (ferror(in)) {
                perror("fread");
                free(buffer);
                fclose(file);
                return 1;
            } else if (feof(in)) {
                break;
            }
        }
    }

    free(buffer);
    fclose(file);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s host port file localfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *server = argv[1];
    int portno = strtol(argv[2], 0, 10);
    char *file = argv[3];
    char *localfile = argv[4];

    int sock = tcp_connect(server, portno);
    if (sock < 0) {
        return EXIT_FAILURE;
    }

    FILE *in, *out;
    if (fdopen_sock(sock, &in, &out) < 0) {
        fprintf(stderr, "fdopen()\n");
        close(sock);
        return EXIT_FAILURE;
    }

    // send
    int res = fprintf(out, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", file, server);
    if (res < 0) {
        perror("fprintf");
        fclose(in);
        fclose(out);
        return EXIT_FAILURE;
    }

    int status_code;
    int content_length = 1024;
    parse_header(in, &status_code, &content_length);

    if (status_code != 200) {
        printf("Status Code is not 200, got %d.\n", status_code);
        char buf[BUFFERSIZE];
        char *result;
        while (1) {
            result = fgets(buf, BUFFERSIZE, in);
            if (!result) {
                break;
            }
        }
        return EXIT_FAILURE;
    }

    if (read_and_save_body(in, content_length, localfile)) {
        fprintf(stderr, "something went wrong.\n");
        return EXIT_FAILURE;
    }

    return 0;
}

#define PORTNO_BUFSIZE 30

int tcp_connect(char *server, int portno) {
    struct addrinfo hints, *ai, *p;
    char portno_str[PORTNO_BUFSIZE];
    int s, err;
    snprintf(portno_str, sizeof(portno_str), "%d", portno);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if ((err = getaddrinfo(server, portno_str, &hints, &ai))) {
        fprintf(stderr, "unknown server %s (%s)\n", server, gai_strerror(err));
        return -1;
    }
    for (p = ai; p; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            perror("socket");
            freeaddrinfo(ai);
            return -1;
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) >= 0) {
            break;
        } else {
            close(s);
        }
    }
    freeaddrinfo(ai);
    return s;
}

int fdopen_sock(int sock, FILE **inp, FILE **outp) {
    int sock2;
    if ((sock2 = dup(sock)) < 0) {
        return -1;
    }
    if ((*inp = fdopen(sock2, "r")) == NULL) {
        close(sock2);
        return -1;
    }
    if ((*outp = fdopen(sock, "w")) == NULL) {
        fclose(*inp);
        *inp = 0;
        return -1;
    }
    setvbuf(*outp, (char *)NULL, _IONBF, 0);
    return 0;
}
