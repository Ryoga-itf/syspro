#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define DATA_FILENAME "file-counter-value.data"

extern void print_header(void);
extern void print_content(int result, int counter);
extern int handle_counter(int *counter);
extern int counter_load(void);
extern void counter_save(int counter);
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

enum {
    SUCCESS,
    PARSE_ERROR,
    INVALID_VALUE,
    TOO_LARGE,
    TOO_SMALL,
};

int main() {
    print_header();
    int counter;
    int res = handle_counter(&counter);
    print_content(res, counter);
}

void print_header() {
    printf("Content-Type: text/html\n");
    printf("\n");
}

void print_content(int result, int counter) {
    printf("<HTML><HEAD></HEAD><BODY>\n");
    switch (result) {
    case SUCCESS:
        printf("%d\n", counter);
        break;
    case PARSE_ERROR:
        printf("Error while parsing query string\n");
        break;
    case INVALID_VALUE:
        printf("Invalid value\n");
        break;
    case TOO_LARGE:
        printf("Too large value\n");
        break;
    case TOO_SMALL:
        printf("Too small value\n");
        break;
    }
    printf("</BODY></HTML>\n");
}

int handle_counter(int *counter) {
    char *query_string = get_query_string();
    *counter = counter_load();
    int result = SUCCESS;

    if (query_string != NULL) {
        int qc;
        char **qv;
        if (string_split(query_string, '&', &qc, &qv) < 0) {
            result = PARSE_ERROR;
        } else {
            char *op = getparam(qc, qv, "op");
            char *val_s = getparam(qc, qv, "val");

            if (op != NULL) {
                if (strcmp(op, "inc") == 0) {
                    (*counter)++;
                    counter_save(*counter);
                } else if (strcmp(op, "set") == 0) {
                    if (val_s == NULL) {
                        result = INVALID_VALUE;
                    } else {
                        char *e;
                        long val = strtol(val_s, &e, 10);
                        if (*e != '\0') {
                            result = INVALID_VALUE;
                        } else {
                            if (val > INT_MAX) {
                                result = TOO_LARGE;
                            } else if (val < INT_MIN) {
                                result = TOO_SMALL;
                            } else {
                                *counter = val;
                                counter_save(*counter);
                            }
                        }
                    }
                }
            }

            free_string_vector(qc, qv);
        }

        free(query_string);
    }

    return result;
}

int counter_load() {
    FILE *f = fopen(DATA_FILENAME, "r");
    if (f == NULL) {
        return 0;
    }

    int counter;
    if (fread(&counter, sizeof(counter), 1, f) != 1) {
        perror("fread");
        fclose(f);
        exit(2);
    }
    fclose(f);
    return counter;
}

void counter_save(int counter) {
    FILE *f = fopen(DATA_FILENAME, "w");
    if (f == NULL) {
        perror(DATA_FILENAME);
        exit(2);
    }
    if (fwrite(&counter, sizeof(counter), 1, f) != 1) {
        perror("fwrite");
        fclose(f);
        exit(2);
    }
    fclose(f);
}

char *get_query_string() {
    char *request_method = getenv("REQUEST_METHOD");
    if (request_method == NULL) {
        return NULL;
    } else if (strcmp(request_method, "GET") == 0) {
        char *query_string = getenv("QUERY_STRING");
        if (query_string == NULL) {
            return NULL;
        } else {
            return strdup(query_string);
        }
    } else if (strcmp(request_method, "POST") == 0) {
        return read_query_string();
    } else {
        printf("Unknown method: ");
        safe_print_string(request_method);
        printf("\n");
        return NULL;
    }
}

char *read_query_string() {
    char *content_length = getenv("CONTENT_LENGTH");
    if (content_length == NULL) {
        return NULL;
    } else {
        int clen = strtol(content_length, 0, 10);
        char *buf = malloc(clen + 1);
        if (buf == 0) {
            printf("read_query_string(): no memory\n");
            exit(-1);
        }
        if (read(0, buf, clen) != clen) {
            printf("read error.\n");
            exit(-1);
        }
        buf[clen] = 0;
        return buf;
    }
}

void safe_printenv(char *name) {
    printf("%s=", name);
    char *val = getenv(name);
    safe_print_string(val);
    printf("\n");
}

void safe_print_string(char *str) {
    if (str == 0) {
        printf("(null)");
        return;
    }
    char *safe_str = html_escape(str);
    if (safe_str == 0) {
        printf("(no memory)");
    } else {
        printf("%s", safe_str);
        free(safe_str);
    }
}

char *html_escape(char *str) {
    const size_t len = strlen(str);
    char *tmp = malloc(len * 6 + 1);
    if (tmp == NULL) {
        return NULL;
    }
    char *p = tmp;
    char c;
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
    char *res = strdup(tmp);
    free(tmp);
    return (res);
}

char *decode_url(char *str) {
    const size_t len = strlen(str);
    char *tmp = malloc(len + 1);
    if (tmp == NULL) {
        return NULL;
    }

    char *p = tmp;
    char c;
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
    char *res = strdup(tmp);
    free(tmp);
    return res;
}

char *getparam(int qc, char *qv[], char *name) {
    const size_t len = strlen(name);
    for (int i = 0; i < qc; i++) {
        if (strncmp(qv[i], name, len) == 0 && qv[i][len] == '=') {
            return (&qv[i][len + 1]);
        }
    }
    return (NULL);
}

int string_split(char *str, char del, int *countp, char ***vecp) {
    if (str == 0) {
        return (-1);
    }
    const int count_max = countchr(str, del) + 1;
    char **vec = malloc(sizeof(char *) * (count_max + 1));
    if (vec == 0) {
        return (-1);
    }

    char *s, *p;
    int i;
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
        size_t len = p - str;
        s = malloc(len + 1);
        if (s == 0) {
            for (int j = 0; j < i; j++) {
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
    return i;
}

void free_string_vector(int qc, char **vec) {
    for (int i = 0; i < qc; i++) {
        if (vec[i] == NULL) {
            break;
        }
        free(vec[i]);
    }
    free(vec);
}

int countchr(char *s, char c) {
    int count;
    for (count = 0; *s; s++) {
        if (*s == c) {
            count++;
        }
    }
    return count;
}
