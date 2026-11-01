#include "zeckendorf.h"

#include <stddef.h>

status_code zeckendorf_to_uint(const char *str, unsigned int *out_value) {
    size_t len;
    size_t coeffs;
    size_t i;
    unsigned int fib[64];
    int count;
    unsigned int sum;

    if (str == NULL || out_value == NULL) {
        return STATUS_NULL_PTR;
    }

    len = 0U;
    while (str[len] != '\0') {
        if (str[len] != '0' && str[len] != '1') {
            return STATUS_INVALID_INPUT;
        }
        ++len;
    }

    if (len == 0U || str[len - 1U] != '1') {
        return STATUS_INVALID_INPUT;
    }

    if (len == 1U) {
        *out_value = 0U;
        return STATUS_OK;
    }

    coeffs = len - 1U;
    fib[0] = 1U;
    fib[1] = 2U;
    count = 2;
    while ((size_t)count < coeffs && count < 64) {
        if (fib[count - 1] > 0xFFFFFFFFU - fib[count - 2]) {
            return STATUS_OVERFLOW;
        }
        fib[count] = fib[count - 1] + fib[count - 2];
        ++count;
    }
    if ((size_t)count < coeffs) {
        return STATUS_OVERFLOW;
    }

    sum = 0U;
    for (i = 0U; i < coeffs; ++i) {
        if (str[i] == '1') {
            if (sum > 0xFFFFFFFFU - fib[i]) {
                return STATUS_OVERFLOW;
            }
            sum += fib[i];
        }
    }

    *out_value = sum;
    return STATUS_OK;
}
