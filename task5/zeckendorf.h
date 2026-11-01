#ifndef ZECKENDORF_H
#define ZECKENDORF_H

#include <stddef.h>
#include "status.h"

status_code uint_to_zeckendorf(const unsigned int value, char *buf, const size_t buf_size);

#endif
