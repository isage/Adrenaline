#include <string.h>
#include <psptypes.h>

static char *__strtok_context;

char tolower(char);
int strtol(const char*, char**, int);

char *strtok_r(char *string, const char *seps, char **context) {
	char *head;  // start of word
	char *tail;  // end of word

	// If we're starting up, initialize context
	if (string)
		*context = string;

	// Get potential start of this next word
	head = *context;
	if (head == NULL)
		return NULL;

	// Skip any leading separators
	while (*head && strchr(seps, *head)) {
		head++;
	}

	 // Did we hit the end?
	if (*head == 0) {
		// Nothing left
		*context = NULL;
		return NULL;
	}

	// skip over word
	tail = head;
	while (*tail && !strchr(seps, *tail)) {
		tail++;
	}

	// Save head for next time in context
	if (*tail == 0) {
		*context = NULL;
	} else {
		*tail = 0;
		tail++;
		*context = tail;
	}

	// Return current word
	return head;
}

char *strtok(char *str, const char *seps) {
	return strtok_r(str, seps, &__strtok_context);
}

void atob(const char *a0, int *a1) {
	char *p;
	*a1 = strtol(a0, &p, 10);
}

int strtol10(const char * str, int * res) {
	char * endptr = NULL;

	int ret = strtol(str, &endptr, 10);

	if (res) {
		*res = ret;
	}

	return ret;
}

SceSize strspn(const char *s, const char *accept) {
	const char *c;
	for (c = s; *c; c++) {
		if (!strchr(accept, *c))
			return c - s;
	}

	return c - s;
}

SceSize strcspn(const char *s, const char *reject) {
	const char *c;
	for (c = s; *c; c++) {
		if (strchr(reject, *c))
			return c - s;
	}

	return c - s;
}

void lowerString(char* orig, char* ret, int strSize) {
	int i=0;
	while (*(orig+i) && i<strSize-1){
		*(ret+i) = *(orig+i);
		if (*(orig+i) >= 'A' && *(orig+i) <= 'Z')
			*(ret+i) += 0x20;
		i++;
	}
	*(ret+i) = 0;
}

int strncasecmp(const char *s1, const char *s2, SceSize n) {
	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0) {
		return 0;
	}

	if (s1 == NULL || s2 == NULL) {
		return (int)s1 - (int)s2;
	}

	do {
		c1 = tolower(*p1);
		c2 = tolower(*p2);

		if (--n == 0 || c1 == '\0') {
			break;
		}

		++p1;
		++p2;
	} while (c1 == c2);

	return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2) {
	return strncasecmp(s1, s2, 2147483647);
}

char *strncat(char *dest, const char *src, SceSize count) {
	char *res = dest;
	dest += strlen(dest);
	while (count && *src) {
		count--;
		*dest++ = *src++;
	}
	*dest++ = 0;
	return res;
}

SceSize strncat_s(char *strDest, SceSize numberOfElements, const char *strSource, SceSize count) {
	SceSize rest;

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

SceSize strncpy_s(char *strDest, SceSize numberOfElements, const char *strSource, SceSize count) {
	if (!strDest || !strSource || numberOfElements == 0) {
		return 0;
	}

	strncpy(strDest, strSource, numberOfElements < count ? numberOfElements : count);
	strDest[numberOfElements - 1] = '\0';

	return strnlen(strDest, numberOfElements);
}