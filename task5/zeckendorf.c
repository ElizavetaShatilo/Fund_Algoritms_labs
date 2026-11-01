#include "zeckendorf.h"

#define ZECK_FIB_MAX 48

status_code uint_to_zeckendorf(const unsigned int value, char *buf, const size_t buf_size) {
    unsigned int fib[ZECK_FIB_MAX];
    int used[ZECK_FIB_MAX];
    int count;
    int highest;
    int i;
    unsigned int rem;
    size_t pos;

    if (buf == NULL || buf_size == 0U) {
        return STATUS_NULL_PTR;
    }

    if (value == 0U) {
        if (buf_size < 2U) {
            return STATUS_OVERFLOW;
        }
        buf[0] = '1';
        buf[1] = '\0';
        return STATUS_OK;
    }

    fib[0] = 1U;
    fib[1] = 2U;
    count = 2;
    while (count < ZECK_FIB_MAX) {
        unsigned int next;
        if (fib[count - 1] > 0xFFFFFFFFU - fib[count - 2]) {
            break;
        }
        next = fib[count - 1] + fib[count - 2];
        if (next > value) {
            break;
        }
        fib[count] = next;
        ++count;
    }

    for (i = 0; i < count; ++i) {
        used[i] = 0;
    }

    rem = value;
    for (i = count - 1; i >= 0; --i) {
        if (fib[i] <= rem) {
            used[i] = 1;
            rem -= fib[i];
            if (i > 0) {
                --i;
            }
        }
    }

    if (rem != 0U) {
        return STATUS_OVERFLOW;
    }

    highest = 0;
    for (i = 0; i < count; ++i) {
        if (used[i]) {
            highest = i;
        }
    }

    if (buf_size < (size_t)highest + 3U) {
        return STATUS_OVERFLOW;
    }

    pos = 0U;
    for (i = 0; i <= highest; ++i) {
        buf[pos++] = (char)(used[i] ? '1' : '0');
    }
    buf[pos++] = '1';
    buf[pos] = '\0';
    return STATUS_OK;
}
