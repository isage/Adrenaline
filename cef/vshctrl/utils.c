#include <pspsysmem.h>
#include <string.h>
#include "utils.h"

void* vsh_malloc(size_t size) {
	SceUID uid = sceKernelAllocPartitionMemory(2, "", PSP_SMEM_High, size+sizeof(u32), NULL);
	int* ptr = sceKernelGetBlockHeadAddr(uid);
	if (ptr){
		ptr[0] = uid;
		return &(ptr[1]);
	}
	return NULL;
}

void vsh_free(void* ptr) {
	if (ptr){
		int* p = ptr;
		int uid = p[-1];
		sceKernelFreePartitionMemory(uid);
	}
}

size_t strncat_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {
	size_t rest;

	if (!strDest || !strSource || numberOfElements == 0) {
		return 0;
	}

	rest = numberOfElements - strnlen(strDest, numberOfElements);

	if (rest == 0) {
		return 0;
	}

	rest--;
	strncat(strDest, strSource, rest < count ? rest : count);

	return strnlen(strDest, numberOfElements);
}

size_t strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {
	if (!strDest || !strSource || numberOfElements == 0) {
		return 0;
	}

	strncpy(strDest, strSource, numberOfElements < count ? numberOfElements : count);
	strDest[numberOfElements - 1] = '\0';

	return strnlen(strDest, numberOfElements);
}

int tolower(int s)
{
	if((s >= 'A') && (s <= 'Z'))
		s = 'a' + (s - 'A');

	return s;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0)
		return 0;

	if (s1 == NULL || s2 == NULL) {
		return (int)s1 - (int)s2;
	}

	do {
		c1 = tolower(*p1);
		c2 = tolower(*p2);

		if (--n == 0 || c1 == '\0')
			break;

		++p1;
		++p2;
	} while (c1 == c2);

	return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2) {
	return strncasecmp(s1, s2, (size_t)-1);
}