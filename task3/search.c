#include "search.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define READ_CHUNK 4096

static size_t my_strlen(const char *s) {
    size_t n = 0;
    if (s == NULL) {
        return 0;
    }
    while (s[n] != '\0') {
        ++n;
    }
    return n;
}

static status_code init_result(file_search_result *result) {
    if (result == NULL) {
        return STATUS_NULL_PTR;
    }
    result->matches = NULL;
    result->match_count = 0;
    result->match_capacity = 0;
    return STATUS_OK;
}

static status_code push_match(file_search_result *result, size_t line, size_t column) {
    match_position *grown;

    if (result == NULL) {
        return STATUS_NULL_PTR;
    }
    if (result->match_count >= result->match_capacity) {
        size_t new_cap = (result->match_capacity == 0) ? 8 : result->match_capacity * 2;
        if (new_cap > SIZE_MAX / sizeof(match_position)) {
            return STATUS_MEMORY_ERROR;
        }
        grown = (match_position *)realloc(result->matches, new_cap * sizeof(match_position));
        if (grown == NULL) {
            return STATUS_MEMORY_ERROR;
        }
        result->matches = grown;
        result->match_capacity = new_cap;
    }
    result->matches[result->match_count].line = line;
    result->matches[result->match_count].column = column;
    result->match_count++;
    return STATUS_OK;
}

void free_file_search_result(file_search_result *result) {
    if (result == NULL) {
        return;
    }
    free(result->matches);
    result->matches = NULL;
    result->match_count = 0;
    result->match_capacity = 0;
}

void free_file_search_results(file_search_result *results, size_t file_count) {
    size_t i;
    if (results == NULL) {
        return;
    }
    for (i = 0; i < file_count; ++i) {
        free_file_search_result(&results[i]);
    }
    free(results);
}

status_code search_substring_in_file(const char *path, const char *substring, file_search_result *out_result) {
    FILE *file;
    char *buffer = NULL;
    size_t sub_len;
    size_t line = 1;
    size_t column = 1;
    size_t carry = 0;
    status_code st;

    if (path == NULL || substring == NULL || out_result == NULL) {
        return STATUS_NULL_PTR;
    }

    st = init_result(out_result);
    if (st != STATUS_OK) {
        return st;
    }

    sub_len = my_strlen(substring);
    if (sub_len == 0) {
        return STATUS_INVALID_INPUT;
    }
    if (sub_len > SIZE_MAX - READ_CHUNK) {
        return STATUS_INVALID_INPUT;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return STATUS_FILE_ERROR;
    }

    buffer = (char *)malloc(READ_CHUNK + sub_len);
    if (buffer == NULL) {
        fclose(file);
        return STATUS_MEMORY_ERROR;
    }

    for (;;) {
        size_t nread = fread(buffer + carry, 1, READ_CHUNK, file);
        size_t total;
        size_t i;

        if (nread == 0) {
            break;
        }

        total = carry + nread;
        i = 0;
        while (i + sub_len <= total) {
            size_t j = 0;
            while (j < sub_len && buffer[i + j] == substring[j]) {
                ++j;
            }
            if (j == sub_len) {
                st = push_match(out_result, line, column);
                if (st != STATUS_OK) {
                    free(buffer);
                    fclose(file);
                    free_file_search_result(out_result);
                    return st;
                }
            }
            if (buffer[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
            ++i;
        }

        carry = total - i;
        {
            size_t k;
            for (k = 0; k < carry; ++k) {
                buffer[k] = buffer[i + k];
            }
        }
    }

    {
        int failed = ferror(file);
        free(buffer);
        fclose(file);
        if (failed) {
            free_file_search_result(out_result);
            return STATUS_FILE_ERROR;
        }
    }
    return STATUS_OK;
}

status_code search_in_files(const char *substring, file_search_result **out_results, size_t file_count, ...) {
    file_search_result *allocated = NULL;
    size_t i;
    va_list args;
    status_code first_error = STATUS_OK;
    status_code precheck = STATUS_OK;

    if (out_results == NULL) {
        return STATUS_NULL_PTR;
    }
    *out_results = NULL;

    if (substring == NULL) {
        return STATUS_NULL_PTR;
    }
    if (file_count == 0) {
        return STATUS_INVALID_INPUT;
    }

    if (my_strlen(substring) == 0) {
        precheck = STATUS_INVALID_INPUT;
    }

    allocated = (file_search_result *)calloc(file_count, sizeof(file_search_result));
    if (allocated == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (i = 0; i < file_count; ++i) {
        allocated[i].path = NULL;
        allocated[i].state = STATUS_OK;
        init_result(&allocated[i]);
    }

    va_start(args, file_count);
    first_error = precheck;

    for (i = 0; i < file_count; ++i) {
        const char *path = va_arg(args, const char *);
        allocated[i].path = path;

        if (precheck != STATUS_OK) {
            allocated[i].state = precheck;
            continue;
        }

        allocated[i].state = search_substring_in_file(path, substring, &allocated[i]);
        if (allocated[i].state != STATUS_OK && first_error == STATUS_OK) {
            first_error = allocated[i].state;
        }
    }

    va_end(args);
    *out_results = allocated;
    return first_error;
}
