#ifndef STATUS_CODE_H
#define STATUS_CODE_H

typedef enum status_code {
    STATUS_OK = 0,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_BASE,
    STATUS_INVALID_DIGIT,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW
} status_code;

#endif
