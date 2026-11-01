#ifndef EMPLOYEES_H
#define EMPLOYEES_H

#include <stddef.h>
#include "status.h"

#define NAME_MAX_LEN 128

typedef struct employee {
    unsigned long long id;
    char first_name[NAME_MAX_LEN];
    char last_name[NAME_MAX_LEN];
    double salary;
} employee;

status_code read_employees(const char *path, employee **out_arr, size_t *out_count);
status_code write_employees(const char *path, const employee *arr, size_t count);
void free_employees(employee **arr, size_t *count);

status_code sort_employees(employee *arr, size_t count, char order, double eps);

#endif
