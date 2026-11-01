#include "vec_ops.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

status_code vector_alloc(int dimension, const double *components, Vector *out_vector) {
    if (dimension <= 0 || components == NULL || out_vector == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }

    double *buffer = (double *)malloc((size_t)dimension * sizeof(double));
    if (buffer == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (int i = 0; i < dimension; ++i) {
        buffer[i] = components[i];
    }

    out_vector->components = buffer;
    out_vector->dimension = dimension;
    return STATUS_OK;
}

void vector_release(Vector *vector) {
    if (vector == NULL) {
        return;
    }
    free(vector->components);
    vector->components = NULL;
    vector->dimension = 0;
}

static double principal_minor_det(
    const double *const *rows,
    int size,
    double epsilon,
    double *workspace
) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            workspace[i * size + j] = rows[i][j];
        }
    }

    double det = 1.0;
    for (int col = 0; col < size; ++col) {
        int pivot = col;
        for (int row = col + 1; row < size; ++row) {
            if (fabs(workspace[row * size + col]) > fabs(workspace[pivot * size + col])) {
                pivot = row;
            }
        }

        if (fabs(workspace[pivot * size + col]) <= epsilon) {
            return 0.0;
        }

        if (pivot != col) {
            for (int j = 0; j < size; ++j) {
                double tmp = workspace[col * size + j];
                workspace[col * size + j] = workspace[pivot * size + j];
                workspace[pivot * size + j] = tmp;
            }
            det = -det;
        }

        det *= workspace[col * size + col];
        for (int row = col + 1; row < size; ++row) {
            double factor = workspace[row * size + col] / workspace[col * size + col];
            for (int j = col; j < size; ++j) {
                workspace[row * size + j] -= factor * workspace[col * size + j];
            }
        }
    }
    return det;
}

static status_code verify_spd(const double *const *rows, int order, double epsilon) {
    for (int i = 0; i < order; ++i) {
        for (int j = i + 1; j < order; ++j) {
            if (fabs(rows[i][j] - rows[j][i]) > epsilon) {
                return STATUS_MATRIX_NOT_SPD;
            }
        }
    }

    double *workspace = (double *)malloc((size_t)order * (size_t)order * sizeof(double));
    if (workspace == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (int k = 1; k <= order; ++k) {
        if (principal_minor_det(rows, k, epsilon, workspace) <= epsilon) {
            free(workspace);
            return STATUS_MATRIX_NOT_SPD;
        }
    }

    free(workspace);
    return STATUS_OK;
}

status_code spd_matrix_alloc(int order, const double *const *rows, double epsilon, SpdMatrix *out_matrix) {
    if (order <= 0 || rows == NULL || out_matrix == NULL || epsilon <= 0.0) {
        return STATUS_INVALID_ARGUMENT;
    }

    for (int i = 0; i < order; ++i) {
        if (rows[i] == NULL) {
            return STATUS_INVALID_ARGUMENT;
        }
    }

    status_code st = verify_spd(rows, order, epsilon);
    if (st != STATUS_OK) {
        return st;
    }

    double **copy = (double **)calloc((size_t)order, sizeof(double *));
    if (copy == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (int i = 0; i < order; ++i) {
        copy[i] = (double *)malloc((size_t)order * sizeof(double));
        if (copy[i] == NULL) {
            for (int j = 0; j < i; ++j) {
                free(copy[j]);
            }
            free(copy);
            return STATUS_MEMORY_ERROR;
        }
        for (int j = 0; j < order; ++j) {
            copy[i][j] = rows[i][j];
        }
    }

    out_matrix->entries = copy;
    out_matrix->order = order;
    return STATUS_OK;
}

void spd_matrix_release(SpdMatrix *matrix) {
    if (matrix == NULL) {
        return;
    }
    if (matrix->entries != NULL) {
        for (int i = 0; i < matrix->order; ++i) {
            free(matrix->entries[i]);
        }
        free(matrix->entries);
    }
    matrix->entries = NULL;
    matrix->order = 0;
}

status_code norm_inf(const Vector *vector, const void *parameter, double epsilon, double *out_norm) {
    (void)parameter;
    if (vector == NULL || vector->components == NULL || vector->dimension <= 0 ||
        out_norm == NULL || epsilon <= 0.0) {
        return STATUS_INVALID_ARGUMENT;
    }

    double max_abs = 0.0;
    for (int i = 0; i < vector->dimension; ++i) {
        double value = fabs(vector->components[i]);
        if (value > max_abs) {
            max_abs = value;
        }
    }
    *out_norm = max_abs;
    return STATUS_OK;
}

status_code norm_p(const Vector *vector, const void *parameter, double epsilon, double *out_norm) {
    if (vector == NULL || vector->components == NULL || vector->dimension <= 0 ||
        parameter == NULL || out_norm == NULL || epsilon <= 0.0) {
        return STATUS_INVALID_ARGUMENT;
    }

    double p = *(const double *)parameter;
    if (p < 1.0 - epsilon) {
        return STATUS_INVALID_ARGUMENT;
    }

    double sum = 0.0;
    for (int i = 0; i < vector->dimension; ++i) {
        sum += pow(fabs(vector->components[i]), p);
    }
    *out_norm = pow(sum, 1.0 / p);
    return STATUS_OK;
}

status_code norm_spd(const Vector *vector, const void *parameter, double epsilon, double *out_norm) {
    const SpdMatrix *matrix = (const SpdMatrix *)parameter;
    if (vector == NULL || vector->components == NULL || vector->dimension <= 0 ||
        matrix == NULL || matrix->entries == NULL || out_norm == NULL ||
        epsilon <= 0.0 || vector->dimension != matrix->order) {
        return STATUS_INVALID_ARGUMENT;
    }

    double quadratic = 0.0;
    for (int i = 0; i < matrix->order; ++i) {
        double row_dot = 0.0;
        for (int j = 0; j < matrix->order; ++j) {
            row_dot += matrix->entries[i][j] * vector->components[j];
        }
        quadratic += row_dot * vector->components[i];
    }

    if (quadratic < -epsilon) {
        return STATUS_MATRIX_NOT_SPD;
    }
    if (quadratic < 0.0) {
        quadratic = 0.0;
    }

    *out_norm = sqrt(quadratic);
    return STATUS_OK;
}

static void discard_partial(Vector **groups, int *counts, int filled) {
    if (groups != NULL) {
        for (int i = 0; i < filled; ++i) {
            free(groups[i]);
        }
    }
    free(groups);
    free(counts);
}

static status_code longest_for_norms(int dimension, double epsilon, int vector_count, const Vector *vectors, int norm_count, const NormCalculator *calculators, const void *const *parameters, Vector ***out_results, int **out_counts) {
    for (int i = 0; i < vector_count; ++i) {
        if (vectors[i].dimension != dimension || vectors[i].components == NULL) {
            return STATUS_INVALID_DIMENSION;
        }
    }

    Vector **groups = (Vector **)calloc((size_t)norm_count, sizeof(Vector *));
    int *counts = (int *)calloc((size_t)norm_count, sizeof(int));
    double *values = (double *)malloc((size_t)vector_count * sizeof(double));
    if (groups == NULL || counts == NULL || values == NULL) {
        free(values);
        discard_partial(groups, counts, 0);
        return STATUS_MEMORY_ERROR;
    }

    for (int n = 0; n < norm_count; ++n) {
        for (int v = 0; v < vector_count; ++v) {
            status_code st = calculators[n](&vectors[v], parameters[n], epsilon, &values[v]);
            if (st != STATUS_OK) {
                free(values);
                discard_partial(groups, counts, n);
                return st;
            }
        }

        double longest = values[0];
        for (int v = 1; v < vector_count; ++v) {
            if (values[v] > longest) {
                longest = values[v];
            }
        }

        int match_count = 0;
        for (int v = 0; v < vector_count; ++v) {
            if (fabs(values[v] - longest) <= epsilon) {
                ++match_count;
            }
        }

        Vector *matched = (Vector *)malloc((size_t)match_count * sizeof(Vector));
        if (matched == NULL) {
            free(values);
            discard_partial(groups, counts, n);
            return STATUS_MEMORY_ERROR;
        }

        int write = 0;
        for (int v = 0; v < vector_count; ++v) {
            if (fabs(values[v] - longest) <= epsilon) {
                matched[write++] = vectors[v];
            }
        }

        groups[n] = matched;
        counts[n] = match_count;
    }

    free(values);
    *out_results = groups;
    *out_counts = counts;
    return STATUS_OK;
}

status_code select_longest_by_norms(int dimension, double epsilon, int vector_count, int norm_count, Vector ***out_results, int **out_counts, ...) {
    if (dimension <= 0 || epsilon <= 0.0 || vector_count <= 0 || norm_count <= 0 ||
        out_results == NULL || out_counts == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }

    Vector *vectors = (Vector *)malloc((size_t)vector_count * sizeof(Vector));
    NormCalculator *calculators =
        (NormCalculator *)malloc((size_t)norm_count * sizeof(NormCalculator));
    const void **parameters =
        (const void **)malloc((size_t)norm_count * sizeof(const void *));

    if (vectors == NULL || calculators == NULL || parameters == NULL) {
        free(vectors);
        free(calculators);
        free(parameters);
        return STATUS_MEMORY_ERROR;
    }

    va_list args;
    va_start(args, out_counts);

    for (int i = 0; i < vector_count; ++i) {
        vectors[i] = va_arg(args, Vector);
    }
    for (int i = 0; i < norm_count; ++i) {
        calculators[i] = va_arg(args, NormCalculator);
        parameters[i] = va_arg(args, const void *);
        if (calculators[i] == NULL) {
            va_end(args);
            free(vectors);
            free(calculators);
            free(parameters);
            return STATUS_NULL_POINTER;
        }
    }
    va_end(args);

    status_code st = longest_for_norms(
        dimension,
        epsilon,
        vector_count,
        vectors,
        norm_count,
        calculators,
        parameters,
        out_results,
        out_counts
    );

    free(vectors);
    free(calculators);
    free(parameters);
    return st;
}
