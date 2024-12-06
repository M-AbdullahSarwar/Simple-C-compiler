#include <stdio.h>

double add() {
    return 12.000000;

}

void some() {
    double a = 13.000000;

    printf("This is my name");

    printf("%f\n", 13.000000);

}

double multiply() {
    double a = 3.000000;

    double b = 4.000000;

    printf("Multiplying the values: ");

    printf("%f\n", 3.000000);

    printf("and ");

    printf("%f\n", 4.000000);

    return 12.000000;

}

int main() {
    add();

    double a = 12.000000;

    printf("Result of add(): ");

    printf("%f\n", 12.000000);

    multiply();

    double b = 12.000000;

    printf("Result of multiply(): ");

    printf("%f\n", 12.000000);

    some();

    double c = 7.200000;

    printf("c = 5.9 + 1.3 = ");

    printf("%f\n", 7.200000);

    double d = 16.200000;

    printf("d = c + 9 = ");

    printf("%f\n", 16.200000);

    double e = 20.571429;

    printf("e = c * a / (c - 3) = ");

    printf("%f\n", 20.571429);

    return 0;
}