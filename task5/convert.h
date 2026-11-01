#ifndef CONVERT_H
#define CONVERT_H

#include <stddef.h>
#include "status.h"

status_code int_to_base_str(const int value, const int base, const int uppercase, char *buf, const size_t buf_size);
status_code base_str_to_int(const char *str, const int base, const int expect_upper, int *out_value);
status_code memory_dump_bin(const void *data, const size_t nbytes, char *buf, const size_t buf_size);

int normalize_base(const int base);

#endif
