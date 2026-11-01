#ifndef STATUS_H
#define STATUS_H

typedef enum status_code {
    STATUS_OK = 0,
    STATUS_INVALID_INPUT,
    STATUS_NULL_PTR,
    STATUS_MEMORY_ERROR,
    STATUS_OVERFLOW,
    STATUS_INVALID_RANGE,
    STATUS_IO_ERROR,
    STATUS_EOF,
    STATUS_MATCH_ERROR,
    STATUS_UNSUPPORTED_FORMAT
} status_code;

#endif
