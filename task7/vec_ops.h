#ifndef VEC_OPS_H
#define VEC_OPS_H

#include "status_code.h"

typedef struct {
    double *components;
    int dimension;
} Vector;

typedef struct {
    double **entries;
    int order;
} SpdMatrix;

typedef status_code (*NormCalculator)(const Vector *vector, const void *parameter, double epsilon, double *out_norm);

status_code vector_alloc(int dimension, const double *components, Vector *out_vector);
void vector_release(Vector *vector);

status_code spd_matrix_alloc(int order, const double *const *rows, double epsilon, SpdMatrix *out_matrix);
void spd_matrix_release(SpdMatrix *matrix);

status_code norm_inf(const Vector *vector, const void *parameter, double epsilon, double *out_norm);
status_code norm_p(const Vector *vector, const void *parameter, double epsilon, double *out_norm);
status_code norm_spd(const Vector *vector, const void *parameter, double epsilon, double *out_norm);

status_code select_longest_by_norms(int dimension, double epsilon, int vector_count, int norm_count, Vector ***out_results, int **out_counts, ...);

#endif
