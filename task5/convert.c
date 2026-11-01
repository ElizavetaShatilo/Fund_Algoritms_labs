#include "convert.h"

#include <limits.h>

int normalize_base(const int base) {
    if (base < 2 || base > 36) {
        return 10;
    }
    return base;
}

static status_code digit_from_char(const char ch, const int base, const int expect_upper, int *out_digit
) {
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

    if (digit < 0 || digit >= base) {
        return STATUS_INVALID_INPUT;
    }

    *out_digit = digit;
    return STATUS_OK;
}

static char digit_to_char(const int digit, const int uppercase) {
    if (digit < 10) {
        return (char)('0' + digit);
    }
    if (uppercase) {
        return (char)('A' + (digit - 10));
    }
    return (char)('a' + (digit - 10));
}

status_code int_to_base_str(const int value, const int base, const int uppercase, char *buf, const size_t buf_size) {
    char tmp[66];
    size_t tpos;
    size_t i;
    int b;
    unsigned int u;
    int negative;

    if (buf == NULL || buf_size == 0U) {
        return STATUS_NULL_PTR;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_RANGE;
    }

    b = base;
    negative = 0;
    if (value < 0) {
        negative = 1;
        u = (unsigned int)(-(value + 1)) + 1U;
    } else {
        u = (unsigned int)value;
    }

    tpos = 0U;
    do {
        tmp[tpos++] = digit_to_char((int)(u % (unsigned int)b), uppercase);
        u /= (unsigned int)b;
    } while (u != 0U);

    if (negative) {
        tmp[tpos++] = '-';
    }

    if (tpos + 1U > buf_size) {
        return STATUS_OVERFLOW;
    }

    for (i = 0U; i < tpos; ++i) {
        buf[i] = tmp[tpos - 1U - i];
    }
    buf[tpos] = '\0';
    return STATUS_OK;
}

status_code base_str_to_int(const char *str, const int base, const int expect_upper, int *out_value) {
    const char *p;
    int sign;
    int b;
    long long acc;
    status_code st;
    int digit;
    int seen;

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

    b = base;
    acc = 0;
    seen = 0;
    while (*p != '\0') {
        st = digit_from_char(*p, b, expect_upper, &digit);
        if (st != STATUS_OK) {
            return st;
        }
        seen = 1;

        if (sign > 0) {
            if (acc > ((long long)INT_MAX - digit) / b) {
                return STATUS_OVERFLOW;
            }
            acc = acc * b + digit;
        } else {
            if (acc < ((long long)INT_MIN + digit) / b) {
                return STATUS_OVERFLOW;
            }
            acc = acc * b - digit;
        }
        ++p;
    }

    if (!seen) {
        return STATUS_INVALID_INPUT;
    }

    *out_value = (int)acc;
    return STATUS_OK;
}

status_code memory_dump_bin(const void *data, const size_t nbytes, char *buf, const size_t buf_size) {
    const unsigned char *bytes;
    size_t need;
    size_t pos;
    size_t i;
    int bit;

    if (data == NULL || buf == NULL) {
        return STATUS_NULL_PTR;
    }
    if (nbytes == 0U) {
        return STATUS_INVALID_INPUT;
    }

    need = nbytes * 8U + (nbytes - 1U) + 1U;
    if (buf_size < need) {
        return STATUS_OVERFLOW;
    }

    bytes = (const unsigned char *)data;
    pos = 0U;
    for (i = 0U; i < nbytes; ++i) {
        if (i > 0U) {
            buf[pos++] = ' ';
        }
        for (bit = 7; bit >= 0; --bit) {
            buf[pos++] = (char)(((bytes[i] >> bit) & 1U) ? '1' : '0');
        }
    }
    buf[pos] = '\0';
    return STATUS_OK;
}
