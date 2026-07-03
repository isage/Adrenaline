#ifndef __XMBCTRL_UTILS_H__
#define __XMBCTRL_UTILS_H__

#include <stddef.h>
#include <pspkerneltypes.h>

// Extract specific string segments from "__DATE__" ("Mmm dd yyyy")
#define YEAR_STR  (__DATE__ + 7)
#define DAY_CHAR1   ((__DATE__[4] == ' ') ? '0' : (__DATE__[4]))
#define DAY_CHAR2   (__DATE__[5])

// Compile-time extraction of month index via character comparisons
#define MONTH_IS(c0, c1, c2) \
    (__DATE__[0] == (c0) && __DATE__[1] == (c1) && __DATE__[2] == (c2))

#define MONTH_STR \
    (MONTH_IS('J','a','n') ? "01" : \
     MONTH_IS('F','e','b') ? "02" : \
     MONTH_IS('M','a','r') ? "03" : \
     MONTH_IS('A','p','r') ? "04" : \
     MONTH_IS('M','a','y') ? "05" : \
     MONTH_IS('J','u','n') ? "06" : \
     MONTH_IS('J','u','l') ? "07" : \
     MONTH_IS('A','u','g') ? "08" : \
     MONTH_IS('S','e','p') ? "09" : \
     MONTH_IS('O','c','t') ? "10" : \
     MONTH_IS('N','o','v') ? "11" : \
     MONTH_IS('D','e','c') ? "12" : "00")


int utf8_to_unicode(wchar_t *dest, char *src);

#endif