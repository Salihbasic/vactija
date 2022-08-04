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
   Subtracts the time (minutes and hours) from the first time
   with the second time. 

   It should be noted that this function deals with edge cases
   that generally should never happen within the context it is used,
   namely subtracting isha with maghrib time.
*/
void subtract_time(struct tm first, struct tm second, struct tm *res)
{
	
    int newmin;
    int newhrs = first.tm_hour;

    int submin = first.tm_min - second.tm_min;
    if (submin < 0) {

        /* Since first's minutes are less than second's, we now 
           take one hour away from first and add it up to first's minutes
           before continuing the subtraction. */

	newhrs -= 1;
        newmin = (first.tm_min + 60) - second.tm_min;

    } else {

        newmin = submin;

    }

    if (newhrs < 0) {

        /* In case first had 0 hours, we essentially would take a day,
           but since we are not dealin with any days, we simply
           set hours to 23 and move on. */
        
        newhrs = 23;

    }

    int subhrs = newhrs - second.tm_hour;
    if (subhrs < 0) {
        
        newhrs = (newhrs + 24) - second.tm_hour;

    } else {

        newhrs = subhrs;

    }
    
    res->tm_hour = newhrs;
    res->tm_min = newmin;

}

/*
   Divides the time (minutes and hours) from the provided time
   by a specified integer.

   This res WILL also feature seconds as remnants of minutes
   in order to avoid rounding and maximum accuracy for the purpose
   of the function (namely calculating exact middle / third
   of the night).
*/
void divide_time(struct tm first, int divisor, struct tm *res)
{
    
    double hrs = (double) first.tm_hour / (double) divisor;
    double hrsi;
    double hrsf = modf(hrs, &hrsi);

    double mins = (double) first.tm_min / (double) divisor;
    double minsi;
    double minsf = modf(mins, &minsi);

    /* Now multiply decimals of hours by 60 and add to minutes.
       Truncate since we need to deal with integral forms only. */
    double newmin = (trunc(hrsf * 60) + minsi);

    /* Now just round the seconds. */
    double newsec = round(minsf * 60);

    res->tm_hour = (int) trunc(hrsi);
    res->tm_min = (int) trunc(newmin);
    res->tm_sec = (int) trunc(newsec);

}

/*
   Takes a time string of the format %H:%M (HH:MM) and parses it
   into a struct tm.
   struct tm will be initialised to 0 for every value other
   than hours and minutes.
*/
void parse_timestr(const char *str, struct tm *res) 
{

    int coli = 0; /* Index of ':' in the string */
    while (str[coli] != '\0') {

        if (str[coli] == ':') {
            break;
        }

        /* ':' should be 2nd or 3rd character */
        if (coli > 2) {
            coli = -1;
            break;
        }
        

        coli++;

    }

    if (coli == -1) {

        printf("Invalid string passed to parse_timestr!\n");
        exit(EXIT_FAILURE);

    }

    int len = strlen(str);
    if (coli == 1 && len != 4) {

        printf("Invalid string passed to parse_timestr!\n");
        exit(EXIT_FAILURE);

    }

    if (coli == 2 && len != 5) {

        printf("Invalid string passed to parse_timestr!\n");
        exit(EXIT_FAILURE);

    }

    /* 
        Doing this to avoid depending on strptime which is not
        always supported.
       
        Ignoring end pointer.
    */
    int hours = strtol(str, NULL, 10);
    int minutes = strtol((str + (coli + 1)), NULL, 10);

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
