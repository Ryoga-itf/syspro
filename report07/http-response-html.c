#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void http_send_reply(FILE *out, char *filename);
extern void http_send_reply_bad_request(FILE *out);
extern void http_send_reply_not_found(FILE *out);

#define BUFFERSIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];
    http_send_reply(stdout, filename);
    return 0;
}

void http_send_reply(FILE *out, char *filename) {
    char *ext = strrchr(filename, '.');
    if (ext == NULL) {
        http_send_reply_bad_request(out);
        return;
    } else if (strcmp(ext, ".html") == 0) {
        printf("filename is [%s], and extention is [%s].\n", filename, ext);

        char buffer[BUFFERSIZE];
        int req;
        req = snprintf(buffer, BUFFERSIZE, "./%s", filename);
        if (req >= BUFFERSIZE) {
            fprintf(stderr, "Out of buffer_size");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen(buffer, "r");
        if (file == NULL) {
            http_send_reply_not_found(out);
            return;
        }

        fprintf(out, "HTTP/1.0 200 OK\r\n");
        fprintf(out, "Content-Type: text/html\r\n");
        fprintf(out, "\r\n");

        while (!feof(file)) {
            int read_num = fread(buffer, sizeof(char), BUFFERSIZE, file);
            if (ferror(file)) {
                perror("fread");
                fclose(file);
                exit(EXIT_FAILURE);
            }

            int write_num = fwrite(buffer, 1, read_num, out);
            if (ferror(out)) {
                perror("fwrite");
                exit(EXIT_FAILURE);
            }
        }

        fclose(file);
        return;
    } else {
        http_send_reply_bad_request(out);
        return;
    }
}

void http_send_reply_bad_request(FILE *out) {
    fprintf(out, "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n");
    fprintf(out, "<html><head></head><body>400 Bad Request</body></html>\n");
}

void http_send_reply_not_found(FILE *out) {
    fprintf(out, "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n");
    fprintf(out, "<html><head></head><body>404 Not Found</body></html>\n");
}
