#include <stdio.h>
#include <stdlib.h>

#include "parsing.h"
#include "status.h"
#include "strings.h"

static void print_error(const status_code err) {
    switch (err) {
        case STATUS_OK:
            break;
        case STATUS_INVALID_INPUT:
            printf("Ошибка: Некорректный ввод аргументов.\n");
            break;
        case STATUS_NULL_PTR:
            printf("Ошибка: Передан нулевой указатель.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: Не удалось выделить память.\n");
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: Произошло переполнение.\n");
            break;
        case STATUS_UNKNOWN_FLAG:
            printf("Ошибка: Неизвестный флаг (допустимы -l, -r, -u, -n, -c).\n");
            break;
        default:
            printf("Неизвестная ошибка.\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    char flag;
    status_code st;
    char *result = NULL;

    if (argc < 3) {
        printf("Использование: %s <флаг -l|-r|-u|-n|-c> <строка> [seed строки...]\n",
               (argc > 0 && argv[0] != NULL) ? argv[0] : "program");
        print_error(STATUS_INVALID_INPUT);
        return STATUS_INVALID_INPUT;
    }

    st = parse_flag(argv[1], &flag);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    if (flag != 'c' && argc != 3) {
        print_error(STATUS_INVALID_INPUT);
        return STATUS_INVALID_INPUT;
    }
    if (flag == 'c' && argc < 4) {
        print_error(STATUS_INVALID_INPUT);
        return STATUS_INVALID_INPUT;
    }

    if (flag == 'l') {
        size_t length = 0;
        st = string_length(argv[2], &length);
        if (st != STATUS_OK) {
            print_error(st);
            return st;
        }
        printf("Длина строки: %lu\n", (unsigned long)length);
        return STATUS_OK;
    }

    switch (flag) {
        case 'r':
            st = string_reverse(argv[2], &result);
            break;
        case 'u':
            st = string_upper_odd(argv[2], &result);
            break;
        case 'n':
            st = string_group_chars(argv[2], &result);
            break;
        case 'c': {
            unsigned int seed = 0;
            st = parse_seed(argv[3], &seed);
            if (st == STATUS_OK) {
                st = string_concat_random(
                    argv[2],
                    (const char *const *)&argv[4],
                    (size_t)(argc - 4),
                    seed,
                    &result);
            }
            break;
        }
        default:
            st = STATUS_UNKNOWN_FLAG;
            break;
    }

    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    printf("Результат: %s\n", result);
    free(result);
    return STATUS_OK;
}
