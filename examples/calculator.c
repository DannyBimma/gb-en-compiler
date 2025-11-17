int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int main() {
    int x = 10;
    int y = 5;
    int sum = add(x, y);
    int diff = subtract(x, y);
    int prod = multiply(x, y);
    return 0;
}
