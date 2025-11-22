/*
	SysclibForUser
	Copyright (C) 2025, GrayJack

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

#ifndef __SYSCLIBFORUSER__
#define __SYSCLIBFORUSER__

#include <psptypes.h>

/**
 * Helper function to allocate memory on P2 (user memory).
 *
 * @param size - amount of bytes to allocate.
 *
 * @returns pointer to allocated buffer, or NULL on error.
 */
void *user_malloc(SceSize size);
/**
 * Helper function to allocate aligned memory on P2 (user memory).
 *
 * @param align - The alignment
 * @param size - amount of bytes to allocate.
 *
 * @returns pointer to allocated buffer, or NULL on error.
 */
void *user_memalign(SceSize align, SceSize size);
/**
 * Deallocate memory allocated by user_malloc.
 *
 * @param ptr - pointer to the allocated memory.
 */
void user_free(void *ptr);

void *malloc(SceSize size);
void *memalign(SceSize align, SceSize size);
void free(void *ptr);

// ARK CFW compat
void oe_free(void *ptr);

void lowerString(char *orig, char *ret, int strSize);
int strncasecmp(const char *s1, const char *s2, SceSize n);
int strcasecmp(const char *s1, const char *s2);

void bcopy(const void *src, void *dest, SceSize len);
int bcmp(const void *src, const void *other, SceSize len);
void bzero(void *str, SceSize len);

void *memchr(const void *src, int byte, SceSize len);
int memcmp(const void *src, const void *other, SceSize len);
void *memmove(void *dest, const void *src, SceSize len);
void *memcpy(void *dest, const void *src, SceSize len);
void *memcpy(void *dest, const void *src, SceSize len);
void *memset(void *src, int val, SceSize len);

int strncmp(const char *str1, const char *str2, SceSize max_size);
char *strstr(const char *str, const char *pattern);
char *strtok_r(char *str, const char *delim, char **state);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, SceSize count);
SceSize strncat_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);
char *strrchr(const char *str, int ch);
SceSize strlen(const char *str);
SceSize strnlen(const char *str, SceSize max_size);
SceSize strspn(const char *str, const char *char_set);
SceSize strcspn(const char *str, const char *char_set);
char *strchr(const char *str, int ch);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, SceSize max_size);
SceSize strncpy_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);
long strtol(const char* str, char** str_end, int base);
unsigned long strtoul(const char* str, char** str_end, int base);
char *strpbrk(const char *dest, const char *breakset);

int toupper(int ch);
int tolower(int ch);

void atob(const char *a0, int *a1);

int sprintf(char *str, const char *format, ...);
int snprintf(char *str, SceSize n, const char * format, ... );
#endif