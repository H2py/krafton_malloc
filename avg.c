#include <stdio.h>

float compute_average(int *arr, int size) {
    int sum = 0;

    for (int i = 0; i < size; i++) {  
        sum += arr[i];
    }

    return sum / size;
}

int main() {
    int scores[] = {80, 90, 70, 60};
    int size = sizeof(scores) / sizeof(scores[0]);

    float avg = compute_average(scores, size);
    printf("Average score: %.2f\n", avg);

    return 0;
}
