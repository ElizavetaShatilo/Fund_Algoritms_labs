#ifndef PARSING_H
#define PARSING_H

#include <stdio.h>
#include "knapsack.h"
#include "status.h"

status_code read_backup_input(FILE *in, backup_file **out_files, int *out_n, int *out_capacity);
void free_backup_files(backup_file *files, int n);

#endif
