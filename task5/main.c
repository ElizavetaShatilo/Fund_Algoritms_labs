#include <stdio.h>
#include <string.h>

#include "overprint.h"
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
            printf("Ошибка: Переполнение буфера или числа.\n");
            break;
        case STATUS_INVALID_RANGE:
            printf("Ошибка: Значение вне допустимого диапазона.\n");
            break;
        case STATUS_IO_ERROR:
            printf("Ошибка: Сбой записи в поток.\n");
            break;
        case STATUS_UNSUPPORTED_FORMAT:
            printf("Ошибка: Неподдерживаемый формат.\n");
            break;
        default:
            printf("Неизвестная ошибка.\n");
            break;
    }
}

int main(void) {
    char buf[1024];
    int written = 0;
    status_code st;
    FILE *fp;
    const int roman_demo = 2024;
    const unsigned int zeck_demo = 11U;
    const int base_num = 255;
    const float fdemo = 1.0f;
    const double ddemo = 1.0;

    printf("=== oversprintf: демо ===\n");

    st = oversprintf(buf, &written, "Roman %d = %Ro\n", roman_demo, roman_demo);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "Zeckendorf %u = %Zr\n", zeck_demo, zeck_demo);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "255 base16 lower = %Cv\n", base_num, 16);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "255 base16 upper = %CV\n", base_num, 16);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "ff base16 -> dec = %to\n", "ff", 16);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "FF base16 -> dec = %TO\n", "FF", 16);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "42 invalid base 99 -> %Cv\n", 42, 99);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "int dump %%mi: %mi\n", 1);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "uint dump %%mu: %mu\n", 1U);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "float dump %%mf: %mf\n", fdemo);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "double dump %%md: %md\n", ddemo);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    st = oversprintf(buf, &written, "mix: %s %d %% %Ro\n", "ok", 7, 12);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }
    printf("%s", buf);

    printf("\n=== overfprintf: stdout ===\n");
    st = overfprintf(stdout, &written,
                     "XIV=%Ro Zr(11)=%Zr hex=%Cv\n",
                     14, 11U, 255, 16);
    if (st != STATUS_OK) {
        print_error(st);
        return st;
    }

    fp = fopen("task5_demo_out.txt", "w");
    if (fp == NULL) {
        print_error(STATUS_IO_ERROR);
        return STATUS_IO_ERROR;
    }
    st = overfprintf(fp, &written, "file: %Ro %Zr %to\n", 9, 3U, "1010", 2);
    if (st != STATUS_OK) {
        print_error(st);
        fclose(fp);
        return st;
    }
    if (fclose(fp) != 0) {
        print_error(STATUS_IO_ERROR);
        return STATUS_IO_ERROR;
    }
    printf("Также записано в task5_demo_out.txt (%d байт формата).\n", written);

    st = oversprintf(buf, &written, "%Ro", 0);
    if (st == STATUS_OK) {
        printf("Ошибка: ожидался STATUS_INVALID_RANGE для %%Ro(0).\n");
        return STATUS_INVALID_INPUT;
    }
    printf("Ожидаемо для %%Ro(0): ");
    print_error(st);

    return STATUS_OK;
}
