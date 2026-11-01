#include "status_code.h"
#include "vec_ops.h"

#include <stdio.h>
#include <stdlib.h>

static void print_error(status_code code) {
    switch (code) {
        case STATUS_NULL_POINTER:
            printf("Ошибка: передан нулевой указатель.\n");
            break;
        case STATUS_INVALID_ARGUMENT:
            printf("Ошибка: некорректный аргумент.\n");
            break;
        case STATUS_INVALID_DIMENSION:
            printf("Ошибка: неверная размерность вектора.\n");
            break;
        case STATUS_MEMORY_ERROR:
            printf("Ошибка: не удалось выделить память.\n");
            break;
        case STATUS_MATRIX_NOT_SPD:
            printf("Ошибка: матрица не является симметричной положительно определённой.\n");
            break;
        default:
            printf("Ошибка: неизвестный код состояния.\n");
            break;
    }
}

static void print_vector(const Vector *vector) {
    printf("  [");
    for (int i = 0; i < vector->dimension; ++i) {
        printf("%g", vector->components[i]);
        if (i + 1 < vector->dimension) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main(void) {
    const int dimension = 2;
    const double epsilon = 1e-9;

    const double raw1[] = {3.0, 4.0};
    const double raw2[] = {-5.0, 0.0};
    const double raw3[] = {3.0, 4.0};

    Vector v1 = {NULL, 0};
    Vector v2 = {NULL, 0};
    Vector v3 = {NULL, 0};
    SpdMatrix matrix = {NULL, 0};
    int exit_code = 1;

    status_code st = vector_alloc(dimension, raw1, &v1);
    if (st == STATUS_OK) {
        st = vector_alloc(dimension, raw2, &v2);
    }
    if (st == STATUS_OK) {
        st = vector_alloc(dimension, raw3, &v3);
    }

    if (st == STATUS_OK) {
        double row0[] = {2.0, 0.0};
        double row1[] = {0.0, 2.0};
        const double *rows[] = {row0, row1};
        st = spd_matrix_alloc(2, rows, epsilon, &matrix);
    }

    if (st == STATUS_OK) {
        const double p = 2.0;
        Vector **longest = NULL;
        int *counts = NULL;

        st = select_longest_by_norms(
            dimension,
            epsilon,
            3,
            3,
            &longest,
            &counts,
            v1, v2, v3,
            norm_inf, (const void *)NULL,
            norm_p, (const void *)&p,
            norm_spd, (const void *)&matrix
        );

        if (st == STATUS_OK) {
            const char *titles[] = {
                "норма бесконечности",
                "p-норма (p = 2)",
                "A-норма"
            };
            printf("Расчёт выполнен успешно.\n");
            for (int i = 0; i < 3; ++i) {
                printf("Норма «%s»: найдено %d самых длинных вектора(ов):\n",
                       titles[i], counts[i]);
                for (int j = 0; j < counts[i]; ++j) {
                    print_vector(&longest[i][j]);
                }
                free(longest[i]);
            }
            free(longest);
            free(counts);
            exit_code = 0;
        }
    }

    if (st != STATUS_OK) {
        print_error(st);
    }

    spd_matrix_release(&matrix);
    vector_release(&v1);
    vector_release(&v2);
    vector_release(&v3);
    return exit_code;
}
