#include "frac_base.h"
#include "status_code.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void print_error(status_code code) {
    switch (code) {
        case STATUS_INVALID_EPSILON:
            printf("Ошибка: некорректный параметр epsilon.\n");
            break;
        case STATUS_INVALID_BASE:
            printf("Ошибка: некорректное основание системы счисления.\n");
            break;
        case STATUS_INVALID_COUNT:
            printf("Ошибка: некорректное число аргументов.\n");
            break;
        case STATUS_INVALID_FRACTION:
            printf("Ошибка: дробь должна лежать в интервале (0, 1).\n");
            break;
        case STATUS_NULL_POINTER:
            printf("Ошибка: передан нулевой указатель.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память.\n");
            break;
        case STATUS_LIMIT_EXCEEDED:
            printf("Ошибка: превышен вычислительный предел.\n");
            break;
        default:
            printf("Ошибка: неизвестный код состояния.\n");
            break;
    }
}

static void print_batch(
    int base,
    const double *values,
    const bool *results,
    int count
) {
    for (int i = 0; i < count; ++i) {
        printf("  %.12g в основании %d: %s\n",
               values[i],
               base,
               results[i] ? "конечное" : "бесконечное");
    }
}

int main(void) {
    const double epsilon = 1e-6;
    const int count = 4;

    const double f1 = 0.5;
    const double f2 = 0.125;
    const double f3 = 1.0 / 3.0;
    const double f4 = 0.2;

    bool *results = (bool *)malloc((size_t)count * sizeof(bool));
    if (results == NULL) {
        print_error(STATUS_MEMORY_ERROR);
        return 1;
    }

    const double values[] = {f1, f2, f3, f4};

    printf("=== Основание 10 ===\n");
    status_code st = check_fractions_in_base(epsilon, 10, results, count, f1, f2, f3, f4);
    if (st == STATUS_OK) {
        print_batch(10, values, results, count);
    } else {
        print_error(st);
    }

    printf("\n=== Основание 2 ===\n");
    st = check_fractions_in_base(epsilon, 2, results, count, f1, f2, f3, f4);
    if (st == STATUS_OK) {
        print_batch(2, values, results, count);
    } else {
        print_error(st);
    }

    printf("\n=== Ошибка: дробь вне (0, 1) ===\n");
    st = check_fractions_in_base(epsilon, 10, results, 1, 1.5);
    if (st != STATUS_OK) {
        print_error(st);
    }

    free(results);
    return 0;
}
