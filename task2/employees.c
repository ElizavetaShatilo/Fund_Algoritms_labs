#include "employees.h"
#include "parsing.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define LINE_SIZE 512

void free_employees(employee **arr, size_t *count) {
    if (arr != NULL) {
        free(*arr);
        *arr = NULL;
    }
    if (count != NULL) {
        *count = 0;
    }
}

static status_code parse_employee_line(const char *line, employee *out) {
    employee emp;
    char id_buf[NAME_MAX_LEN];
    char salary_buf[NAME_MAX_LEN];
    const char *p = line;
    status_code st;

    memset(&emp, 0, sizeof(emp));

    st = next_token(&p, id_buf, sizeof(id_buf));
    if (st != STATUS_OK) {
        return st;
    }
    st = parse_unsigned_ll(id_buf, &emp.id);
    if (st != STATUS_OK) {
        return st;
    }

    st = next_token(&p, emp.first_name, sizeof(emp.first_name));
    if (st != STATUS_OK) {
        return st;
    }
    st = parse_latin_name(emp.first_name);
    if (st != STATUS_OK) {
        return st;
    }

    st = next_token(&p, emp.last_name, sizeof(emp.last_name));
    if (st != STATUS_OK) {
        return st;
    }
    st = parse_latin_name(emp.last_name);
    if (st != STATUS_OK) {
        return st;
    }

    st = next_token(&p, salary_buf, sizeof(salary_buf));
    if (st != STATUS_OK) {
        return st;
    }
    st = parse_salary(salary_buf, &emp.salary);
    if (st != STATUS_OK) {
        return st;
    }

    if (!is_blank_line(p)) {
        return STATUS_INVALID_DATA;
    }

    *out = emp;
    return STATUS_OK;
}

static status_code grow_array(employee **arr, size_t *capacity, size_t needed) {
    size_t new_cap;
    employee *tmp;

    if (needed <= *capacity) {
        return STATUS_OK;
    }
    if (*capacity > SIZE_MAX / 2 / sizeof(employee)) {
        return STATUS_OVERFLOW;
    }
    new_cap = (*capacity == 0) ? INITIAL_CAPACITY : (*capacity * 2);
    if (new_cap < needed) {
        new_cap = needed;
    }
    tmp = (employee *)realloc(*arr, new_cap * sizeof(employee));
    if (tmp == NULL) {
        return STATUS_MEMORY_ERROR;
    }
    *arr = tmp;
    *capacity = new_cap;
    return STATUS_OK;
}

status_code read_employees(const char *path, employee **out_arr, size_t *out_count) {
    FILE *file;
    employee *arr = NULL;
    size_t capacity = INITIAL_CAPACITY;
    size_t size = 0;
    status_code st = STATUS_OK;
    char line[LINE_SIZE];

    if (path == NULL || out_arr == NULL || out_count == NULL) {
        return STATUS_NULL_PTR;
    }
    *out_arr = NULL;
    *out_count = 0;

    file = fopen(path, "r");
    if (file == NULL) {
        return STATUS_FILE_ERROR;
    }

    arr = (employee *)malloc(capacity * sizeof(employee));
    if (arr == NULL) {
        fclose(file);
        return STATUS_MEMORY_ERROR;
    }

    while (st == STATUS_OK && fgets(line, (int)sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        employee emp;

        if (len > 0 && line[len - 1] != '\n' && !feof(file)) {
            st = STATUS_OVERFLOW;
            break;
        }
        if (is_blank_line(line)) {
            continue;
        }

        st = parse_employee_line(line, &emp);
        if (st != STATUS_OK) {
            break;
        }

        st = grow_array(&arr, &capacity, size + 1);
        if (st != STATUS_OK) {
            break;
        }
        arr[size++] = emp;
    }

    if (st == STATUS_OK && ferror(file)) {
        st = STATUS_FILE_ERROR;
    }
    if (st == STATUS_OK && size == 0) {
        st = STATUS_EMPTY_FILE;
    }
    fclose(file);

    if (st != STATUS_OK) {
        free(arr);
        return st;
    }

    *out_arr = arr;
    *out_count = size;
    return STATUS_OK;
}

status_code write_employees(const char *path, const employee *arr, size_t count) {
    FILE *file;
    size_t i;
    status_code st = STATUS_OK;

    if (path == NULL || (arr == NULL && count > 0)) {
        return STATUS_NULL_PTR;
    }

    file = fopen(path, "w");
    if (file == NULL) {
        return STATUS_FILE_ERROR;
    }

    for (i = 0; i < count && st == STATUS_OK; ++i) {
        if (fprintf(file, "%llu %s %s %.2f\n",
                    arr[i].id,
                    arr[i].first_name,
                    arr[i].last_name,
                    arr[i].salary) < 0) {
            st = STATUS_FILE_ERROR;
        }
    }

    if (fclose(file) != 0 && st == STATUS_OK) {
        st = STATUS_FILE_ERROR;
    }
    return st;
}

static int name_cmp(const char *a, const char *b) {
    int c = strcmp(a, b);
    if (c < 0) {
        return -1;
    }
    if (c > 0) {
        return 1;
    }
    return 0;
}

static int compare_employees(const employee *a, const employee *b, double eps) {
    int c;

    if (fabs(a->salary - b->salary) > eps) {
        return (a->salary > b->salary) ? 1 : -1;
    }

    c = name_cmp(a->last_name, b->last_name);
    if (c != 0) {
        return c;
    }

    c = name_cmp(a->first_name, b->first_name);
    if (c != 0) {
        return c;
    }

    if (a->id < b->id) {
        return -1;
    }
    if (a->id > b->id) {
        return 1;
    }
    return 0;
}

#define DEFAULT_SALARY_EPS 1e-9

static int compare_asc(const void *left, const void *right) {
    return compare_employees((const employee *)left, (const employee *)right, DEFAULT_SALARY_EPS);
}

static int compare_desc(const void *left, const void *right) {
    return compare_employees((const employee *)right, (const employee *)left, DEFAULT_SALARY_EPS);
}

status_code sort_employees(employee *arr, size_t count, char order, double eps) {
    if (arr == NULL && count > 0) {
        return STATUS_NULL_PTR;
    }
    if (order != 'a' && order != 'd') {
        return STATUS_INVALID_INPUT;
    }
    if (eps < 0.0) {
        return STATUS_INVALID_INPUT;
    }
    (void)eps;
    if (count <= 1) {
        return STATUS_OK;
    }

    qsort(arr, count, sizeof(employee), (order == 'a') ? compare_asc : compare_desc);
    return STATUS_OK;
}
