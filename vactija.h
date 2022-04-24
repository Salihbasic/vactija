#ifndef VACTIJA_H
#define VACTIJA_H

#include <time.h>

#define VAKTIJA_API_URL "https://api.vaktija.ba/vaktija/v1/"

/* 
    Number of tokens (JSON elements) in vaktija JSON file.
    Essentially determined experimentally.
*/
#define VACTIJA_JSMN_TOKENS 15

/*
    Array sizes as they appear in the vaktija JSON file.
*/
#define PRAYER_TIME_NUM 6
#define DATUM_NUM 2

struct vaktija {

    char **prayers;

    char *location;

    char **dates;

};

char *download_vaktija(const char *loc, const char *date);

struct vaktija *parse_data(const char *json);

int next_vakat(const struct vaktija *vaktija, struct tm time);

#endif