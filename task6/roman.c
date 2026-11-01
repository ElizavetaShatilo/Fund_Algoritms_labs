#include "roman.h"

#include <stddef.h>

static int roman_value(const char ch) {
    switch (ch) {
        case 'I':
            return 1;
        case 'V':
            return 5;
        case 'X':
            return 10;
        case 'L':
            return 50;
        case 'C':
            return 100;
        case 'D':
            return 500;
        case 'M':
            return 1000;
        default:
            return -1;
    }
}

status_code roman_to_int(const char *str, int *out_value) {
    int total;
    int prev;
    const char *p;

    if (str == NULL || out_value == NULL) {
        return STATUS_NULL_PTR;
    }
    if (str[0] == '\0') {
        return STATUS_INVALID_INPUT;
    }

    total = 0;
    prev = 0;
    p = str;
    while (*p != '\0') {
        int cur = roman_value(*p);
        if (cur < 0) {
            return STATUS_INVALID_INPUT;
        }
        if (prev > 0 && cur > prev) {
            total -= 2 * prev;
            total += cur;
            prev = 0;
        } else {
            total += cur;
            prev = cur;
        }
        if (total < 0) {
            return STATUS_OVERFLOW;
        }
        ++p;
    }

    *out_value = total;
    return STATUS_OK;
}
