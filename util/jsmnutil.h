#include <stddef.h>

#define JSMN_HEADER
#include "../jsmn/jsmn.h"

int jsmn_compare(const char *js, jsmntok_t *token, const char *str);

jsmntok_t *find_by_key(const char *js, const char *key, jsmntok_t tokens[], size_t toklen);
int find_idx_by_key(const char *js, const char *key, jsmntok_t tokens[], size_t toklen);

char *get_simple(const char *js, jsmntok_t *token);
char **get_array(const char *js, int idx, jsmntok_t tokens[], size_t arrlen);

void free_array(char **arr, size_t arrlen);
