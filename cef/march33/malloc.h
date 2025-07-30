#ifndef ____MALLOC_H____

#define ____MALLOC_H____

int  mallocinit();
#define malloc oe_malloc
#define free  oe_free
void *oe_malloc(size_t size);
void oe_free(void *p);
int  mallocterminate();

#endif

