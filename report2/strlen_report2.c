#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned long int ul;

size_t my_strlen1(const char *s) {
    size_t result = 0;
    for (const char *ptr = s; *ptr; ptr++) {
        result++;
    }
    return result;
}

#define hi 0x8080808080808080
#define lo 0x0101010101010101

size_t my_strlen2(const char *s) {
    // not 8 bytes aligned part
    register const char *ptr_char = s;
    while (((ul)ptr_char & 0x07) != 0) {
        if (*ptr_char == '\0') {
            return ptr_char - s;
        }
        ++ptr_char;
    }

    for (register const ul *ptr = (const ul *)ptr_char;; ++ptr) {
        register ul longword = *ptr;
        register ul masked = (longword - lo) & ~longword & hi;
        if (masked != 0) {
            int index = __builtin_ctzl(masked) >> 3;
            return (size_t)ptr - (size_t)s + index;
        }
    }
}

int main() {
    const int N = 30000000;
    char *str = malloc(N + 1);
    for (int i = 0; i < N; i++) {
        str[i] = (char)('A' + rand() % 26);
    }
    str[N] = '\0';

    const int T = 100;
    printf("N = %d, T = %d\n\n", N, T);

    {
        double takes = 0.0;
        clock_t start = clock();
        for (int i = 0; i < T; i++) {
            size_t result = my_strlen1(str);
        }
        clock_t end = clock();
        double elapsed = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;

        printf("my_strlen1\n");
        printf("  takes  : %.4fms\n", elapsed);
        printf("  average: %.4fms\n", elapsed / T);
        printf("-------------------\n");
    }
    {
        double takes = 0.0;
        clock_t start = clock();
        for (int i = 0; i < T; i++) {
            size_t result = my_strlen2(str);
        }
        clock_t end = clock();
        double elapsed = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;

        printf("my_strlen2\n");
        printf("  takes  : %.4fms\n", elapsed);
        printf("  average: %.4fms\n", elapsed / T);
        printf("-------------------\n");
    }
    {
        double takes = 0.0;
        clock_t start = clock();
        for (int i = 0; i < T; i++) {
            size_t result = strlen(str);
        }
        clock_t end = clock();
        double elapsed = ((double)(end - start) / CLOCKS_PER_SEC) * 1000;

        printf("len\n");
        printf("  takes  : %.4fms\n", elapsed);
        printf("  average: %.4fms\n", elapsed / T);
        printf("-------------------\n");
    }

    free(str);
    return 0;
}
