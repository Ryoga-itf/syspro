#include <stdio.h>
#include <stdlib.h>

int g_swap_count = 0;

void swap_array(int a[], int i, int j) {
    int tmp;
    tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
    ++g_swap_count;
}

void sort(int data[], int count) {
    int n = count - 1;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - i; j++) {
            if (data[j] > data[j + 1]) {
                swap_array(data, j, j + 1);
            }
        }
    }
}

int main(void) {
    int count = 50000;
    int *data = (int *)malloc(sizeof(int) * count);
    srand(0);
    for (int i = 0; i < count; i++) {
        data[i] = rand();
    }
    sort(data, count);
    printf("swap count: %d\n", g_swap_count);
    free(data);
    return 0;
}
