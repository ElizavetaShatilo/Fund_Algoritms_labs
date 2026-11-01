#ifndef PARSING_H
#define PARSING_H

#include "status.h"

status_code parse_flag(const char *arg, char *out_flag);
status_code parse_seed(const char *arg, unsigned int *out_seed);

#endif
