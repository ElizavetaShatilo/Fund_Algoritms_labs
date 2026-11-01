#include "parsing.h"

#include <ctype.h>
#include <limits.h>
#include <stddef.h>


status_code parse_flag(const char *arg, char *out_flag) {
    if (arg == NULL || out_flag == NULL) {
        return STATUS_NULL_PTR;
    }
    if ((arg[0] != '-' && arg[0] != '/') || arg[1] == '\0' || arg[2] != '\0') {
        return STATUS_UNKNOWN_FLAG;
    }

    switch (arg[1]) {
        case 'l':
        case 'r':
        case 'u':
        case 'n':
        case 'c':
            *out_flag = arg[1];
            return STATUS_OK;
        default:
            return STATUS_UNKNOWN_FLAG;
    }
}

status_code parse_seed(const char *arg, unsigned int *out_seed) {
    unsigned int value = 0;
    size_t i = 0;

    if (arg == NULL || out_seed == NULL) {
        return STATUS_NULL_PTR;
    }
    if (arg[0] == '\0') {
        return STATUS_INVALID_INPUT;
    }

    for (i = 0; arg[i] != '\0'; ++i) {
        unsigned int digit;

        if (!isdigit((unsigned char)arg[i])) {
            return STATUS_INVALID_INPUT;
        }
        digit = (unsigned int)(arg[i] - '0');
        if (value > (UINT_MAX - digit) / 10u) {
            return STATUS_OVERFLOW;
        }
        value = value * 10u + digit;
    }

    *out_seed = value;
    return STATUS_OK;
}
