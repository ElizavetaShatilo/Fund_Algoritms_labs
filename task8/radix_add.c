#include "radix_add.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int symbol_to_value(char symbol) {
    if (symbol >= '0' && symbol <= '9') {
        return symbol - '0';
    }
    if (symbol >= 'a' && symbol <= 'z') {
        return symbol - 'a' + 10;
    }
    if (symbol >= 'A' && symbol <= 'Z') {
        return symbol - 'A' + 10;
    }
    return -1;
}

static char value_to_symbol(int value) {
    if (value >= 0 && value <= 9) {
        return (char)('0' + value);
    }
    if (value >= 10 && value <= 35) {
        return (char)('A' + (value - 10));
    }
    return '0';
}

status_code check_number_string(const char *number, int base) {
    if (number == NULL || number[0] == '\0') {
        return STATUS_INVALID_ARGUMENT;
    }
    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }

    for (size_t i = 0; number[i] != '\0'; ++i) {
        int digit = symbol_to_value(number[i]);
        if (digit < 0 || digit >= base) {
            return STATUS_INVALID_DIGIT;
        }
    }
    return STATUS_OK;
}

status_code add_in_base(const char *left, const char *right, int base, char **out_sum) {
    if (left == NULL || right == NULL || out_sum == NULL) {
        return STATUS_INVALID_ARGUMENT;
    }
    *out_sum = NULL;

    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }

    status_code st = check_number_string(left, base);
    if (st != STATUS_OK) {
        return st;
    }
    st = check_number_string(right, base);
    if (st != STATUS_OK) {
        return st;
    }

    size_t len_left = strlen(left);
    size_t len_right = strlen(right);
    size_t max_len = len_left > len_right ? len_left : len_right;

    if (max_len > SIZE_MAX - 2) {
        return STATUS_OVERFLOW;
    }

    char *digits = (char *)malloc(max_len + 2);
    if (digits == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    size_t i = len_left;
    size_t j = len_right;
    size_t written = 0;
    int carry = 0;

    while (i > 0 || j > 0 || carry > 0) {
        int d1 = 0;
        int d2 = 0;
        if (i > 0) {
            --i;
            d1 = symbol_to_value(left[i]);
        }
        if (j > 0) {
            --j;
            d2 = symbol_to_value(right[j]);
        }

        int total = d1 + d2 + carry;
        carry = total / base;
        digits[written++] = value_to_symbol(total % base);
    }
    digits[written] = '\0';

    for (size_t k = 0; k < written / 2; ++k) {
        char tmp = digits[k];
        digits[k] = digits[written - 1 - k];
        digits[written - 1 - k] = tmp;
    }

    size_t start = 0;
    while (digits[start] == '0' && digits[start + 1] != '\0') {
        ++start;
    }
    if (start > 0) {
        memmove(digits, digits + start, written - start + 1);
    }

    *out_sum = digits;
    return STATUS_OK;
}

status_code sum_in_base(int base, int count, char **out_sum, ...) {
    if (out_sum == NULL || count <= 0) {
        return STATUS_INVALID_ARGUMENT;
    }
    *out_sum = NULL;

    if (base < 2 || base > 36) {
        return STATUS_INVALID_BASE;
    }

    char *accumulator = (char *)malloc(2);
    if (accumulator == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    accumulator[0] = '0';
    accumulator[1] = '\0';

    va_list args;
    va_start(args, out_sum);

    for (int index = 0; index < count; ++index) {
        const char *next = va_arg(args, const char *);
        char *partial = NULL;
        status_code st = add_in_base(accumulator, next, base, &partial);
        free(accumulator);
        if (st != STATUS_OK) {
            va_end(args);
            return st;
        }
        accumulator = partial;
    }

    va_end(args);
    *out_sum = accumulator;
    return STATUS_OK;
}
