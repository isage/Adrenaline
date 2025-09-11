#ifndef __virtualsfo__
#define __virtualsfo__

void virtual_sfo_init();
int virtual_sfo_size();
unsigned char* virtual_sfo_get();
void sfo_set_int_param(char *key, int value);
void sfo_set_string_param(char *key, char* value);
int sfo_get_int_param(char *key);
char* sfo_get_string_param(char *key);

#endif
