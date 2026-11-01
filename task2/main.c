#include <stdio.h>
#include <string.h>

#include "employees.h"
#include "parsing.h"
#include "status.h"

#define SALARY_EPS 1e-9

static void print_error(const status_code err) {
    switch (err) {
        case STATUS_OK:
            break;
        case STATUS_INVALID_INPUT:
            printf("Ошибка: Некорректное количество или значение аргументов.\n");
            break;
        case STATUS_NULL_PTR:
            printf("Ошибка: Передан нулевой указатель.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: Не удалось выделить память.\n");
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: Переполнение или слишком длинное значение.\n");
            break;
        case STATUS_UNKNOWN_FLAG:
            printf("Ошибка: Неизвестный флаг (допустимы -a, /a, -d, /d).\n");
            break;
        case STATUS_FILE_ERROR:
            printf("Ошибка: Не удалось открыть, прочитать или записать файл.\n");
            break;
        case STATUS_INVALID_DATA:
            printf("Ошибка: Неверный формат данных во входном файле.\n");
            break;
        case STATUS_EMPTY_FILE:
            printf("Ошибка: Входной файл пуст.\n");
            break;
        default:
            printf("Неизвестная ошибка.\n");
            break;
    }
}

static int same_path(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    return strcmp(a, b) == 0;
}

int main(int argc, char *argv[]) {
    char flag;
    status_code st;
    employee *employees = NULL;
    size_t count = 0;
    size_t written;

    if (argc != 4) {
        printf("Использование: %s <входной_файл> <флаг -a|-d|/a|/d> <выходной_файл>\n",
               (argc > 0 && argv[0] != NULL) ? argv[0] : "program");
        print_error(STATUS_INVALID_INPUT);
        return STATUS_INVALID_INPUT;
    }

    st = parse_flag(argv[2], &flag);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    if (same_path(argv[1], argv[3])) {
        printf("Входной и выходной файлы не должны совпадать.\n");
        print_error(STATUS_INVALID_INPUT);
        return STATUS_INVALID_INPUT;
    }

    st = read_employees(argv[1], &employees, &count);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    st = sort_employees(employees, count, flag, SALARY_EPS);
    if (st != STATUS_OK) {
        free_employees(&employees, &count);
        print_error(st);
        return st;
    }

    st = write_employees(argv[3], employees, count);
    written = count;
    free_employees(&employees, &count);

    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    printf("Готово. Записано записей: %lu\n", (unsigned long)written);
    return STATUS_OK;
}
