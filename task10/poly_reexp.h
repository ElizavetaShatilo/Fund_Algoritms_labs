#ifndef POLY_REEXP_H
#define POLY_REEXP_H

#include "status_code.h"

status_code reexpand_at_point(double epsilon, double a, double **out_coeffs, int n, ...);

#endif
