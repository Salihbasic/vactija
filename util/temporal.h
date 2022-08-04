#ifndef TEMPORAL_H
#define TEMPORAL_H

#include <time.h>
#include <math.h>

int compare_date(struct tm first, struct tm second);
int compare_time(struct tm first, struct tm second);

void subtract_time(struct tm first, struct tm second, struct tm *res);
void divide_time(struct tm first, int divisor, struct tm *res);

void parse_timestr(const char *str, struct tm *res);

#endif
