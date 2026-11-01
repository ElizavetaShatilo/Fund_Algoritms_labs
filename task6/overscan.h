#ifndef OVERSCAN_H
#define OVERSCAN_H

#include <stdio.h>
#include "status.h"

status_code overfscanf(FILE *stream, int *assigned, const char *format, ...);
status_code oversscanf(const char *str, int *assigned, const char *format, ...);

#endif
