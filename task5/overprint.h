#ifndef OVERPRINT_H
#define OVERPRINT_H

#include <stdio.h>
#include "status.h"

status_code overfprintf(FILE *stream, int *written, const char *format, ...);
status_code oversprintf(char *str, int *written, const char *format, ...);

#endif
