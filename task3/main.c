#include <stdio.h>

#include "search.h"
#include "status.h"

static void print_escaped(const char *s) {
    size_t i;
    for (i = 0; s[i] != '\0'; ++i) {
        if (s[i] == '\n') {
            printf("\\n");
        } else if (s[i] == '\t') {
            printf("\\t");
        } else if (s[i] == ' ') {
            printf("·");
        } else {
            putchar(s[i]);
        }
    }
}

static void print_state(status_code st) {
    switch (st) {
        case STATUS_OK:
            break;
        case STATUS_NULL_PTR:
            printf("  Ошибка: передан нулевой указатель.\n");
            break;
        case STATUS_INVALID_INPUT:
            printf("  Ошибка: некорректный аргумент (в т.ч. пустая подстрока).\n");
            break;
        case STATUS_FILE_ERROR:
            printf("  Ошибка открытия или чтения файла.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("  Ошибка выделения памяти.\n");
            break;
        default:
            printf("  Неизвестная ошибка.\n");
            break;
    }
}

static void print_results(const char *substring,
                          const file_search_result *results,
                          size_t file_count) {
    size_t i;
    size_t j;

    printf("Подстрока: \"");
    print_escaped(substring);
    printf("\"\n");

    for (i = 0; i < file_count; ++i) {
        const char *path = (results[i].path != NULL) ? results[i].path : "(нет пути)";
        printf("Файл: %s\n", path);

        if (results[i].state != STATUS_OK) {
            print_state(results[i].state);
            continue;
        }
        if (results[i].match_count == 0) {
            printf("  Совпадений не найдено.\n");
        } else {
            for (j = 0; j < results[i].match_count; ++j) {
                printf("  Строка: %lu, Символ: %lu\n",
                       (unsigned long)results[i].matches[j].line,
                       (unsigned long)results[i].matches[j].column);
            }
        }
    }
    printf("\n");
}

int main(void) {
    file_search_result *results = NULL;

    search_in_files("ab", &results, 3, "start_end.txt", "overlap.txt", "missing.txt");
    print_results("ab", results, 3);
    free_file_search_results(results, 3);

    search_in_files("aa", &results, 2, "overlap.txt", "one_char.txt");
    print_results("aa", results, 2);
    free_file_search_results(results, 2);

    search_in_files("b\t\tc\n\n d", &results, 4,
                    "spaces.txt", "start_end.txt", "empty.txt", "newlines.txt");
    print_results("b\t\tc\n\n d", results, 4);
    free_file_search_results(results, 4);

    search_in_files("\n\n", &results, 1, "newlines.txt");
    print_results("\n\n", results, 1);
    free_file_search_results(results, 1);

    search_in_files("", &results, 2, "newlines.txt", "spaces.txt");
    print_results("", results, 2);
    free_file_search_results(results, 2);

    return 0;
}
