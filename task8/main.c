#include "radix_add.h"
#include "status_code.h"

#include <stdio.h>
#include <stdlib.h>

static void print_error(status_code code) {
    switch (code) {
        case STATUS_INVALID_ARGUMENT:
            printf("Ошибка: некорректные аргументы.\n");
            break;
        case STATUS_INVALID_BASE:
            printf("Ошибка: основание должно быть в диапазоне [2..36].\n");
            break;
        case STATUS_INVALID_DIGIT:
            printf("Ошибка: недопустимая цифра для заданного основания.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память.\n");
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: переполнение размера.\n");
            break;
        default:
            printf("Ошибка: неизвестный код состояния.\n");
            break;
    }
}

static void show_case(const char *title, status_code st, char *sum) {
    printf("%s\n", title);
    if (st == STATUS_OK) {
        printf("  сумма = %s\n", sum);
        free(sum);
    } else {
        printf("  ");
        print_error(st);
    }
}

int main(void) {
    char *sum = NULL;
    status_code st;

    st = sum_in_base(
        10, 3, &sum,
        "12300000000000000000000000000000000000000",
        "00000000000000000000000456",
        "789"
    );
    show_case("base 10: 123... + 456 + 789", st, sum);

    st = sum_in_base(16, 2, &sum, "f", "1");
    show_case("base 16: f + 1", st, sum);

    st = sum_in_base(16, 2, &sum, "ff", "FF");
    show_case("base 16: ff + FF", st, sum);

    st = sum_in_base(2, 3, &sum, "1", "1", "1");
    show_case("base 2: 1 + 1 + 1", st, sum);

    st = sum_in_base(10, 2, &sum, "000", "0");
    show_case("base 10: 000 + 0", st, sum);

    st = sum_in_base(10, 3, &sum, "0007", "0003", "00");
    show_case("base 10: 0007 + 0003 + 00", st, sum);

    st = sum_in_base(36, 2, &sum, "ZZZZ", "1");
    show_case("base 36: ZZZZ + 1", st, sum);

    st = sum_in_base(10, 1, &sum, "42");
    show_case("base 10: одно число 42", st, sum);

    st = sum_in_base(
        10, 2, &sum,
        "99999999999999999999999999999999999999999999999999",
        "1"
    );
    show_case("base 10: 50 девяток + 1", st, sum);

    st = sum_in_base(10, 2, &sum, "12a", "5");
    show_case("ошибка: недопустимая цифра", st, sum);

    st = sum_in_base(2, 2, &sum, "101", "102");
    show_case("ошибка: цифра 2 в двоичной", st, sum);

    st = sum_in_base(1, 2, &sum, "1", "1");
    show_case("ошибка: основание 1", st, sum);

    st = sum_in_base(37, 2, &sum, "1", "1");
    show_case("ошибка: основание 37", st, sum);

    st = sum_in_base(10, 0, &sum, "1");
    show_case("ошибка: count = 0", st, sum);

    st = sum_in_base(10, 2, &sum, "", "1");
    show_case("ошибка: пустая строка", st, sum);

    st = sum_in_base(10, 2, &sum, NULL, "1");
    show_case("ошибка: NULL", st, sum);

    return 0;
}
