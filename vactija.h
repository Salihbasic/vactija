#ifndef VACTIJA_H
#define VACTIJA_H

#include <time.h>

#define VAKTIJA_API_URL "https://api.vaktija.ba/"
#define PRAYER_TIME_NUM 6
#define JSON_PARSER_STORELENGTH 64

struct vaktija {

    char *prayers[PRAYER_TIME_NUM];

    char *location;

    char *date_greg;

    char *date_hijra;

};

int cache_exists(const char *path);
int cache_outdated(const char *path);

void download_latest(const char *path);

void read_cache(const char *path, struct vaktija *result);

#endif