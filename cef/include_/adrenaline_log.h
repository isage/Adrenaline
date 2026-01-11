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

static int itostr(char *buf, int in_data, int base, int upper, int sign) {
	int res, len, i;
	unsigned int data;
	char *str;

	if (base==10 && sign && in_data<0) {
		data = -in_data;
	} else {
		data = in_data;
	}

	str = buf;
	do {
		res = data%base;
		data = data/base;
		if(res<10){
			res += '0';
		}else{
			if(upper){
				res += 'A'-10;
			}else{
				res += 'a'-10;
			}
		}
		*str++ = res;
	}while(data);
	len = str-buf;

	/* reverse digital order */
	for(i=0; i<len/2; i++){
		res = buf[i];
		buf[i] = buf[len-1-i];
		buf[len-1-i] = res;
	}

	return len;
}

/*
 * _vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 */
#define OUT_C(c) \
	if(str<end){ \
		*str++ = (c); \
	} else { \
		goto exit; \
	}

static char digital_buf[32];
int _vsnprintf(char *buf, int size, char *fmt, va_list args) {
	char ch, *s, *str, *end, *sstr;
	int zero_pad, left_adj, add_sign, field_width, sign;
	int i, base, upper, len;


	if (!buf || !fmt ||!size) {
		return 0;
	}

	str = buf;
	end = buf+size;

	while (*fmt) {
		if(*fmt!='%'){
			OUT_C(*fmt++);
			continue;
		}

		/* skip '%' */
		sstr = fmt;
		fmt++;

		/* %% */
		if(*fmt=='%'){
			OUT_C(*fmt++);
			continue;
		}

		/* get flag */
		zero_pad = ' ';
		left_adj = 0;
		add_sign = 0;
		while ((ch=*fmt)) {

			if(*fmt=='0'){
				zero_pad = '0';
			}else if(*fmt=='-'){
				left_adj = 1;
			}else if(*fmt=='#'){
			}else if(*fmt==' '){
				if(add_sign!='+')
					add_sign = ' ';
			}else if(*fmt=='+'){
				add_sign = '+';
			}else{
				break;
			}
			fmt++;
		}

		/* get field width: m.n */
		field_width = 0;
		/* get m */
		while (*fmt && *fmt>'0' && *fmt<='9') {
			field_width = field_width*10+(*fmt-'0');
			fmt++;
		}
		if (*fmt && *fmt=='.') {
			fmt++;
			/* skip n */
			while(*fmt && *fmt>'0' && *fmt<='9'){
				fmt++;
			}
		}

		/* get format char */
		upper = 0;
		base = 0;
		sign = 0;
		len = 0;
		s = digital_buf;
		while ((ch=*fmt)) {
			fmt++;
			switch(ch){
			/* hexadecimal */
			case 'p':
			case 'X':
				upper = 1;
			case 'x':
				base = 16;
				break;

			/* decimal */
			case 'd':
			case 'i':
				sign = 1;
			case 'u':
				base = 10;
				break;

			/* octal */
			case 'o':
				base = 8;
				break;

			/* character */
			case 'c':
				digital_buf[0] = (unsigned char) va_arg(args, int);
				len = 1;
				break;

			/* string */
			case 's':
				s = va_arg(args, char *);
				if(!s) s = "<NUL>";
				len = strlen(s);
				break;

			/* float format, skip it */
			case 'e': case 'E': case 'f': case 'F': case 'g': case 'G': case 'a': case 'A':
				va_arg(args, double);
				s = NULL;
				break;

			/* length modifier */
			case 'l': case 'L': case 'h': case 'j': case 'z': case 't':
				/* skip it */
				continue;

			/* bad format */
			default:
				s = sstr;
				len = fmt-sstr;
				break;
			}
			break;
		}

		if(base){
			i = va_arg(args, int);
			if(base==10 && sign){
				if(i<0){
					add_sign = '-';
				}
			}else{
				add_sign = 0;
			}

			len = itostr(digital_buf, i, base, upper, sign);
		} else {
			zero_pad = ' ';
			add_sign = 0;
		}

		if(s) {
			if (len>=field_width) {
				field_width = len;
				if(add_sign)
					field_width++;
			}
			for (i=0; i<field_width; i++) {
				if(left_adj){
					if(i<len){
						OUT_C(*s++);
					}else{
						OUT_C(' ');
					}
				} else {
					if (add_sign && (zero_pad=='0' || i==(field_width-len-1))) {
						OUT_C(add_sign);
						add_sign = 0;
						continue;
					}
					if (i<(field_width-len)) {
						OUT_C(zero_pad);
					} else {
						OUT_C(*s++);
					}
				}
			}
		}
	}

	OUT_C(0);

exit:
	return str-buf;
}

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
	va_list args;

	if ( 0 )
		return 0;

	u32 k1 = pspSdkSetK1(0);

	va_start(args, fmt);
	int printed_len = _vsnprintf(log_buf, sizeof(log_buf), fmt, args);
	va_end(args);
	printed_len--;
	appendToMemoryLog(printed_len);

	pspSdkSetK1(k1);

	return printed_len;
}

static int __logmsg(char *fmt, ...) {
	va_list args;
	int printed_len;

	u32 k1 = pspSdkSetK1(0);

	if (0 == isCpuIntrEnabled()) {
		// interrupt disabled, let's do the work quickly before the watchdog bites
		va_start(args, fmt);
		printed_len = _vsnprintf(log_buf, sizeof(log_buf), fmt, args);
		va_end(args);
		printed_len--;
		appendToMemoryLog(printed_len);
	} else {
		logLock();
		va_start(args, fmt);
		printed_len = _vsnprintf(log_buf, sizeof(log_buf), fmt, args);
		va_end(args);
		printed_len--;
		logOutput(printed_len);
		logUnlock();
	}

	pspSdkSetK1(k1);

	return printed_len;
}

int _logmsg(char *fmt, ...) {
	extern SceModuleInfo module_info;
	int res = __logmsg("[%s v%d.%d]: ", module_info.modname, module_info.modversion[1], module_info.modversion[0]);

	va_list args;
	int printed_len;

	u32 k1 = pspSdkSetK1(0);

	if (0 == isCpuIntrEnabled()) {
		// interrupt disabled, let's do the work quickly before the watchdog bites
		va_start(args, fmt);
		printed_len = _vsnprintf(log_buf, sizeof(log_buf), fmt, args);
		va_end(args);
		printed_len--;
		appendToMemoryLog(printed_len);
	} else {
		logLock();
		va_start(args, fmt);
		printed_len = _vsnprintf(log_buf, sizeof(log_buf), fmt, args);
		va_end(args);
		printed_len--;
		logOutput(printed_len);
		logUnlock();
	}

	pspSdkSetK1(k1);

	return res + printed_len;
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

#endif // DEBUG

#endif // _ADRENALINE_LOG_IMPL_