#ifndef CACHEFILE_H
#define CACHEFILE_H

int cache_exists(const char *path);
int cache_outdated(const char *path);

void write_cache(const char *path, const char *json);
char *read_cache(const char *path);

#endif