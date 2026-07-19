/*
	Adrenaline Log
	Copyright (C) 2025, GrayJack
	Copyright (C) 2021, PRO CFW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* THIS IS A HEADER-ONLY LIBRARY
 *
 * This means that you have to include the header ONLY ONCE PER PROJECT with
 * `_ADRENALINE_LOG_IMPL_` macro defined. Like below:
 *
 * <code>
 * #define _ADRENALINE_LOG_IMPL_
 * #include <adrenaline_log.h>
 * </code>
 *
 * Any subsequent usage in the same project files can be just the simple include.
 *
 * <code>
 * #include <adrenaline_log.h>
 * </code>
 *
 * The `logmsg` works like a `printf`. It supports the following formats:
 *
 *  - %c
 *  - %s
 *  - %p
 *  - %[[0]n]x
 *  - %[[0]n]X
 *  - %[[0]n]d
 *
 * This library will not output anything unless `DEBUG` is defined (you can
 * define it passing `-DDEBUG` or `-DDEBUG=<level>` to `CFLAGS`).
 *
 * This library also has helper definition to log on different debug levels
 * (`logmsg2`, `logmsg3`, `logmsg4`) that will not output unless `DEBUG` is
 * defined with a value equal or bigger than the level.
 */

#ifndef __ADRENALINE_LOG_H__
#define __ADRENALINE_LOG_H__

#ifdef DEBUG
// Initialize log
int _logInit(const char* filename);

// Log info
int _logmsg(char *fmt, ...)__attribute__((format (printf, 1, 2)));

int _logCached(char *fmt, ...) __attribute__((format(printf, 1, 2)));

// Output all info remaining in the memory to file
int _logSync(void);

// Initialize log
#define logInit(output) {_logInit(output);};

// Log info - Level 1
#define logmsg(fmt, ...) {_logmsg(fmt, ##__VA_ARGS__);};

#if DEBUG >= 2
// Log info - Level 2
#define logmsg2(fmt, ...) {_logmsg(fmt, ##__VA_ARGS__);};
#else
// Log info - Level 2
#define logmsg2(fmt, ...);
#endif // DEBUG >= 2

#if DEBUG >= 3
// Log info - Level 3
#define logmsg3(fmt, ...) {_logmsg(fmt, ##__VA_ARGS__);};
#else
// Log info - Level 3
#define logmsg3(fmt, ...);
#endif // DEBUG >= 3

#if DEBUG >= 4
// Log info - Level 4
#define logmsg4(fmt, ...) {_logmsg(fmt, ##__VA_ARGS__);};
#else
// Log info - Level 4
#define logmsg4(fmt, ...);
#endif // DEBUG >= 4

// Output all info remaining in the memory to file
#define logSync() {_logSync();};

#define logCached(fmt, ...) {_logCached(fmt, ##__VA_ARGS__);};
#else
// Initialize log
#define logInit(output);

// Log info - Level 1
#define logmsg(fmt, ...);
// Log info - Level 2
#define logmsg2(fmt, ...);
// Log info - Level 3
#define logmsg3(fmt, ...);
// Log info - Level 4
#define logmsg4(fmt, ...);

// Output all info remaining in the memory to file
#define logSync();
#define logCached(fmt, ...);
#endif // DEBUG

#endif //__ADRENALINE_LOG_H__

#ifdef _ADRENALINE_LOG_IMPL_

#ifdef DEBUG

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <pspmoduleinfo.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Lock log
void logLock(void);

// Unlock log
void logUnlock(void);

struct pthread_mlock_t {
	volatile unsigned long l;
	unsigned int c;
	int thread_id;
};

typedef struct pthread_mlock_t MLOCK_T;

static MLOCK_T lock;

#ifdef _ADRENALINE_LOG_USE_PAF_

int sce_paf_private_vsnprintf(char *buf, int size, char *fmt, va_list args);
char *sce_paf_private_strcpy(char *, const char *);
void *sce_paf_private_memset(void *, char, int);
void *sce_paf_private_memcpy(void *, void *, int);
char *sce_paf_private_strcat(char *, const char *);
int sce_paf_private_snprintf(char *, SceSize, const char *, ...);

#define _vsnprintf sce_paf_private_vsnprintf
#define strcpy sce_paf_private_strcpy
#define memset sce_paf_private_memset
#define memcpy sce_paf_private_memcpy
#define strcat sce_paf_private_strcat
#define snprintf sce_paf_private_snprintf

#else

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
int _vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
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

    return n+1;
}

#endif // _ADRENALINE_LOG_USE_PAF_

static char log_buf[256];
static const char *log_output_fn;
static char default_path[256] = {0};

static char log_memory_log[1024*4] __attribute__((aligned(64)));
static char *log_memory_log_ptr = log_memory_log;

static void flushLogMemoryLog(int fd, int kout) {
	sceIoWrite(fd, log_memory_log, log_memory_log_ptr - log_memory_log);
	int ret = sceIoWrite(kout, log_memory_log, log_memory_log_ptr - log_memory_log);

	if (ret >= 0) {
		memset(log_memory_log, 0, sizeof(log_memory_log));
		log_memory_log_ptr = log_memory_log;
	}
}

static void appendToMemoryLog(int printed_len) {
	if (log_memory_log_ptr + printed_len < log_memory_log + sizeof(log_memory_log)) {
		memcpy(log_memory_log_ptr, log_buf, printed_len);
		log_memory_log_ptr += printed_len;
	}
}

static int logOpenOutput(void) {
	if (log_output_fn == NULL) {
		strcpy(default_path, "ms0:/");
		strcat(default_path, "LOG.TXT");
		log_output_fn = default_path;
	}

	int fd = sceIoOpen(log_output_fn, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);

	return fd;
}

static void logOutput(int printed_len) {
	int kout = sceKernelStdout();
	sceIoWrite(kout, log_buf, printed_len);
	int fd = logOpenOutput();

	if (fd >= 0) {
		if (log_memory_log_ptr > log_memory_log) {
			flushLogMemoryLog(fd, kout);
		}

		sceIoWrite(fd, log_buf, printed_len);
		sceIoClose(fd);
	} else {
		appendToMemoryLog(printed_len);
	}

	sceKernelDelayThread(10000);
}

static int isCpuIntrEnabled(void) {
	int ret;

	__asm__ volatile ("mfic    %0, $0\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			: "=r"(ret)
			);

	return ret;
}

int _logCached(char *fmt, ...) {
	extern SceModuleInfo module_info;
	va_list args;

	if ( 0 )
		return 0;

	u32 k1 = pspSdkSetK1(0);

	int printed_len = snprintf(log_buf, sizeof(log_buf), "[%s v%d.%d]: ", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
	va_start(args, fmt);
	printed_len = _vsnprintf(log_buf+printed_len, sizeof(log_buf)-printed_len, fmt, args);
	va_end(args);
	#ifndef _ADRENALINE_LOG_USE_PAF_
	printed_len--;
	#endif
	appendToMemoryLog(printed_len);

	pspSdkSetK1(k1);

	return printed_len;
}

int _logmsg(char *fmt, ...) {
	extern SceModuleInfo module_info;
	va_list args;
	int printed_len;

	u32 k1 = pspSdkSetK1(0);

	if (0 == isCpuIntrEnabled()) {
		// interrupt disabled, let's do the work quickly before the watchdog bites
		printed_len = snprintf(log_buf, sizeof(log_buf), "[%s v%d.%d]: ", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
		va_start(args, fmt);
		printed_len += _vsnprintf(log_buf+printed_len, sizeof(log_buf)-printed_len, fmt, args);
		va_end(args);
		#ifndef _ADRENALINE_LOG_USE_PAF_
		printed_len--;
		#endif
		appendToMemoryLog(printed_len);
	} else {
		logLock();
		printed_len = snprintf(log_buf, sizeof(log_buf), "[%s v%d.%d]: ", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
		va_start(args, fmt);
		printed_len += _vsnprintf(log_buf+printed_len, sizeof(log_buf)-printed_len, fmt, args);
		va_end(args);
		#ifndef _ADRENALINE_LOG_USE_PAF_
		printed_len--;
		#endif
		logOutput(printed_len);
		logUnlock();
	}

	pspSdkSetK1(k1);

	return printed_len;
}

static unsigned long InterlockedExchange(unsigned long volatile *dst, unsigned long exchange) {
	unsigned int flags = pspSdkDisableInterrupts();
	unsigned long origvalue = *dst;

	*dst = exchange;
	pspSdkEnableInterrupts(flags);

	return origvalue;
}

static inline int psp_mutex_lock(MLOCK_T *s) {
	for (;;) {
		if (s->l != 0) {
			if (s->thread_id == sceKernelGetThreadId()) {
				++s->c;
				return 0;
			}
		} else {
			if (!InterlockedExchange(&s->l, 1)) {
				s->thread_id = sceKernelGetThreadId();
				s->c = 1;
				return 0;
			}
		}

		sceKernelDelayThread(1000);
	}

	return 0;
}

static inline void psp_mutex_unlock(MLOCK_T *s) {
	if (--s->c == 0) {
		s->thread_id = 0;
		InterlockedExchange(&s->l, 0);
	}
}

void logLock(void) {
	psp_mutex_lock(&lock);
}

void logUnlock(void) {
	psp_mutex_unlock(&lock);
}

int _logInit(const char *output) {
	static char dynamic_path[256];

	MLOCK_T *s = &lock;

	s->l = 0;
	s->c = 0;
	s->thread_id = sceKernelGetThreadId();

	if (output != NULL)
	{
		strcpy(dynamic_path, output);
		log_output_fn = dynamic_path;
	} else {
		strcpy(dynamic_path, "ms0:log.txt");
		log_output_fn = dynamic_path;
	}

	return 0;
}

int _logSync(void) {
	int kout = sceKernelStdout();
	int fd = logOpenOutput();

	if (fd >= 0) {
		if (log_memory_log_ptr > log_memory_log) {
			flushLogMemoryLog(fd, kout);
		}

		sceIoClose(fd);
		sceKernelDelayThread(10000);
	}

	return 0;
}

#ifdef _ADRENALINE_LOG_USE_PAF_
#undef _vsnprintf
#undef strcpy
#undef memset
#undef memcpy
#undef strcat
#undef snprintf
#endif // _ADRENALINE_LOG_USE_PAF_

#endif // DEBUG

#endif // _ADRENALINE_LOG_IMPL_