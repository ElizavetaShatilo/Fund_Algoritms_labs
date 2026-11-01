#include "strings.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

status_code string_length(const char *s, size_t *out_len) {
    size_t n = 0;

    if (s == NULL || out_len == NULL) {
        return STATUS_NULL_PTR;
    }
    while (s[n] != '\0') {
        ++n;
    }
    *out_len = n;
    return STATUS_OK;
}

static status_code allocate_string(size_t len, char **out) {
    char *buffer;

    if (out == NULL) {
        return STATUS_NULL_PTR;
    }
    if (len == SIZE_MAX) {
        return STATUS_OVERFLOW;
    }
    buffer = (char *)malloc(len + 1);
    if (buffer == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    buffer[len] = '\0';
    *out = buffer;
    return STATUS_OK;
}

status_code string_reverse(const char *s, char **out_result) {
    size_t len = 0;
    size_t i;
    char *result = NULL;
    status_code st;

    if (s == NULL || out_result == NULL) {
        return STATUS_NULL_PTR;
    }

    st = string_length(s, &len);
    if (st != STATUS_OK) {
        return st;
    }
    st = allocate_string(len, &result);
    if (st != STATUS_OK) {
        return st;
    }
    for (i = 0; i < len; ++i) {
        result[i] = s[len - 1 - i];
    }
    *out_result = result;
    return STATUS_OK;
}

status_code string_upper_odd(const char *s, char **out_result) {
    size_t len = 0;
    size_t i;
    char *result = NULL;
    status_code st;

    if (s == NULL || out_result == NULL) {
        return STATUS_NULL_PTR;
    }

    st = string_length(s, &len);
    if (st != STATUS_OK) {
        return st;
    }
    st = allocate_string(len, &result);
    if (st != STATUS_OK) {
        return st;
    }
    for (i = 0; i < len; ++i) {
        if ((i % 2u) == 1u) {
            result[i] = (char)toupper((unsigned char)s[i]);
        } else {
            result[i] = s[i];
        }
    }
    *out_result = result;
    return STATUS_OK;
}

status_code string_group_chars(const char *s, char **out_result) {
    size_t len = 0;
    size_t i;
    size_t pos = 0;
    char *result = NULL;
    status_code st;

    if (s == NULL || out_result == NULL) {
        return STATUS_NULL_PTR;
    }

    st = string_length(s, &len);
    if (st != STATUS_OK) {
        return st;
    }
    st = allocate_string(len, &result);
    if (st != STATUS_OK) {
        return st;
    }

    for (i = 0; i < len; ++i) {
        if (isdigit((unsigned char)s[i])) {
            result[pos++] = s[i];
        }
    }
    for (i = 0; i < len; ++i) {
        if (isalpha((unsigned char)s[i])) {
            result[pos++] = s[i];
        }
    }
    for (i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)s[i]) && !isalpha((unsigned char)s[i])) {
            result[pos++] = s[i];
        }
    }

    *out_result = result;
    return STATUS_OK;
}

status_code string_concat_random(const char *first, const char *const *rest, size_t rest_count, unsigned int seed, char **out_result) {
    const char **items = NULL;
    size_t count;
    size_t total = 0;
    size_t len = 0;
    size_t i;
    size_t j;
    size_t pos = 0;
    size_t k;
    char *result = NULL;
    status_code st;

    if (first == NULL || out_result == NULL || (rest == NULL && rest_count > 0)) {
        return STATUS_NULL_PTR;
    }
    if (rest_count == SIZE_MAX) {
        return STATUS_OVERFLOW;
    }

    count = rest_count + 1;
    if (count > SIZE_MAX / sizeof(*items)) {
        return STATUS_OVERFLOW;
    }

    items = (const char **)malloc(count * sizeof(*items));
    if (items == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    items[0] = first;
    for (i = 0; i < rest_count; ++i) {
        if (rest[i] == NULL) {
            free((void *)items);
            return STATUS_NULL_PTR;
        }
        items[i + 1] = rest[i];
    }

    for (i = 0; i < count; ++i) {
        st = string_length(items[i], &len);
        if (st != STATUS_OK) {
            free((void *)items);
            return st;
        }
        if (len > SIZE_MAX - 1 - total) {
            free((void *)items);
            return STATUS_OVERFLOW;
        }
        total += len;
    }

    st = allocate_string(total, &result);
    if (st != STATUS_OK) {
        free((void *)items);
        return st;
    }

    srand(seed);
    for (i = count; i > 1; --i) {
        const char *tmp;
        j = (size_t)rand() % i;
        tmp = items[i - 1];
        items[i - 1] = items[j];
        items[j] = tmp;
    }

    for (i = 0; i < count; ++i) {
        for (k = 0; items[i][k] != '\0'; ++k) {
            result[pos++] = items[i][k];
        }
    }

    free((void *)items);
    *out_result = result;
    return STATUS_OK;
}
