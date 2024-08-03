#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

extern void print_header(void);
extern void print_content(void);
extern char *get_query_string();
extern char *read_query_string();
extern void safe_printenv(char *name);
extern void safe_print_string(char *str);
extern char *html_escape(char *str);
extern char *decode_url(char *str);
extern char *getparam(int qc, char *qv[], char *name);

extern int string_split(char *str, char del, int *countp, char ***vecp);
extern void free_string_vector(int qc, char **vec);
extern int countchr(char *s, char c);

int main() {
    print_header();
    print_content();
}

void print_header() {
    printf("Content-Type: text/html\n");
    printf("\n");
}

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

void print_content() {
    char *query_string;
    int qc;
    char **qv;

    printf("<HTML><HEAD></HEAD><BODY><PRE>\n");
    query_string = get_query_string();
    if (query_string == NULL) {
        printf("No query string.\n");
        // print
    } else if (string_split(query_string, '&', &qc, &qv) < 0) {
        printf("Error while parsing query string\n");
    } else {
        int valid = 1;
        long year, month;
        char *year_s = getparam(qc, qv, "year");
        if (year_s == NULL) {
            printf("parameter 'year' is required.\n");
            valid = 0;
        } else {
            char *e;
            year = strtol(year_s, &e, 10);
            if (*e != '\0') {
                printf("Invalid value.\n");
                valid = 0;
            } else {
                if (year <= 0 || year >= 10000) {
                    printf("Invalid value.\n");
                    valid = 0;
                }
            }
        }

        char *month_s = getparam(qc, qv, "month");
        if (month_s == NULL) {
            printf("parameter 'month' is required.\n");
            valid = 0;
        } else {
            char *e;
            month = strtol(month_s, &e, 10);
            if (*e != '\0') {
                printf("Invalid value.\n");
                valid = 0;
            } else {
                if (month <= 0 || month > 12) {
                    printf("Invalid value.\n");
                    valid = 0;
                }
            }
        }

        if (valid) {
            int child, status;
            if ((child = fork()) < 0) {
                perror("fork");
                exit(-1);
            }

            if (child == 0) {
                // child
                char month_buffer[32], year_buffer[32];
                snprintf(month_buffer, 32, "%ld", month);
                snprintf(year_buffer, 32, "%ld", year);
                char *argv[4];
                argv[0] = "/home/linuxbrew/.linuxbrew/bin/cal";
                argv[1] = month_buffer;
                argv[2] = year_buffer;
                argv[3] = NULL;
                if (execve("/usr/bin/cal", argv, NULL) < 0) {
                    perror("execve");
                    exit(-1);
                }
            } else {
                // parent
                if (wait(&status) < 0) {
                    perror("wait");
                    exit(-1);
                }
            }
        }

        free_string_vector(qc, qv);
    }

    printf("</PRE></BODY></HTML>\n");
    if (query_string) {
        free(query_string);
    }
}

char *get_query_string() {
    char *request_method, *query_string;
    request_method = getenv("REQUEST_METHOD");
    if (request_method == 0) {
        return (0);
    } else if (strcmp(request_method, "GET") == 0) {
        query_string = getenv("QUERY_STRING");
        if (query_string == 0) {
            return (0);
        } else {
            return (strdup(query_string));
        }
    } else if (strcmp(request_method, "POST") == 0) {
        return (read_query_string());
    } else {
        printf("Unknown method: ");
        safe_print_string(request_method);
        printf("\n");
        return (0);
    }
}

char *read_query_string() {
    int clen;
    char *content_length;
    char *buf;

    content_length = getenv("CONTENT_LENGTH");
    if (content_length == 0) {
        return (0);
    } else {
        clen = strtol(content_length, 0, 10);
        buf = malloc(clen + 1);
        if (buf == 0) {
            printf("read_query_string(): no memory\n");
            exit(-1);
        }
        if (read(0, buf, clen) != clen) {
            printf("read error.\n");
            exit(-1);
        }
        buf[clen] = 0;
        return (buf);
    }
}

void safe_printenv(char *name) {
    char *val;
    char *safe_val;

    printf("%s=", name);
    val = getenv(name);
    safe_print_string(val);
    printf("\n");
}

void safe_print_string(char *str) {
    char *safe_str;

    if (str == 0) {
        printf("(null)");
        return;
    }
    safe_str = html_escape(str);
    if (safe_str == 0) {
        printf("(no memory)");
    } else {
        printf("%s", safe_str);
        free(safe_str);
    }
}

char *html_escape(char *str) {
    int len;
    char c, *tmp, *p, *res;

    len = strlen(str);
    tmp = malloc(len * 6 + 1);
    if (tmp == 0) {
        return (0);
    }
    p = tmp;
    while ((c = *str++)) {
        switch (c) {
        case '&':
            memcpy(p, "&amp;", 5);
            p += 5;
            break;
        case '<':
            memcpy(p, "&lt;", 4);
            p += 4;
            break;
        case '>':
            memcpy(p, "&gt;", 4);
            p += 4;
            break;
        case '"':
            memcpy(p, "&quot;", 6);
            p += 6;
            break;
        default:
            *p = c;
            p++;
            break;
        }
    }
    *p = 0;
    res = strdup(tmp);
    free(tmp);
    return (res);
}

char *decode_url(char *str) {
    int len;
    char c, *tmp, *p, *res;

    len = strlen(str);
    tmp = malloc(len + 1);
    if (tmp == 0) {
        return (0);
    }
    p = tmp;

    while (*str) {
        if (*str == '%' && isxdigit(*(str + 1)) && isxdigit(*(str + 2))) {
            char hexstr[3];
            hexstr[0] = *(str + 1);
            hexstr[1] = *(str + 2);
            hexstr[2] = 0;
            c = strtol(hexstr, 0, 16);
            *p++ = c;
            str += 3;
        } else if (*str == '+') {
            *p++ = ' ';
            str++;
        } else {
            *p++ = *str;
            str++;
        }
    }
    *p = 0;
    res = strdup(tmp);
    free(tmp);
    return (res);
}

char *getparam(int qc, char *qv[], char *name) {
    int i;
    size_t len;

    len = strlen(name);
    for (i = 0; i < qc; i++) {
        if (strncmp(qv[i], name, len) == 0 && qv[i][len] == '=') {
            return (&qv[i][len + 1]);
        }
    }
    return (NULL);
}

int string_split(char *str, char del, int *countp, char ***vecp) {
    char **vec;
    int count_max, i, len;
    char *s, *p;

    if (str == 0) {
        return (-1);
    }
    count_max = countchr(str, del) + 1;
    vec = malloc(sizeof(char *) * (count_max + 1));
    if (vec == 0) {
        return (-1);
    }

    for (i = 0; i < count_max; i++) {
        while (*str == del) {
            str++;
        }
        if (*str == 0) {
            break;
        }
        for (p = str; *p != del && *p != 0; p++) {
            continue;
        }
        /* *p == del || *p=='\0' */
        len = p - str;
        s = malloc(len + 1);
        if (s == 0) {
            int j;
            for (j = 0; j < i; j++) {
                free(vec[j]);
                vec[j] = 0;
            }
            return (-1);
        }
        memcpy(s, str, len);
        s[len] = 0;
        vec[i] = s;
        str = p;
    }
    vec[i] = 0;
    *countp = i;
    *vecp = vec;
    return (i);
}

void free_string_vector(int qc, char **vec) {
    int i;
    for (i = 0; i < qc; i++) {
        if (vec[i] == NULL) {
            break;
        }
        free(vec[i]);
    }
    free(vec);
}

int countchr(char *s, char c) {
    int count;
    for (count = 0; *s; s++)
        if (*s == c) {
            count++;
        }
    return (count);
}
