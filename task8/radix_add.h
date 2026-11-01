#ifndef RADIX_ADD_H
#define RADIX_ADD_H

#include "status_code.h"

status_code check_number_string(const char *number, int base);
status_code add_in_base(const char *left, const char *right, int base, char **out_sum);
status_code sum_in_base(int base, int count, char **out_sum, ...);

#endif
