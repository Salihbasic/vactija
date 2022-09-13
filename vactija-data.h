#ifndef VACTIJA_DATA_H
#define VACTIJA_DATA_H

/*
    In case the system does not support or process UTF-8 data, this
    macro will, given the USE_ASCII_ONLY option, provide strings
    without Bosnian diacritics, compatible with ASCII.
*/
#ifdef USE_ASCII_ONLY
    #define WITH_ASCII(normal, ascii) ascii
#else
    #define WITH_ASCII(normal, ascii) normal
#endif

/*
    Holds vakat information for each day of the year in the format of
    [month][day][vakat-id].

    All data is in seconds of the day, and it should be first callibrated
    with data from differences array.
*/
extern const int vakats[12][31][6];

/*
    Holds information for vakat differences for each month of the year
    in the format of [location-id][month][vakat-id].

    Differences are simply the +/- seconds/minutes for each particular
    location due to geographic distance involved.
*/
extern const int differences[118][12][6];

/*
    Names of every vakat in their daily order.
*/
extern const char *vakat_names[6];

/*
    Location names coinciding with the specific location ID.
    
    That is why their order in this list must never be changed
    without also changing the order in the differences array.
*/
extern const char *locations[118];

#endif