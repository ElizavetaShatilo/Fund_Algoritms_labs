#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include "status.h"

typedef struct match_position {
    size_t line;
    size_t column;
} match_position;

typedef struct file_search_result {
    const char *path;
    status_code state;
    match_position *matches;
    size_t match_count;
    size_t match_capacity;
} file_search_result;

status_code search_substring_in_file(const char *path, const char *substring, file_search_result *out_result);
status_code search_in_files(const char *substring, file_search_result **out_results, size_t file_count, ...);

void free_file_search_result(file_search_result *result);
void free_file_search_results(file_search_result *results, size_t file_count);

#endif
