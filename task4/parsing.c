#include "parsing.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF 4096
#define NAME_BUF 512

static int is_blank(const char *s) {
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

static status_code parse_nonneg_int(const char *s, int *out) {
    long value = 0;
    size_t i;

    if (s == NULL || out == NULL) {
        return STATUS_NULL_PTR;
    }
    if (s[0] == '\0') {
        return STATUS_INVALID_INPUT;
    }

    for (i = 0; s[i] != '\0'; ++i) {
        int digit;
        if (!isdigit((unsigned char)s[i])) {
            return STATUS_INVALID_INPUT;
        }
        digit = s[i] - '0';
        if (value > (LONG_MAX - digit) / 10) {
            return STATUS_OVERFLOW;
        }
        value = value * 10 + digit;
    }
    if (value > (long)INT_MAX) {
        return STATUS_OVERFLOW;
    }
    *out = (int)value;
    return STATUS_OK;
}

static status_code next_token(const char **pos, char *out, size_t out_size) {
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
        return STATUS_INVALID_INPUT;
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

void free_backup_files(backup_file *files, int n) {
    int i;
    if (files == NULL) {
        return;
    }
    for (i = 0; i < n; ++i) {
        free(files[i].name);
        files[i].name = NULL;
    }
    free(files);
}

status_code read_backup_input(FILE *in, backup_file **out_files, int *out_n, int *out_capacity) {
    char line[LINE_BUF];
    char tok1[NAME_BUF];
    char tok2[NAME_BUF];
    char tok3[NAME_BUF];
    const char *p;
    status_code st;
    int n = 0;
    int capacity = 0;
    int i;
    backup_file *files = NULL;

    if (in == NULL || out_files == NULL || out_n == NULL || out_capacity == NULL) {
        return STATUS_NULL_PTR;
    }
    *out_files = NULL;
    *out_n = 0;
    *out_capacity = 0;

    if (fgets(line, (int)sizeof(line), in) == NULL) {
        return STATUS_INVALID_INPUT;
    }
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] != '\n' && !feof(in)) {
            return STATUS_OVERFLOW;
        }
    }

    p = line;
    st = next_token(&p, tok1, sizeof(tok1));
    if (st != STATUS_OK) {
        return st;
    }
    st = next_token(&p, tok2, sizeof(tok2));
    if (st != STATUS_OK) {
        return st;
    }
    if (!is_blank(p)) {
        return STATUS_INVALID_INPUT;
    }

    st = parse_nonneg_int(tok1, &n);
    if (st != STATUS_OK) {
        return st;
    }
    st = parse_nonneg_int(tok2, &capacity);
    if (st != STATUS_OK) {
        return st;
    }

    if (n == 0) {
        *out_n = 0;
        *out_capacity = capacity;
        *out_files = NULL;
        return STATUS_OK;
    }

    files = (backup_file *)calloc((size_t)n, sizeof(backup_file));
    if (files == NULL) {
        return STATUS_MEMORY_ERROR;
    }

    for (i = 0; i < n; ++i) {
        size_t name_len;
        char *name_copy;

        if (fgets(line, (int)sizeof(line), in) == NULL) {
            free_backup_files(files, n);
            return STATUS_INVALID_INPUT;
        }
        {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] != '\n' && !feof(in)) {
                free_backup_files(files, n);
                return STATUS_OVERFLOW;
            }
        }

        p = line;
        st = next_token(&p, tok1, sizeof(tok1));
        if (st != STATUS_OK) {
            free_backup_files(files, n);
            return st;
        }
        st = next_token(&p, tok2, sizeof(tok2));
        if (st != STATUS_OK) {
            free_backup_files(files, n);
            return st;
        }
        st = next_token(&p, tok3, sizeof(tok3));
        if (st != STATUS_OK) {
            free_backup_files(files, n);
            return st;
        }
        if (!is_blank(p)) {
            free_backup_files(files, n);
            return STATUS_INVALID_INPUT;
        }

        st = parse_nonneg_int(tok2, &files[i].size);
        if (st != STATUS_OK) {
            free_backup_files(files, n);
            return st;
        }
        st = parse_nonneg_int(tok3, &files[i].importance);
        if (st != STATUS_OK) {
            free_backup_files(files, n);
            return st;
        }

        name_len = strlen(tok1);
        name_copy = (char *)malloc(name_len + 1);
        if (name_copy == NULL) {
            free_backup_files(files, n);
            return STATUS_MEMORY_ERROR;
        }
        memcpy(name_copy, tok1, name_len + 1);
        files[i].name = name_copy;
    }

    *out_files = files;
    *out_n = n;
    *out_capacity = capacity;
    return STATUS_OK;
}
