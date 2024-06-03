#include <math.h>
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

void insertionsort(int data[], int left, int right) {
    print_data(sample, SAMPLE_COUNT);
    for (int i = left + 1; i < right; i++) {
        int value = data[i];
        int j = i;
        while (j > left && data[j - 1] > value) {
            data[j] = data[j - 1];
            --j;
        }
        data[j] = value;
    }
}

void heapify(int data[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = left + 1;

    if (left < n && data[left] > data[largest]) {
        largest = left;
    }

    if (right < n && data[right] > data[largest]) {
        largest = right;
    }

    if (largest != i) {
        swap_array(data, i, largest);
        heapify(data, n, largest);
    }
}

void heapsort(int data[], int left, int right) {
    print_data(sample, SAMPLE_COUNT);
    int size = right + left;
    // make heap
    for (int p = size / 2 - 1; p >= 0; p--) {
        heapify(data + left, size, p);
    }
    // sort heap
    for (int i = size - 1; i >= 0; i--) {
        swap_array(data, left, i);
        heapify(data + left, i, 0);
    }
}

int partition(int data[], int left, int right) {
    --right;
    int pivot = data[right];
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (data[j] <= pivot) {
            ++i;
            swap_array(data, i, j);
        }
    }
    swap_array(data, i + 1, right);
    return i + 1;
}

void introsort(int data[], int left, int right, int depth) {
    print_data(sample, SAMPLE_COUNT);
    if (right - left < 16) {
        // do insertionsort
        printf("do insertionsort:\n");
        insertionsort(data, left, right);
    } else if (depth == 0) {
        // do heapsort
        printf("do heapsort:\n");
        heapsort(data, left, right);
    } else {
        // do quicksort
        printf("do quicksort:\n");
        int pivot = partition(data, left, right);
        introsort(data, left, pivot, depth - 1);
        introsort(data, pivot + 1, right, depth - 1);
    }
}

void sort(int data[], int count) {
    int maxdepth = log2(count) * 2;
    introsort(data, 0, count, maxdepth);
}

int main(void) {
    sort(sample, SAMPLE_COUNT);
    print_data(sample, SAMPLE_COUNT);
    printf("swap count: %d\n", g_swap_count);
    return 0;
}
