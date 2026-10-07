//
// Compact printf family for the native Atari build. Replaces MiNTlib's vfprintf, which
// unconditionally drags in its long-double/multi-precision float formatter (~30 KB).
// Supports: flags -0+ #, width/precision (incl. *), h/hh/l/ll/z/j/t, d i u o x X p c s % f.
//

#include "config.h"

#if defined(ALIS_USE_NATIVE_ATARI)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    char  *buf;     // snprintf target (NULL for FILE)
    size_t cap;
    FILE  *fp;
    char   chunk[128];
    int    n;       // chars produced (snprintf semantics: counts past cap)
    int    used;    // bytes pending in chunk
} sink;

static void put(sink *s, char c)
{
    if (s->fp) {
        s->chunk[s->used++] = c;
        if (s->used == (int)sizeof(s->chunk)) { fwrite(s->chunk, 1, s->used, s->fp); s->used = 0; }
    } else if ((size_t)s->n + 1 < s->cap) {
        s->buf[s->n] = c;
    }
    s->n++;
}

static void pad(sink *s, char c, int k) { while (k-- > 0) put(s, c); }

// Digits of v (base 8/10/16) right-aligned in tmp; returns start.
static char *utoa_r(unsigned long long v, int base, int upper, char *end)
{
    const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    *--end = 0;
    do { *--end = dig[v % base]; v /= base; } while (v);
    return end;
}

static void emit(sink *s, const char *pre, const char *body, int len, int width, int left, int zero)
{
    int plen = (int)strlen(pre), fill = width - plen - len;
    if (!left && !zero) pad(s, ' ', fill);
    while (*pre) put(s, *pre++);
    if (!left && zero) pad(s, '0', fill);
    while (len-- > 0) put(s, *body++);
    if (left) pad(s, ' ', fill);
}

static void format(sink *s, const char *f, va_list ap)
{
    char tmp[48];
    for (; *f; f++) {
        if (*f != '%') { put(s, *f); continue; }
        int left = 0, zero = 0, plus = 0, space = 0, alt = 0, width = 0, prec = -1, lng = 0;
        for (;; f++) {
            if      (f[1] == '-') left = 1;
            else if (f[1] == '0') zero = 1;
            else if (f[1] == '+') plus = 1;
            else if (f[1] == ' ') space = 1;
            else if (f[1] == '#') alt = 1;
            else break;
        }
        f++;
        if (*f == '*') { width = va_arg(ap, int); if (width < 0) { left = 1; width = -width; } f++; }
        else while (*f >= '0' && *f <= '9') width = width * 10 + (*f++ - '0');
        if (*f == '.') {
            f++; prec = 0;
            if (*f == '*') { prec = va_arg(ap, int); f++; }
            else while (*f >= '0' && *f <= '9') prec = prec * 10 + (*f++ - '0');
        }
        while (*f == 'h' || *f == 'l' || *f == 'z' || *f == 'j' || *f == 't' || *f == 'L')
            lng += (*f == 'l') ? 1 : (*f == 'h') ? 0 : 1, f++;
        if (left) zero = 0;

        char c = *f;
        if (!c) break;
        if (c == 'd' || c == 'i' || c == 'u' || c == 'x' || c == 'X' || c == 'o' || c == 'p') {
            unsigned long long v; int neg = 0;
            if (c == 'p') { v = (unsigned long)va_arg(ap, void *); alt = 1; }
            else if (c == 'd' || c == 'i') {
                long long sv = lng >= 2 ? va_arg(ap, long long) : lng ? va_arg(ap, long) : va_arg(ap, int);
                neg = sv < 0; v = neg ? -(unsigned long long)sv : (unsigned long long)sv;
            } else
                v = lng >= 2 ? va_arg(ap, unsigned long long) : lng ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
            int base = (c == 'o') ? 8 : (c == 'd' || c == 'i' || c == 'u') ? 10 : 16;
            char *d = utoa_r(v, base, c == 'X', tmp + sizeof(tmp));
            int len = (int)strlen(d);
            if (prec == 0 && v == 0) len = 0;
            const char *pre = neg ? "-" : plus && base == 10 && c != 'u' ? "+" : space && base == 10 && c != 'u' ? " " :
                              (alt && base == 16 && (v || c == 'p')) ? (c == 'X' ? "0X" : "0x") : (alt && base == 8) ? "0" : "";
            if (prec >= 0) {                              // precision = minimum digits
                zero = 0;
                while (len < prec) { *--d = '0'; len++; }
            }
            emit(s, pre, d, len, width, left, zero);
        } else if (c == 'c') {
            char ch = (char)va_arg(ap, int);
            emit(s, "", &ch, 1, width, left, 0);
        } else if (c == 's') {
            const char *str = va_arg(ap, const char *);
            if (!str) str = "(null)";
            int len = (int)strlen(str);
            if (prec >= 0 && len > prec) len = prec;
            emit(s, "", str, len, width, left, 0);
        } else if (c == 'f' || c == 'F' || c == 'e' || c == 'E' || c == 'g' || c == 'G') {
            double v = va_arg(ap, double);                // e/g printed as fixed point
            if (prec < 0) prec = 6;
            if (prec > 9) prec = 9;
            int neg = v < 0; if (neg) v = -v;
            unsigned long scale = 1; for (int i = 0; i < prec; i++) scale *= 10;
            unsigned long long ip = (unsigned long long)v;
            unsigned long fp = (unsigned long)((v - (double)ip) * scale + 0.5);
            if (fp >= scale) { ip++; fp -= scale; }
            char *e = tmp + sizeof(tmp), *d = e - 1; *d = 0;
            for (int i = 0; i < prec; i++) { *--d = (char)('0' + fp % 10); fp /= 10; }
            if (prec || alt) *--d = '.';
            do { *--d = (char)('0' + ip % 10); ip /= 10; } while (ip);
            emit(s, neg ? "-" : plus ? "+" : space ? " " : "", d, (int)(e - 1 - d), width, left, zero);
        } else {
            put(s, c);                                    // %% and unknown conversions
        }
    }
}

static int to_file(FILE *fp, const char *f, va_list ap)
{
    sink s = { .fp = fp };
    format(&s, f, ap);
    if (s.used) fwrite(s.chunk, 1, s.used, fp);
    return s.n;
}

static int to_buf(char *buf, size_t cap, const char *f, va_list ap)
{
    sink s = { .buf = buf, .cap = cap };
    format(&s, f, ap);
    if (cap) buf[(size_t)s.n < cap ? (size_t)s.n : cap - 1] = 0;
    return s.n;
}

int vfprintf(FILE *fp, const char *f, va_list ap)              { return to_file(fp, f, ap); }
int vprintf(const char *f, va_list ap)                         { return to_file(stdout, f, ap); }
int vsnprintf(char *b, size_t n, const char *f, va_list ap)    { return to_buf(b, n, f, ap); }
int vsprintf(char *b, const char *f, va_list ap)               { return to_buf(b, (size_t)-1 >> 1, f, ap); }

int printf(const char *f, ...)            { va_list ap; va_start(ap, f); int r = to_file(stdout, f, ap); va_end(ap); return r; }
int fprintf(FILE *fp, const char *f, ...) { va_list ap; va_start(ap, f); int r = to_file(fp, f, ap); va_end(ap); return r; }
int sprintf(char *b, const char *f, ...)  { va_list ap; va_start(ap, f); int r = to_buf(b, (size_t)-1 >> 1, f, ap); va_end(ap); return r; }
int snprintf(char *b, size_t n, const char *f, ...) { va_list ap; va_start(ap, f); int r = to_buf(b, n, f, ap); va_end(ap); return r; }

#endif // ALIS_USE_NATIVE_ATARI
