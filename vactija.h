#ifndef VACTIJA_H
#define VACTIJA_H

#include <time.h>

#define VAKTIJA_API_URL "https://api.vaktija.ba/"
#define PRAYER_TIME_NUM 6

struct vaktija {

    char *prayers[PRAYER_TIME_NUM];

    char *location;

    char *date_greg;

    char *date_hijra;

};

int cache_exists(const char *path);
int cache_outdated(const char *path);

void download_latest(const char *path);

#endif