#ifndef PARSING_H
#define PARSING_H

#include <stddef.h>
#include "status.h"

status_code parse_flag(const char *arg, char *out_flag);
status_code parse_unsigned_ll(const char *s, unsigned long long *out);
status_code parse_salary(const char *s, double *out);
status_code parse_latin_name(const char *s);
status_code next_token(const char **pos, char *out, size_t out_size);
int is_blank_line(const char *s);

#endif
