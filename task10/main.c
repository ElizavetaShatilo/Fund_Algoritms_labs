#include "poly_reexp.h"
#include "status_code.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void print_error(status_code code) {
    switch (code) {
        case STATUS_NULL_POINTER:
            printf("Ошибка: передан нулевой указатель.\n");
            break;
        case STATUS_INVALID_DEGREE:
            printf("Ошибка: некорректная степень многочлена.\n");
            break;
        case STATUS_INVALID_EPSILON:
            printf("Ошибка: некорректное значение epsilon.\n");
            break;
        case STATUS_INVALID_VALUE:
            printf("Ошибка: недопустимое значение (NaN или бесконечность).\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память.\n");
            break;
        default:
            printf("Ошибка: неизвестный код состояния.\n");
            break;
    }
}

static void show_report(
    const char *title,
    status_code st,
    const double *coeffs,
    int n
) {
    printf("=== %s ===\n", title);
    if (st == STATUS_OK && coeffs != NULL) {
        for (int i = 0; i <= n; ++i) {
            printf("g[%d] = %g\n", i, coeffs[i]);
        }
    } else {
        print_error(st);
    }
    printf("\n");
}

int main(void) {
    const double epsilon = 1e-9;
    double *result = NULL;
    status_code st;

    st = reexpand_at_point(epsilon, 2.0, &result, 3, 1.0, -3.0, 3.0, -1.0);
    show_report("1 - 3x + 3x^2 - x^3, a = 2 (ожидание -1, -3, -3, -1)", st, result, 3);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 5.0, &result, 0, 7.0);
    show_report("константа 7, a = 5 (ожидание 7)", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 1.0, &result, 2, 1.0, 2.0, 1.0);
    show_report("1 + 2x + x^2, a = 1 (ожидание 4, 4, 1)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 1.0, &result, 2, 1.0, -2.0, 1.0);
    show_report("1 - 2x + x^2, a = 1 (ожидание 0, 0, 1)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 0.0, &result, 2, 1.0, 2.0, 3.0);
    show_report("1 + 2x + 3x^2, a = 0 (ожидание 1, 2, 3)", st, result, 2);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, -1.5, &result, 4, 0.0, 0.0, 0.0, 0.0, 2.0);
    show_report("2x^4, a = -1.5", st, result, 4);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 2.0, NULL, 2, 1.0, 2.0, 3.0);
    show_report("ошибка: NULL out_coeffs", st, NULL, 0);

    st = reexpand_at_point(epsilon, 2.0, &result, -1);
    show_report("ошибка: отрицательная степень", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 2.0, &result, INT_MAX);
    show_report("ошибка: степень INT_MAX", st, result, 0);
    free(result);
    result = NULL;

    st = reexpand_at_point(0.0, 2.0, &result, 1, 1.0, 2.0);
    show_report("ошибка: epsilon = 0", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_at_point(-1e-9, 2.0, &result, 1, 1.0, 2.0);
    show_report("ошибка: epsilon < 0", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, NAN, &result, 1, 1.0, 2.0);
    show_report("ошибка: a = NaN", st, result, 1);
    free(result);
    result = NULL;

    st = reexpand_at_point(epsilon, 2.0, &result, 2, 1.0, INFINITY, 3.0);
    show_report("ошибка: коэффициент = infinity", st, result, 2);
    free(result);
    result = NULL;

    return 0;
}
