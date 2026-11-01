#include "overscan.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "roman.h"
#include "zeckendorf.h"

#define TOKEN_MAX 256

typedef struct {
    FILE *file;
    const char *str;
    size_t pos;
    int at_eof;
} scan_src;

static void src_init_file(scan_src *s, FILE *f) {
    s->file = f;
    s->str = NULL;
    s->pos = 0U;
    s->at_eof = 0;
}

static void src_init_str(scan_src *s, const char *str) {
    s->file = NULL;
    s->str = str;
    s->pos = 0U;
    s->at_eof = 0;
}

static int src_getc(scan_src *s) {
    int c;

    if (s == NULL) {
        return EOF;
    }
    if (s->file != NULL) {
        c = fgetc(s->file);
    } else if (s->str != NULL) {
        if (s->str[s->pos] == '\0') {
            c = EOF;
        } else {
            c = (unsigned char)s->str[s->pos++];
        }
    } else {
        c = EOF;
    }
    if (c == EOF) {
        s->at_eof = 1;
    }
    return c;
}

static void src_ungetc(scan_src *s, const int c) {
    if (s == NULL || c == EOF) {
        return;
    }
    if (s->file != NULL) {
        if (ungetc(c, s->file) == EOF) {
            return;
        }
        s->at_eof = 0;
        return;
    }
    if (s->pos > 0U) {
        --s->pos;
    }
    s->at_eof = 0;
}

static void skip_ws(scan_src *s) {
    int c;
    for (;;) {
        c = src_getc(s);
        if (c == EOF) {
            return;
        }
        if (!isspace(c)) {
            src_ungetc(s, c);
            return;
        }
    }
}

static int is_custom_scan(const char *p, char out_tag[3]) {
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
    return 0;
}

static int is_roman_char(const int c) {
    return c == 'I' || c == 'V' || c == 'X' || c == 'L' ||
           c == 'C' || c == 'D' || c == 'M';
}

static status_code read_roman_token(scan_src *s, char *buf, const size_t buflen, int *out_negative) {
    size_t n;
    int c;

    if (s == NULL || buf == NULL || buflen < 2U || out_negative == NULL) {
        return STATUS_NULL_PTR;
    }

    *out_negative = 0;
    skip_ws(s);
    c = src_getc(s);
    if (c == EOF) {
        return STATUS_EOF;
    }
    if (c == '-') {
        *out_negative = 1;
    } else {
        src_ungetc(s, c);
    }

    n = 0U;
    for (;;) {
        c = src_getc(s);
        if (c == EOF) {
            break;
        }
        if (!is_roman_char(c)) {
            src_ungetc(s, c);
            break;
        }
        if (n + 1U >= buflen) {
            return STATUS_OVERFLOW;
        }
        buf[n++] = (char)c;
    }
    buf[n] = '\0';
    if (n == 0U) {
        return s->at_eof ? STATUS_EOF : STATUS_MATCH_ERROR;
    }
    return STATUS_OK;
}

static status_code read_zeck_token(scan_src *s, char *buf, const size_t buflen) {
    size_t n;
    int c;
    int prev_one;

    if (s == NULL || buf == NULL || buflen < 2U) {
        return STATUS_NULL_PTR;
    }

    skip_ws(s);
    n = 0U;
    prev_one = 0;
    for (;;) {
        c = src_getc(s);
        if (c == EOF) {
            break;
        }
        if (c != '0' && c != '1') {
            src_ungetc(s, c);
            break;
        }
        if (n + 1U >= buflen) {
            return STATUS_OVERFLOW;
        }
        buf[n++] = (char)c;
        if (prev_one && c == '1') {
            break;
        }
        prev_one = (c == '1');
    }
    buf[n] = '\0';
    if (n == 0U) {
        return s->at_eof ? STATUS_EOF : STATUS_MATCH_ERROR;
    }
    return STATUS_OK;
}

static int char_is_base_digit(const int c, const int base, const int expect_upper) {
    int digit;

    if (c >= '0' && c <= '9') {
        digit = c - '0';
    } else if (!expect_upper && c >= 'a' && c <= 'z') {
        digit = 10 + (c - 'a');
    } else if (expect_upper && c >= 'A' && c <= 'Z') {
        digit = 10 + (c - 'A');
    } else {
        return 0;
    }
    return digit >= 0 && digit < base;
}

static status_code read_base_token(scan_src *s, char *buf, const size_t buflen, const int base, const int expect_upper) {
    size_t n;
    int c;

    if (s == NULL || buf == NULL || buflen < 2U) {
        return STATUS_NULL_PTR;
    }

    skip_ws(s);
    n = 0U;
    c = src_getc(s);
    if (c == '+' || c == '-') {
        if (n + 1U >= buflen) {
            return STATUS_OVERFLOW;
        }
        buf[n++] = (char)c;
    } else if (c != EOF) {
        src_ungetc(s, c);
    }

    for (;;) {
        c = src_getc(s);
        if (c == EOF) {
            break;
        }
        if (!char_is_base_digit(c, base, expect_upper)) {
            src_ungetc(s, c);
            break;
        }
        if (n + 1U >= buflen) {
            return STATUS_OVERFLOW;
        }
        buf[n++] = (char)c;
    }
    buf[n] = '\0';
    if (n == 0U || (n == 1U && (buf[0] == '+' || buf[0] == '-'))) {
        return s->at_eof ? STATUS_EOF : STATUS_MATCH_ERROR;
    }
    return STATUS_OK;
}

static status_code handle_custom_scan(const char tag[3], scan_src *s, va_list *ap, int *assigned) {
    char token[TOKEN_MAX];
    status_code st;

    if (tag == NULL || s == NULL || ap == NULL || assigned == NULL) {
        return STATUS_NULL_PTR;
    }

    if (tag[0] == 'R' && tag[1] == 'o') {
        int *out = va_arg(*ap, int *);
        int value;
        int negative = 0;
        st = read_roman_token(s, token, sizeof(token), &negative);
        if (st != STATUS_OK) {
            return st;
        }
        st = roman_to_int(token, &value);
        if (st != STATUS_OK) {
            return st;
        }
        if (negative) {
            value = -value;
        }
        if (out != NULL) {
            *out = value;
        }
        ++(*assigned);
        return STATUS_OK;
    }

    if (tag[0] == 'Z' && tag[1] == 'r') {
        unsigned int *out = va_arg(*ap, unsigned int *);
        unsigned int value;
        st = read_zeck_token(s, token, sizeof(token));
        if (st != STATUS_OK) {
            return st;
        }
        st = zeckendorf_to_uint(token, &value);
        if (st != STATUS_OK) {
            return st;
        }
        if (out != NULL) {
            *out = value;
        }
        ++(*assigned);
        return STATUS_OK;
    }

    if (tag[0] == 'C' && (tag[1] == 'v' || tag[1] == 'V')) {
        int *out = va_arg(*ap, int *);
        int base = va_arg(*ap, int);
        int value;
        int expect_upper = (tag[1] == 'V');
        base = normalize_base(base);
        st = read_base_token(s, token, sizeof(token), base, expect_upper);
        if (st != STATUS_OK) {
            return st;
        }
        st = base_str_to_int(token, base, expect_upper, &value);
        if (st != STATUS_OK) {
            return st;
        }
        if (out != NULL) {
            *out = value;
        }
        ++(*assigned);
        return STATUS_OK;
    }

    return STATUS_UNSUPPORTED_FORMAT;
}

static const char *skip_std_scanspec(const char *p) {
    if (p == NULL) {
        return NULL;
    }
    if (*p == '*') {
        ++p;
    }
    while (*p >= '0' && *p <= '9') {
        ++p;
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

static status_code emit_std_scan(const char *fmt_start, const char *fmt_end, scan_src *s, va_list *ap, int *assigned) {
    char fmtbuf[64];
    size_t flen;
    char spec;
    int suppress;
    const char *q;
    int n;

    if (fmt_start == NULL || fmt_end == NULL || s == NULL || ap == NULL || assigned == NULL) {
        return STATUS_NULL_PTR;
    }

    flen = (size_t)(fmt_end - fmt_start);
    if (flen == 0U || flen >= sizeof(fmtbuf)) {
        return STATUS_UNSUPPORTED_FORMAT;
    }
    memcpy(fmtbuf, fmt_start, flen);
    fmtbuf[flen] = '\0';

    q = fmt_start + 1;
    suppress = 0;
    if (*q == '*') {
        suppress = 1;
        ++q;
    }
    while (*q >= '0' && *q <= '9') {
        ++q;
    }
    if (*q == 'h' || *q == 'l' || *q == 'L' || *q == 'z' || *q == 'j' || *q == 't') {
        if (*q == 'h' && q[1] == 'h') {
            q += 2;
        } else if (*q == 'l' && q[1] == 'l') {
            q += 2;
        } else {
            ++q;
        }
    }
    spec = *q;

    if (s->file != NULL) {
        if (suppress) {
            n = fscanf(s->file, fmtbuf);
            if (n == EOF) {
                return STATUS_EOF;
            }
            if (n != 0 && spec != '%') {
                return STATUS_MATCH_ERROR;
            }
            return STATUS_OK;
        }

        switch (spec) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o': {
                void *p = va_arg(*ap, void *);
                n = fscanf(s->file, fmtbuf, p);
                break;
            }
            case 'c': {
                char *p = va_arg(*ap, char *);
                n = fscanf(s->file, fmtbuf, p);
                break;
            }
            case 's': {
                char *p = va_arg(*ap, char *);
                n = fscanf(s->file, fmtbuf, p);
                break;
            }
            case 'f':
            case 'e':
            case 'g':
            case 'F':
            case 'E':
            case 'G': {
                void *p = va_arg(*ap, void *);
                n = fscanf(s->file, fmtbuf, p);
                break;
            }
            case 'p': {
                void **p = va_arg(*ap, void **);
                n = fscanf(s->file, fmtbuf, p);
                break;
            }
            case '%':
                n = fscanf(s->file, "%%");
                if (n == EOF) {
                    return STATUS_EOF;
                }
                return STATUS_OK;
            default:
                return STATUS_UNSUPPORTED_FORMAT;
        }
        if (n == EOF) {
            return (*assigned == 0) ? STATUS_EOF : STATUS_MATCH_ERROR;
        }
        if (n != 1) {
            return STATUS_MATCH_ERROR;
        }
        ++(*assigned);
        return STATUS_OK;
    }

    {
        const char *rest = s->str + s->pos;
        int consumed = 0;
        char scanfmt[96];
        void *p;

        if (spec == '%') {
            const char *r = rest;
            while (isspace((unsigned char)*r)) {
                ++r;
            }
            if (*r != '%') {
                return STATUS_MATCH_ERROR;
            }
            s->pos += (size_t)(r - rest) + 1U;
            return STATUS_OK;
        }

        if (flen + 2U >= sizeof(scanfmt)) {
            return STATUS_UNSUPPORTED_FORMAT;
        }
        memcpy(scanfmt, fmtbuf, flen);
        scanfmt[flen] = '%';
        scanfmt[flen + 1U] = 'n';
        scanfmt[flen + 2U] = '\0';

        if (suppress) {
            n = sscanf(rest, scanfmt, &consumed);
            if (n == EOF) {
                return (*assigned == 0) ? STATUS_EOF : STATUS_MATCH_ERROR;
            }
            if (consumed <= 0) {
                return STATUS_MATCH_ERROR;
            }
            s->pos += (size_t)consumed;
            return STATUS_OK;
        }

        switch (spec) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'c':
            case 's':
            case 'f':
            case 'e':
            case 'g':
            case 'F':
            case 'E':
            case 'G':
            case 'p':
                p = va_arg(*ap, void *);
                n = sscanf(rest, scanfmt, p, &consumed);
                break;
            default:
                return STATUS_UNSUPPORTED_FORMAT;
        }

        if (n == EOF) {
            return (*assigned == 0) ? STATUS_EOF : STATUS_MATCH_ERROR;
        }
        if (n != 1) {
            return STATUS_MATCH_ERROR;
        }
        s->pos += (size_t)consumed;
        ++(*assigned);
        return STATUS_OK;
    }
}

static status_code over_vscan(scan_src *s, const char *format, va_list *ap, int *assigned) {
    const char *p;
    status_code st;
    int local_assigned = 0;
    int *out_assigned;

    if (s == NULL || format == NULL || ap == NULL) {
        return STATUS_NULL_PTR;
    }

    out_assigned = (assigned != NULL) ? assigned : &local_assigned;
    *out_assigned = 0;

    p = format;
    while (*p != '\0') {
        if (isspace((unsigned char)*p)) {
            skip_ws(s);
            while (isspace((unsigned char)*p)) {
                ++p;
            }
            continue;
        }

        if (*p != '%') {
            int c = src_getc(s);
            if (c == EOF) {
                return (*out_assigned == 0) ? STATUS_EOF : STATUS_MATCH_ERROR;
            }
            if (c != (unsigned char)*p) {
                src_ungetc(s, c);
                return STATUS_MATCH_ERROR;
            }
            ++p;
            continue;
        }

        if (p[1] == '%') {
            int c = src_getc(s);
            if (c == EOF) {
                return (*out_assigned == 0) ? STATUS_EOF : STATUS_MATCH_ERROR;
            }
            if (c != '%') {
                src_ungetc(s, c);
                return STATUS_MATCH_ERROR;
            }
            p += 2;
            continue;
        }

        {
            char tag[3];
            if (is_custom_scan(p + 1, tag)) {
                st = handle_custom_scan(tag, s, ap, out_assigned);
                if (st != STATUS_OK) {
                    return st;
                }
                p += 3;
                continue;
            }
        }

        {
            const char *start = p;
            const char *end = skip_std_scanspec(p + 1);
            if (end == NULL || end <= p + 1) {
                return STATUS_UNSUPPORTED_FORMAT;
            }
            st = emit_std_scan(start, end, s, ap, out_assigned);
            if (st != STATUS_OK) {
                return st;
            }
            p = end;
        }
    }

    return STATUS_OK;
}

status_code overfscanf(FILE *stream, int *assigned, const char *format, ...) {
    scan_src src;
    va_list ap;
    status_code st;

    if (stream == NULL || format == NULL) {
        return STATUS_NULL_PTR;
    }

    src_init_file(&src, stream);
    va_start(ap, format);
    st = over_vscan(&src, format, &ap, assigned);
    va_end(ap);
    return st;
}

status_code oversscanf(const char *str, int *assigned, const char *format, ...) {
    scan_src src;
    va_list ap;
    status_code st;

    if (str == NULL || format == NULL) {
        return STATUS_NULL_PTR;
    }

    src_init_str(&src, str);
    va_start(ap, format);
    st = over_vscan(&src, format, &ap, assigned);
    va_end(ap);
    return st;
}
