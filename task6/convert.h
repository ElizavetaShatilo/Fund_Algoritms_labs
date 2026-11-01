#ifndef CONVERT_H
#define CONVERT_H

#include "status.h"

int normalize_base(const int base);

status_code base_str_to_int(
    const char *str,
    const int base,
    const int expect_upper,
    int *out_value
);

#endif
