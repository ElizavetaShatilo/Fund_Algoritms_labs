#ifndef FRAC_BASE_H
#define FRAC_BASE_H

#include "status_code.h"

#include <stdbool.h>

status_code has_finite_base_form(double fraction, int base, double epsilon, bool *out_finite);
status_code check_fractions_in_base(double epsilon, int base, bool *results, int count, ...);

#endif
