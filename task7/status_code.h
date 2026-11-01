#ifndef STATUS_CODE_H
#define STATUS_CODE_H

typedef enum status_code {
    STATUS_OK = 0,
    STATUS_NULL_POINTER,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_DIMENSION,
    STATUS_MEMORY_ERROR,
    STATUS_MATRIX_NOT_SPD
} status_code;

#endif
