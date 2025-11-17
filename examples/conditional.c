void check_age(int age) {
    if (age >= 18) {
        printf("You are an adult.\n");
    } else if (age >= 13) {
        printf("You are a teenager.\n");
    } else {
        printf("You are a child.\n");
    }
}

int main() {
    int my_age = 25;
    check_age(my_age);
    return 0;
}
