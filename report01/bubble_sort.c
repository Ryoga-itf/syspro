#include <stdio.h>

#define SAMPLE_COUNT 24
int sample[SAMPLE_COUNT] = {
    3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4,
};

int g_swap_count = 0;

void print_data(int a[], int count) {
    for (int i = 0; i < count; i++) {
        printf("%2d%c", a[i], " \n"[i == count - 1]);
    }
}

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
        printf("%d: ", i);
        print_data(data, count);
        for (int j = 0; j < n - i; j++) {
            if (data[j] > data[j + 1]) {
                swap_array(data, j, j + 1);
            }
        }
    }
}

int main(void) {
    sort(sample, SAMPLE_COUNT);
    print_data(sample, SAMPLE_COUNT);
    printf("swap count: %d\n", g_swap_count);
    return 0;
}
