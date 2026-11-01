#ifndef ZECKENDORF_H
#define ZECKENDORF_H

#include "status.h"

status_code zeckendorf_to_uint(const char *str, unsigned int *out_value);

#endif
