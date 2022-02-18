#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "temporal.h"

/* 
   Compares the date (year and day of the year) of the first and 
   second tm provided. 
   If first is later than second, returns 1.
   If first is the same as second, returns 0.
   If first is before second, returns -1.
*/
int compare_date(struct tm first, struct tm second) 
{

    int year = first.tm_year - second.tm_year;
    int yday = first.tm_yday - second.tm_yday;

    if (year > 0) {
        return 1;
    } else if (year < 0) {
        return -1;
    } else {
        
        if (yday > 0) {
            return 1;
        } else if (yday < 0) {
            return -1;
        } else {
            return 0;
        }

    }

}

/* 
   Compares the time (minutes and hours) of the first and 
   second tm provided. 
   If first is later than second, returns 1.
   If first is the same as second, returns 0.
   If first is before second, returns -1.
*/
int compare_time(struct tm first, struct tm second)
{

    int hour = first.tm_hour - second.tm_hour;
    int minutes = first.tm_min - second.tm_min;

    if (hour > 0) {
        return 1;
    } else if (hour < 0) {
        return -1;
    } else {
        
        if (minutes > 0) {
            return 1;
        } else if (minutes < 0) {
            return -1;
        } else {
            return 0;
        }

    }

}

/*
   Takes a time string of the format %H:%M (HH:MM) and parses it
   into a struct tm.
   struct tm will be initialised to 0 for every value other
   than hours and minutes.
*/
void parse_timestr(const char *str, struct tm *res) 
{

    if (strlen(str) != 5) {
        printf("Invalid string passed to parse_timestr!\n");
        exit(EXIT_FAILURE);
    }

    /* Doing this to avoid depending on strptime which is not
       in the stdlib. 
       
       Ignoring end pointer.
    */
    int hours = strtol(str, NULL, 10);
    int minutes = strtol((str + 3), NULL, 10);

    res->tm_sec = 0;
    res->tm_min = abs(minutes);
    res->tm_hour = abs(hours);
    res->tm_mday = 0;
    res->tm_mon = 1;
    res->tm_year = 0;
    res->tm_wday = 0;
    res->tm_year = 0;
    res->tm_isdst = -1; /* = unspecified, per Linux manual */

}