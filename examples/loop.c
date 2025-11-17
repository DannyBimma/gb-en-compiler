int sum_array(int arr[], int size) {
    int total = 0;
    for (int i = 0; i < size; i++) {
        total = total + arr[i];
    }
    return total;
}

int main() {
    int numbers[5];
    int sum = sum_array(numbers, 5);
    return 0;
}
