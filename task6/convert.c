#include "convert.h"

#include <limits.h>
#include <stddef.h>

int normalize_base(const int base) {
    if (base < 2 || base > 36) {
        return 10;
    }
    return base;
}

static status_code digit_from_char(const char ch, const int base, const int expect_upper, int *out_digit) {
    int digit;

    if (out_digit == NULL) {
        return STATUS_NULL_PTR;
    }

    if (ch >= '0' && ch <= '9') {
        digit = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
        if (expect_upper) {
            return STATUS_INVALID_INPUT;
        }
        digit = 10 + (ch - 'a');
    } else if (ch >= 'A' && ch <= 'Z') {
        if (!expect_upper) {
            return STATUS_INVALID_INPUT;
        }
        digit = 10 + (ch - 'A');
    } else {
        return STATUS_INVALID_INPUT;
    }

    if (digit >= base) {
        return STATUS_INVALID_INPUT;
    }

    *out_digit = digit;
    return STATUS_OK;
}

status_code base_str_to_int(const char *str, const int base, const int expect_upper, int *out_value) {
    const char *p;
    int sign;
    long long acc;
    int digit;
    int seen;
    status_code st;

    if (str == NULL || out_value == NULL) {
        return STATUS_NULL_PTR;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_RANGE;
    }
    if (str[0] == '\0') {
        return STATUS_INVALID_INPUT;
    }

    p = str;
    sign = 1;
    if (*p == '+' || *p == '-') {
        if (*p == '-') {
            sign = -1;
        }
        ++p;
        if (*p == '\0') {
            return STATUS_INVALID_INPUT;
        }
    }

    acc = 0;
    seen = 0;
    while (*p != '\0') {
        st = digit_from_char(*p, base, expect_upper, &digit);
        if (st != STATUS_OK) {
            return st;
        }
        seen = 1;
        if (sign > 0) {
            if (acc > ((long long)INT_MAX - digit) / base) {
                return STATUS_OVERFLOW;
            }
            acc = acc * base + digit;
        } else {
            if (acc < ((long long)INT_MIN + digit) / base) {
                return STATUS_OVERFLOW;
            }
            acc = acc * base - digit;
        }
        ++p;
    }

    if (!seen) {
        return STATUS_INVALID_INPUT;
    }

    *out_value = (int)acc;
    return STATUS_OK;
}
