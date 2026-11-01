#include "poly_reexp.h"

#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

status_code reexpand_at_point(double epsilon, double a, double **out_coeffs, int n, ...) {
    if (out_coeffs == NULL) {
        return STATUS_NULL_POINTER;
    }
    *out_coeffs = NULL;

    if (n < 0 || n == INT_MAX) {
        return STATUS_INVALID_DEGREE;
    }
    if (!isfinite(epsilon) || epsilon <= 0.0) {
        return STATUS_INVALID_EPSILON;
    }
    if (!isfinite(a)) {
        return STATUS_INVALID_VALUE;
    }

    double *f = (double *)malloc((size_t)(n + 1) * sizeof(double));
    if (f == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    double *g = (double *)malloc((size_t)(n + 1) * sizeof(double));
    if (g == NULL) {
        free(f);
        return STATUS_MEMORY_ERROR;
    }

    va_list args;
    va_start(args, n);
    for (int i = 0; i <= n; ++i) {
        f[i] = va_arg(args, double);
    }
    va_end(args);

    for (int i = 0; i <= n; ++i) {
        if (!isfinite(f[i])) {
            free(f);
            free(g);
            return STATUS_INVALID_VALUE;
        }
    }

    for (int k = 0; k <= n; ++k) {
        for (int i = n - 1; i >= k; --i) {
            f[i] += a * f[i + 1];
        }
        g[k] = (fabs(f[k]) < epsilon) ? 0.0 : f[k];
    }

    free(f);
    *out_coeffs = g;
    return STATUS_OK;
}
