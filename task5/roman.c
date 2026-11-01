#include "roman.h"

status_code int_to_roman(const int value, char *buf, const size_t buf_size) {
    static const int values[] = {
        1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1
    };
    static const char *const numerals[] = {
        "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"
    };
    size_t pos;
    int n;
    int i;

    if (buf == NULL || buf_size == 0U) {
        return STATUS_NULL_PTR;
    }

    if (value == 0) {
        return STATUS_INVALID_RANGE;
    }

    n = value;
    pos = 0U;
    buf[0] = '\0';

    if (n < 0) {
        if (buf_size < 2U) {
            return STATUS_OVERFLOW;
        }
        buf[pos++] = '-';
        buf[pos] = '\0';
        if (n == (int)(-2147483647 - 1)) {
            return STATUS_OVERFLOW;
        }
        n = -n;
    }

    if (n < 1 || n > 3999) {
        return STATUS_INVALID_RANGE;
    }

    for (i = 0; i < 13; ++i) {
        while (n >= values[i]) {
            const char *p = numerals[i];
            while (*p != '\0') {
                if (pos + 1U >= buf_size) {
                    return STATUS_OVERFLOW;
                }
                buf[pos++] = *p++;
            }
            n -= values[i];
        }
    }

    buf[pos] = '\0';
    return STATUS_OK;
}
