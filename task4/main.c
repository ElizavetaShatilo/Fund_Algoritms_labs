#include <stdio.h>

#include "knapsack.h"
#include "parsing.h"
#include "status.h"

static void print_error(const status_code err) {
    switch (err) {
        case STATUS_OK:
            break;
        case STATUS_INVALID_INPUT:
            printf("Ошибка: Некорректный ввод.\n");
            break;
        case STATUS_NULL_PTR:
            printf("Ошибка: Передан нулевой указатель.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: Не удалось выделить память.\n");
            break;
        case STATUS_OVERFLOW:
            printf("Ошибка: Переполнение или слишком длинная строка.\n");
            break;
        case STATUS_FILE_ERROR:
            printf("Ошибка: Проблема при чтении ввода.\n");
            break;
        default:
            printf("Неизвестная ошибка.\n");
            break;
    }
}

int main(void) {
    backup_file *files = NULL;
    int n = 0;
    int capacity = 0;
    backup_selection result;
    status_code st;
    int i;

    result.names = NULL;
    result.count = 0;
    result.total_size = 0;
    result.total_importance = 0;

    st = read_backup_input(stdin, &files, &n, &capacity);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    st = select_backup_files(files, n, capacity, &result);
    if (st != STATUS_OK) {
        print_error(st);
        free_backup_files(files, n);
        return st;
    }

    printf("TOTAL_SIZE %d\n", result.total_size);
    printf("TOTAL_IMPORTANCE %lld\n", result.total_importance);
    printf("FILES %d\n", result.count);
    for (i = 0; i < result.count; ++i) {
        printf("%s\n", result.names[i]);
    }

    free_backup_selection(&result);
    free_backup_files(files, n);
    return STATUS_OK;
}
