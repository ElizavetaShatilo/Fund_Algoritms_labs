#include "frac_base.h"

#include <math.h>
#include <stdarg.h>
#include <stddef.h>

static long long gcd_ll(long long a, long long b) {
    if (a < 0) {
        a = -a;
    }
    if (b < 0) {
        b = -b;
    }
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

static status_code ensure_epsilon(double epsilon) {
    if (!isfinite(epsilon) || epsilon <= 0.0 || epsilon >= 1.0) {
        return STATUS_INVALID_EPSILON;
    }
    return STATUS_OK;
}

static int max_decimal_digits(double epsilon) {
    int k = 0;
    double inv = 1.0 / epsilon;
    while (inv >= 10.0 && k < 15) {
        inv /= 10.0;
        ++k;
    }
    k += 3;
    if (k < 1) {
        k = 1;
    }
    if (k > 15) {
        k = 15;
    }
    return k;
}

static int denominator_ok_for_base(long long den, int base) {
    long long b = (long long)base;
    long long common;

    if (den <= 0) {
        return 0;
    }
    while (den > 1) {
        common = gcd_ll(den, b);
        if (common <= 1) {
            return 0;
        }
        while ((den % common) == 0) {
            den /= common;
        }
    }
    return 1;
}

status_code has_finite_base_form(double fraction, int base, double epsilon, bool *out_finite) {
    status_code st;
    int max_k;
    int k;
    long long scale = 1;

    if (out_finite == NULL) {
        return STATUS_NULL_POINTER;
    }

    st = ensure_epsilon(epsilon);
    if (st != STATUS_OK) {
        return st;
    }
    if (base < 2) {
        return STATUS_INVALID_BASE;
    }
    if (!isfinite(fraction) || fraction <= 0.0 || fraction >= 1.0) {
        return STATUS_INVALID_FRACTION;
    }

    max_k = max_decimal_digits(epsilon);
    *out_finite = 0;

    for (k = 0; k <= max_k; ++k) {
        double scaled = fraction * (double)scale;
        double nearest = round(scaled);
        double scaled_err = fabs(scaled - nearest);
        long long num;
        long long den;
        long long g;

        if (scaled_err <= epsilon) {
            num = (long long)nearest;
            den = scale;
            if (num <= 0) {
                return STATUS_OK;
            }
            g = gcd_ll(num, den);
            num /= g;
            den /= g;
            *out_finite = denominator_ok_for_base(den, base) ? 1 : 0;
            return STATUS_OK;
        }

        if (k == max_k) {
            break;
        }
        if (scale > (long long)(9.0e17 / 10.0)) {
            break;
        }
        scale *= 10;
    }

    return STATUS_OK;
}

status_code check_fractions_in_base(double epsilon, int base, bool *results, int count, ...) {
    status_code st;
    va_list args;
    int i;

    if (results == NULL) {
        return STATUS_NULL_POINTER;
    }

    st = ensure_epsilon(epsilon);
    if (st != STATUS_OK) {
        return st;
    }
    if (base < 2) {
        return STATUS_INVALID_BASE;
    }
    if (count <= 0) {
        return STATUS_INVALID_COUNT;
    }

    va_start(args, count);
    for (i = 0; i < count; ++i) {
        double fraction = va_arg(args, double);
        st = has_finite_base_form(fraction, base, epsilon, &results[i]);
        if (st != STATUS_OK) {
            va_end(args);
            return st;
        }
    }
    va_end(args);
    return STATUS_OK;
}
