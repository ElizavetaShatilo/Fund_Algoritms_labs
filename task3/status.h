#ifndef STATUS_H
#define STATUS_H

typedef enum status_code {
    STATUS_OK = 0,
    STATUS_INVALID_INPUT,
    STATUS_NULL_PTR,
    STATUS_MEMORY_ERROR,
    STATUS_FILE_ERROR
} status_code;

#endif
