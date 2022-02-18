#ifndef TEMPORAL_H
#define TEMPORAL_H

#include <time.h>

int compare_date(struct tm first, struct tm second);
int compare_time(struct tm first, struct tm second);

void parse_timestr(const char *str, struct tm *res);

#endif