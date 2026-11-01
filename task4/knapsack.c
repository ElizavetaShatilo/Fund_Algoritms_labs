#include "knapsack.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void free_name_list(char **names, int count) {
    int i;
    if (names == NULL) {
        return;
    }
    for (i = 0; i < count; ++i) {
        free(names[i]);
    }
    free(names);
}

void free_backup_selection(backup_selection *result) {
    if (result == NULL) {
        return;
    }
    free_name_list(result->names, result->count);
    result->names = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;
}

status_code select_backup_files(const backup_file *files, int n, int capacity, backup_selection *out_result) {
    size_t cols;
    size_t rows;
    long long *dp = NULL;
    unsigned char *take = NULL;
    unsigned char *chosen = NULL;
    char **names = NULL;
    int i;
    int w;
    int selected_count = 0;
    int total_size = 0;
    long long total_imp;
    int w_curr;
    int pos;

    if (out_result == NULL) {
        return STATUS_NULL_PTR;
    }
    out_result->names = NULL;
    out_result->count = 0;
    out_result->total_size = 0;
    out_result->total_importance = 0;

    if (n < 0 || capacity < 0) {
        return STATUS_INVALID_INPUT;
    }
    if (files == NULL && n > 0) {
        return STATUS_NULL_PTR;
    }

    for (i = 0; i < n; ++i) {
        if (files[i].name == NULL || files[i].size < 0 || files[i].importance < 0) {
            return STATUS_INVALID_INPUT;
        }
    }

    if (n == 0) {
        return STATUS_OK;
    }

    rows = (size_t)n;
    cols = (size_t)capacity + 1;
    if (cols == 0 || rows > SIZE_MAX / cols || cols > SIZE_MAX / sizeof(long long)) {
        return STATUS_OVERFLOW;
    }

    dp = (long long *)calloc(cols, sizeof(long long));
    take = (unsigned char *)calloc(rows * cols, sizeof(unsigned char));
    chosen = (unsigned char *)calloc(rows, sizeof(unsigned char));
    if (dp == NULL || take == NULL || chosen == NULL) {
        free(dp);
        free(take);
        free(chosen);
        return STATUS_MEMORY_ERROR;
    }

    for (i = 0; i < n; ++i) {
        int sz = files[i].size;
        long long imp = (long long)files[i].importance;
        for (w = capacity; w >= sz; --w) {
            long long cand = dp[w - sz] + imp;
            if (cand > dp[w]) {
                dp[w] = cand;
                take[(size_t)i * cols + (size_t)w] = 1;
            }
        }
    }

    total_imp = dp[capacity];
    w_curr = capacity;
    for (i = n - 1; i >= 0; --i) {
        if (take[(size_t)i * cols + (size_t)w_curr]) {
            chosen[i] = 1;
            ++selected_count;
            total_size += files[i].size;
            w_curr -= files[i].size;
        }
    }

    if (selected_count > 0) {
        names = (char **)calloc((size_t)selected_count, sizeof(char *));
        if (names == NULL) {
            free(dp);
            free(take);
            free(chosen);
            return STATUS_MEMORY_ERROR;
        }
        pos = 0;
        for (i = 0; i < n; ++i) {
            size_t len;
            if (!chosen[i]) {
                continue;
            }
            len = strlen(files[i].name);
            names[pos] = (char *)malloc(len + 1);
            if (names[pos] == NULL) {
                free_name_list(names, pos);
                free(dp);
                free(take);
                free(chosen);
                return STATUS_MEMORY_ERROR;
            }
            memcpy(names[pos], files[i].name, len + 1);
            ++pos;
        }
    }

    out_result->names = names;
    out_result->count = selected_count;
    out_result->total_size = total_size;
    out_result->total_importance = total_imp;

    free(dp);
    free(take);
    free(chosen);
    return STATUS_OK;
}
