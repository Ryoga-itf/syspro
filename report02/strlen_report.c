#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

size_t my_strlen1(const char *s) {
    size_t result = 0;
    for (const char *ptr = s; *ptr; ptr++) {
        result++;
    }
    return result;
}

size_t my_strlen2(const char *s) {
    register const char *ptr = s;
    while (*ptr++)
        ;
    return ptr - s;
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
