#include "overprint.h"

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "roman.h"
#include "zeckendorf.h"

#define TMP_BUF 512
#define FMT_PIECE 64

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} dyn_buf;

static status_code dyn_buf_init(dyn_buf *b) {
    if (b == NULL) {
        return STATUS_NULL_PTR;
    }
    b->capacity = 128U;
    b->size = 0U;
    b->data = (char *)malloc(b->capacity);
    if (b->data == NULL) {
        b->capacity = 0U;
        return STATUS_MEMORY_ERROR;
    }
    b->data[0] = '\0';
    return STATUS_OK;
}

static void dyn_buf_free(dyn_buf *b) {
    if (b == NULL) {
        return;
    }
    free(b->data);
    b->data = NULL;
    b->size = 0U;
    b->capacity = 0U;
}

static status_code dyn_buf_append(dyn_buf *b, const char *s) {
    size_t len;
    size_t need;
    char *p;

    if (b == NULL || s == NULL || b->data == NULL) {
        return STATUS_NULL_PTR;
    }

    len = strlen(s);
    need = b->size + len + 1U;
    if (need > b->capacity) {
        size_t new_cap = b->capacity;
        while (new_cap < need) {
            if (new_cap > (size_t)(-1) / 2U) {
                return STATUS_OVERFLOW;
            }
            new_cap *= 2U;
        }
        p = (char *)realloc(b->data, new_cap);
        if (p == NULL) {
            return STATUS_MEMORY_ERROR;
        }
        b->data = p;
        b->capacity = new_cap;
    }

    memcpy(b->data + b->size, s, len + 1U);
    b->size += len;
    return STATUS_OK;
}

static status_code dyn_buf_append_char(dyn_buf *b, const char ch) {
    char tmp[2];
    tmp[0] = ch;
    tmp[1] = '\0';
    return dyn_buf_append(b, tmp);
}

static int is_custom_flag(const char *p, char out_tag[3]) {
    if (p == NULL || out_tag == NULL) {
        return 0;
    }
    if (p[0] == 'R' && p[1] == 'o') {
        out_tag[0] = 'R';
        out_tag[1] = 'o';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 'Z' && p[1] == 'r') {
        out_tag[0] = 'Z';
        out_tag[1] = 'r';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 'C' && p[1] == 'v') {
        out_tag[0] = 'C';
        out_tag[1] = 'v';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 'C' && p[1] == 'V') {
        out_tag[0] = 'C';
        out_tag[1] = 'V';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 't' && p[1] == 'o') {
        out_tag[0] = 't';
        out_tag[1] = 'o';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 'T' && p[1] == 'O') {
        out_tag[0] = 'T';
        out_tag[1] = 'O';
        out_tag[2] = '\0';
        return 1;
    }
    if (p[0] == 'm' && (p[1] == 'i' || p[1] == 'u' || p[1] == 'd' || p[1] == 'f')) {
        out_tag[0] = 'm';
        out_tag[1] = p[1];
        out_tag[2] = '\0';
        return 1;
    }
    return 0;
}

static status_code handle_custom(const char tag[3], va_list *ap, dyn_buf *out) {
    char tmp[TMP_BUF];
    status_code st;

    if (tag == NULL || ap == NULL || out == NULL) {
        return STATUS_NULL_PTR;
    }

    if (tag[0] == 'R' && tag[1] == 'o') {
        int v = va_arg(*ap, int);
        st = int_to_roman(v, tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'Z' && tag[1] == 'r') {
        unsigned int v = va_arg(*ap, unsigned int);
        st = uint_to_zeckendorf(v, tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'C' && (tag[1] == 'v' || tag[1] == 'V')) {
        int v = va_arg(*ap, int);
        int base = va_arg(*ap, int);
        base = normalize_base(base);
        st = int_to_base_str(v, base, tag[1] == 'V', tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if ((tag[0] == 't' && tag[1] == 'o') || (tag[0] == 'T' && tag[1] == 'O')) {
        const char *s = va_arg(*ap, const char *);
        int base = va_arg(*ap, int);
        int value;
        base = normalize_base(base);
        st = base_str_to_int(s, base, tag[0] == 'T', &value);
        if (st != STATUS_OK) {
            return st;
        }
        {
            int n = snprintf(tmp, sizeof(tmp), "%d", value);
            if (n < 0 || (size_t)n >= sizeof(tmp)) {
                return STATUS_OVERFLOW;
            }
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'm' && tag[1] == 'i') {
        int v = va_arg(*ap, int);
        st = memory_dump_bin(&v, 4U, tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'm' && tag[1] == 'u') {
        unsigned int v = va_arg(*ap, unsigned int);
        st = memory_dump_bin(&v, 4U, tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'm' && tag[1] == 'd') {
        double v = va_arg(*ap, double);
        st = memory_dump_bin(&v, sizeof(double), tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    if (tag[0] == 'm' && tag[1] == 'f') {
        double promoted = va_arg(*ap, double);
        float v = (float)promoted;
        st = memory_dump_bin(&v, sizeof(float), tmp, sizeof(tmp));
        if (st != STATUS_OK) {
            return st;
        }
        return dyn_buf_append(out, tmp);
    }

    return STATUS_UNSUPPORTED_FORMAT;
}

static const char *skip_std_specifier(const char *p) {
    if (p == NULL) {
        return NULL;
    }
    while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
        ++p;
    }
    if (*p == '*') {
        ++p;
    } else {
        while (*p >= '0' && *p <= '9') {
            ++p;
        }
    }
    if (*p == '.') {
        ++p;
        if (*p == '*') {
            ++p;
        } else {
            while (*p >= '0' && *p <= '9') {
                ++p;
            }
        }
    }
    if (*p == 'h') {
        ++p;
        if (*p == 'h') {
            ++p;
        }
    } else if (*p == 'l') {
        ++p;
        if (*p == 'l') {
            ++p;
        }
    } else if (*p == 'L' || *p == 'z' || *p == 'j' || *p == 't') {
        ++p;
    }
    if (*p != '\0') {
        ++p;
    }
    return p;
}

static status_code emit_std(const char *fmt_start, const char *fmt_end, va_list *ap, dyn_buf *out) {
    char fmtbuf[FMT_PIECE];
    char tmp[TMP_BUF];
    size_t flen;
    const char *q;
    int has_star_w = 0;
    int has_star_p = 0;
    int width = 0;
    int prec = 0;
    char length = 0;
    char spec;
    int n;

    if (fmt_start == NULL || fmt_end == NULL || ap == NULL || out == NULL) {
        return STATUS_NULL_PTR;
    }

    flen = (size_t)(fmt_end - fmt_start);
    if (flen == 0U || flen >= sizeof(fmtbuf)) {
        return STATUS_UNSUPPORTED_FORMAT;
    }
    memcpy(fmtbuf, fmt_start, flen);
    fmtbuf[flen] = '\0';

    q = fmt_start + 1; /* после '%' */
    while (*q == '-' || *q == '+' || *q == ' ' || *q == '#' || *q == '0') {
        ++q;
    }
    if (*q == '*') {
        has_star_w = 1;
        ++q;
    } else {
        while (*q >= '0' && *q <= '9') {
            ++q;
        }
    }
    if (*q == '.') {
        ++q;
        if (*q == '*') {
            has_star_p = 1;
            ++q;
        } else {
            while (*q >= '0' && *q <= '9') {
                ++q;
            }
        }
    }
    if (*q == 'h') {
        ++q;
        if (*q == 'h') {
            length = 'H';
            ++q;
        } else {
            length = 'h';
        }
    } else if (*q == 'l') {
        ++q;
        if (*q == 'l') {
            length = 'W'; /* ll */
            ++q;
        } else {
            length = 'l';
        }
    } else if (*q == 'L') {
        length = 'D';
        ++q;
    } else if (*q == 'z') {
        length = 'z';
        ++q;
    }

    spec = *q;
    if (has_star_w) {
        width = va_arg(*ap, int);
    }
    if (has_star_p) {
        prec = va_arg(*ap, int);
    }

    switch (spec) {
        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'X':
        case 'o':
            if (length == 'W') {
                long long v = va_arg(*ap, long long);
                if (has_star_w && has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, v);
                } else if (has_star_w) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
                } else if (has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, v);
                } else {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
                }
            } else if (length == 'l' || length == 'z') {
                long v = va_arg(*ap, long);
                if (has_star_w && has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, v);
                } else if (has_star_w) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
                } else if (has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, v);
                } else {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
                }
            } else {
                int v = va_arg(*ap, int); /* hh/h тоже int после продвижения */
                if (has_star_w && has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, v);
                } else if (has_star_w) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
                } else if (has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, v);
                } else {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
                }
            }
            break;
        case 'c': {
            int v = va_arg(*ap, int);
            if (has_star_w) {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
            } else {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
            }
            break;
        }
        case 's': {
            const char *s = va_arg(*ap, const char *);
            if (has_star_w && has_star_p) {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, s);
            } else if (has_star_w) {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, width, s);
            } else if (has_star_p) {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, s);
            } else {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, s);
            }
            break;
        }
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            if (length == 'D') {
                long double v = va_arg(*ap, long double);
                if (has_star_w && has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, v);
                } else if (has_star_w) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
                } else if (has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, v);
                } else {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
                }
            } else {
                double v = va_arg(*ap, double);
                if (has_star_w && has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, prec, v);
                } else if (has_star_w) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
                } else if (has_star_p) {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, prec, v);
                } else {
                    n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
                }
            }
            break;
        case 'p': {
            void *v = va_arg(*ap, void *);
            if (has_star_w) {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, width, v);
            } else {
                n = snprintf(tmp, sizeof(tmp), fmtbuf, v);
            }
            break;
        }
        case '%':
            n = snprintf(tmp, sizeof(tmp), "%%");
            break;
        default:
            return STATUS_UNSUPPORTED_FORMAT;
    }

    if (n < 0 || (size_t)n >= sizeof(tmp)) {
        return STATUS_OVERFLOW;
    }
    return dyn_buf_append(out, tmp);
}

static status_code over_vformat(dyn_buf *out, const char *format, va_list *ap) {
    const char *p;
    status_code st;

    if (out == NULL || format == NULL || ap == NULL) {
        return STATUS_NULL_PTR;
    }

    p = format;
    while (*p != '\0') {
        if (*p != '%') {
            st = dyn_buf_append_char(out, *p);
            if (st != STATUS_OK) {
                return st;
            }
            ++p;
            continue;
        }

        if (p[1] == '%') {
            st = dyn_buf_append_char(out, '%');
            if (st != STATUS_OK) {
                return st;
            }
            p += 2;
            continue;
        }

        {
            char tag[3];
            if (is_custom_flag(p + 1, tag)) {
                st = handle_custom(tag, ap, out);
                if (st != STATUS_OK) {
                    return st;
                }
                p += 3;
                continue;
            }
        }

        {
            const char *start = p;
            const char *end = skip_std_specifier(p + 1);
            if (end == NULL || end == p + 1) {
                return STATUS_UNSUPPORTED_FORMAT;
            }
            st = emit_std(start, end, ap, out);
            if (st != STATUS_OK) {
                return st;
            }
            p = end;
        }
    }

    return STATUS_OK;
}

status_code overfprintf(FILE *stream, int *written, const char *format, ...) {
    dyn_buf buf;
    va_list ap;
    status_code st;
    size_t n;

    if (stream == NULL || format == NULL) {
        return STATUS_NULL_PTR;
    }

    st = dyn_buf_init(&buf);
    if (st != STATUS_OK) {
        return st;
    }

    va_start(ap, format);
    st = over_vformat(&buf, format, &ap);
    va_end(ap);
    if (st != STATUS_OK) {
        dyn_buf_free(&buf);
        return st;
    }

    n = buf.size;
    if (fwrite(buf.data, 1U, n, stream) != n) {
        dyn_buf_free(&buf);
        return STATUS_IO_ERROR;
    }

    if (written != NULL) {
        if (n > (size_t)INT_MAX) {
            dyn_buf_free(&buf);
            return STATUS_OVERFLOW;
        }
        *written = (int)n;
    }

    dyn_buf_free(&buf);
    return STATUS_OK;
}

status_code oversprintf(char *str, int *written, const char *format, ...) {
    dyn_buf buf;
    va_list ap;
    status_code st;

    if (str == NULL || format == NULL) {
        return STATUS_NULL_PTR;
    }

    st = dyn_buf_init(&buf);
    if (st != STATUS_OK) {
        return st;
    }

    va_start(ap, format);
    st = over_vformat(&buf, format, &ap);
    va_end(ap);
    if (st != STATUS_OK) {
        dyn_buf_free(&buf);
        return st;
    }

    memcpy(str, buf.data, buf.size + 1U);
    if (written != NULL) {
        if (buf.size > (size_t)INT_MAX) {
            dyn_buf_free(&buf);
            return STATUS_OVERFLOW;
        }
        *written = (int)buf.size;
    }

    dyn_buf_free(&buf);
    return STATUS_OK;
}
