#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>
#include "status.h"

status_code string_length(const char *s, size_t *out_len);
status_code string_reverse(const char *s, char **out_result);
status_code string_upper_odd(const char *s, char **out_result);
status_code string_group_chars(const char *s, char **out_result);
status_code string_concat_random(const char *first, const char *const *rest, size_t rest_count,unsigned int seed,char **out_result);

#endif
