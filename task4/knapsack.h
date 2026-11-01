#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "status.h"

typedef struct backup_file {
    char *name;
    int size;
    int importance;
} backup_file;

typedef struct backup_selection {
    char **names;
    int count;
    int total_size;
    long long total_importance;
} backup_selection;

status_code select_backup_files(const backup_file *files, int n, int capacity, backup_selection *out_result);

void free_backup_selection(backup_selection *result);

#endif
