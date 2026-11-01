#ifndef ROMAN_H
#define ROMAN_H

#include <stddef.h>
#include "status.h"

status_code int_to_roman(const int value, char *buf, const size_t buf_size);

#endif
