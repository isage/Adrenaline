#include <psptypes.h>

#include <string.h>
#include <stdarg.h>

#define is_digit(c) (c >= '0' && c <= '9')

static int get_atoi(const char **str) {
    int n;
    for (n = 0; is_digit(**str); (*str)++) {
        n = n * 10 + **str - '0';
	}
    return n;
}

static void bputc(char *buf, size_t *pos, size_t max, char c) {
    if (*pos < max) {
        buf[(*pos)] = c;
	}
    (*pos)++;
}

#define F_ALTERNATE 0001    // put 0x infront 16, 0 on octals, b on binary
#define F_ZEROPAD   0002    // value should be zero padded
#define F_LEFT      0004    // left justified if set, otherwise right justified
#define F_SPACE     0010    // place a space before positive number
#define F_PLUS      0020    // show +/- on signed numbers, default only for -
#define F_SIGNED    0040    // is an unsigned number?
#define F_SMALL     0100    // use lowercase for hex?

/**
 * Formats an integer number
 *  buf - buffer to print into
 *  len - current position in buffer
 *  maxlen - last valid position in buf
 *  num - number to print
 *  base - it's base
 *  width - how many spaces this should have; padding
 *  flags - above F flags
 */
static void fmt_int(char *buf, size_t *len, size_t maxlen, long long num, int base, int width, int flags) {
    char nbuf[64], sign = 0;
    char altb[8]; // small buf for sign and #
    unsigned long n = num;
    int npad;           // number of pads
    char pchar = ' ';   // padding character
    char *digits = "0123456789ABCDEF";
    char *ldigits = "0123456789abcdef";
    int i, j;

    if (base < 2 || base > 16) {
        return;
	}
    if (flags & F_SMALL) digits = ldigits;
    if (flags & F_LEFT) flags &= ~F_ZEROPAD;

    if ((flags & F_SIGNED) && num < 0) {
        n = -num;
        sign = '-';
    } else if (flags & F_PLUS) {
        sign = '+';
    } else if (flags & F_SPACE) {
        sign = ' ';
	}

    i = 0;
    do {
        nbuf[i++] = digits[n % base];
        n = n / base;
    } while (n > 0);

    j = 0;
    if (sign) altb[j++] = sign;
    if (flags & F_ALTERNATE) {
        if (base == 8 || base == 16) {
            altb[j++] = '0';
            if (base == 16) {
                altb[j++] = (flags & F_SMALL) ? 'x' : 'X';
			}
        }
    }
    altb[j] = 0;

    npad = width > i + j ? width - i - j : 0;

    if (width > i + j) {
        npad = width - i - j;
	}

    if (npad > 0 && ((flags & F_LEFT) == 0)) {
        if (flags & F_ZEROPAD) {
            for (j = 0; altb[j]; j++) {
                bputc(buf, len, maxlen, altb[j]);
			}
            altb[0] = 0;
        }
        while (npad-- > 0) {
            bputc(buf, len, maxlen, (flags & F_ZEROPAD) ? '0' : ' ');
		}
    }
    for (j = 0; altb[j]; j++) {
        bputc(buf, len, maxlen, altb[j]);
	}

    while (i-- > 0) {
        bputc(buf, len, maxlen, nbuf[i]);
	}

    if (npad > 0 && (flags & F_LEFT)) {

		while(npad-- > 0) {
			bputc(buf, len, maxlen, pchar);
		}
	}
}

static void fmt_chr(char *buf, size_t *pos, size_t max, char c, int width, int flags) {
    int npad = 0;
    if (width > 0) {
		npad = width - 1;
	}
    if (npad < 0) {
		npad = 0;
	}

    if (npad && ((flags & F_LEFT) == 0)) {
		while (npad-- > 0) {
			bputc(buf, pos, max, ' ');
		}
	}

    bputc(buf, pos, max, c);

    if (npad && (flags & F_LEFT)) {
		while (npad-- > 0) {
			bputc(buf, pos, max, ' ');
		}
	}
}

static void fmt_str(char *buf, size_t *pos, size_t max, char *s, int width, int flags) {
    int npad = 0;
    if (width > 0) {
		npad = width - strlen(s);
	}
    if (npad < 0) {
		npad = 0;
	}

    if (npad && ((flags & F_LEFT) == 0)) {
		while (npad-- > 0) {
			bputc(buf, pos, max, ' ');
		}
	}

    while (*s) {
        bputc(buf, pos, max, *s++);
	}

    if (npad && (flags & F_LEFT)) {
		while (npad-- > 0) {
			bputc(buf, pos, max, ' ');
		}
	}
}

/* Format states */
#define S_DEFAULT   0
#define S_FLAGS     1
#define S_WIDTH     2
#define S_PRECIS    3
#define S_LENGTH    4
#define S_CONV      5

/* Lenght flags */
#define L_CHAR      1
#define L_SHORT     2
#define L_LONG      3
#define L_LLONG     4
#define L_DOUBLE    5

/**
 * Shrinked down, vsnprintf implementation.
 *  This will not handle floating numbers (yet).
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    size_t n = 0;
    char c, *s;
    char state = 0;
    long long num;
    int base;
    int flags, width, precision, lflags;

    if (!buf) {
		size = 0;
	}

    for (;;) {
        c = *fmt++;
        if (state == S_DEFAULT) {
            if (c == '%') {
                state = S_FLAGS;
                flags = 0;
            } else {
                bputc(buf, &n, size, c);
            }
        } else if (state == S_FLAGS) {
            switch (c) {
                case '#': flags |= F_ALTERNATE; break;
                case '0': flags |= F_ZEROPAD; break;
                case '-': flags |= F_LEFT; break;
                case ' ': flags |= F_SPACE; break;
                case '+': flags |= F_PLUS; break;
                case '\'':
                case 'I' : break; // not yet used
                default: fmt--; width = 0; state = S_WIDTH;
            }
        } else if (state == S_WIDTH) {
            if (c == '*') {
                width = va_arg(ap, int);
                if (width < 0) {
                    width = -width;
                    flags |= F_LEFT;
                }
            } else if (is_digit(c) && c > '0') {
                fmt--;
                width = get_atoi(&fmt);
            } else {
                fmt--;
                precision = -1;
                state = S_PRECIS;
            }
        } else if (state == S_PRECIS) {
            // Ignored for now, but skip it
            if (c == '.') {
                if (is_digit(*fmt)) {
                    precision = get_atoi(&fmt);

				} else if (*fmt == '*') {
                    precision = va_arg(ap, int);
				}
                precision = precision < 0 ? 0 : precision;
            } else {
                fmt--;
			}
            lflags = 0;
            state = S_LENGTH;
        } else if (state == S_LENGTH) {
            switch(c) {
                case 'h': lflags = lflags == L_CHAR ? L_SHORT : L_CHAR; break;
                case 'l': lflags = lflags == L_LONG ? L_LLONG : L_LONG; break;
                case 'L': lflags = L_DOUBLE; break;
                default: fmt--; state = S_CONV;
            }
        } else if (state == S_CONV) {
            if (c == 'd' || c == 'i' || c == 'o' || c == 'b' || c == 'u'
                    || c == 'x' || c == 'X') {
                if (lflags == L_LONG) {
                    num = va_arg(ap, long);

				} else if (lflags & (L_LLONG | L_DOUBLE)) {
                    num = va_arg(ap, long long);

				} else if (c == 'd' || c == 'i') {
                    num = va_arg(ap, int);

				} else {
                    num = (unsigned int) va_arg(ap, int);
				}

                base = 10;
                if (c == 'd' || c == 'i') {
                    flags |= F_SIGNED;
                } else if (c == 'x' || c == 'X') {
                    flags |= c == 'x' ? F_SMALL : 0;
                    base = 16;
                } else if (c == 'o') {
                    base = 8;
                } else if (c == 'b') {
                    base = 2;
                }
                fmt_int(buf, &n, size, num, base, width, flags);
            } else if (c == 'p') {
                num = (long) va_arg(ap, void *);
                base = 16;
                flags |= F_SMALL | F_ALTERNATE;
                fmt_int(buf, &n, size, num, base, width, flags);
            } else if (c == 's') {
                s = va_arg(ap, char *);
                if (!s)
                    s = "<NULL>";
                fmt_str(buf, &n, size, s, width, flags);
            } else if (c == 'c') {
                c = va_arg(ap, int);
                fmt_chr(buf, &n, size, c, width, flags);
            } else if (c == '%') {
                bputc(buf, &n, size, c);
            } else {
                bputc(buf, &n, size, '%');
                bputc(buf, &n, size, c);
            }
            state = S_DEFAULT;
        }
        if (c == 0) {
            break;
		}
    }
    n--;
    if (n < size) {
        buf[n] = 0;
	}
    else if (size > 0) {
        buf[size - 1] = 0;
	}

    return n;
}