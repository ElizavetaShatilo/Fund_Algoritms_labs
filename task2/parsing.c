#include "parsing.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

status_code parse_flag(const char *arg, char *out_flag) {
    if (arg == NULL || out_flag == NULL) {
        return STATUS_NULL_PTR;
    }
    if ((arg[0] != '-' && arg[0] != '/') || arg[1] == '\0' || arg[2] != '\0') {
        return STATUS_UNKNOWN_FLAG;
    }
    if (arg[1] != 'a' && arg[1] != 'd') {
        return STATUS_UNKNOWN_FLAG;
    }
    *out_flag = arg[1];
    return STATUS_OK;
}

int is_blank_line(const char *s) {
    size_t i;

    if (s == NULL) {
        return 1;
    }
    for (i = 0; s[i] != '\0'; ++i) {
        if (!isspace((unsigned char)s[i])) {
            return 0;
        }
    }
    return 1;
}

status_code next_token(const char **pos, char *out, size_t out_size) {
    const char *p;
    size_t len = 0;

    if (pos == NULL || *pos == NULL || out == NULL || out_size < 2) {
        return STATUS_INVALID_INPUT;
    }

    p = *pos;
    while (*p != '\0' && isspace((unsigned char)*p)) {
        ++p;
    }
    if (*p == '\0') {
        return STATUS_INVALID_DATA;
    }

    while (*p != '\0' && !isspace((unsigned char)*p)) {
        if (len + 1 >= out_size) {
            return STATUS_OVERFLOW;
        }
        out[len++] = *p++;
    }
    out[len] = '\0';
    *pos = p;
    return STATUS_OK;
}

status_code parse_unsigned_ll(const char *s, unsigned long long *out) {
    unsigned long long value = 0;
    size_t i;

    if (s == NULL || out == NULL) {
        return STATUS_NULL_PTR;
    }
    if (s[0] == '\0') {
        return STATUS_INVALID_DATA;
    }

    for (i = 0; s[i] != '\0'; ++i) {
        unsigned long long digit;

        if (!isdigit((unsigned char)s[i])) {
            return STATUS_INVALID_DATA;
        }
        digit = (unsigned long long)(s[i] - '0');
        if (value > (ULLONG_MAX - digit) / 10ull) {
            return STATUS_OVERFLOW;
        }
        value = value * 10ull + digit;
    }

    *out = value;
    return STATUS_OK;
}

status_code parse_salary(const char *s, double *out) {
    size_t i;
    size_t digits = 0;
    int has_dot = 0;
    char *endptr = NULL;
    double val;

    if (s == NULL || out == NULL) {
        return STATUS_NULL_PTR;
    }
    if (s[0] == '\0') {
        return STATUS_INVALID_DATA;
    }

    for (i = 0; s[i] != '\0'; ++i) {
        if (s[i] >= '0' && s[i] <= '9') {
            ++digits;
        } else if (s[i] == '.' && !has_dot) {
            has_dot = 1;
        } else {
            return STATUS_INVALID_DATA;
        }
    }
    if (digits == 0) {
        return STATUS_INVALID_DATA;
    }

    val = strtod(s, &endptr);
    if (endptr == s || *endptr != '\0') {
        return STATUS_INVALID_DATA;
    }
    if (val == HUGE_VAL) {
        return STATUS_OVERFLOW;
    }

    *out = val;
    return STATUS_OK;
}

status_code parse_latin_name(const char *s) {
    size_t i;

    if (s == NULL) {
        return STATUS_NULL_PTR;
    }
    if (s[0] == '\0') {
        return STATUS_INVALID_DATA;
    }

    for (i = 0; s[i] != '\0'; ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            return STATUS_INVALID_DATA;
        }
    }
    return STATUS_OK;
}
