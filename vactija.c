#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>

#define JSMN_HEADER
#include "jsmn/jsmn.h"
#include "vactija.h"
#include "util/temporal.h"
#include "util/jsmnutil.h"
#include "util/cachefile.h"

#ifndef vactija_error
/* 
    Errcode needs to be equal to whetever errno value
    the error is supposed to display.
*/
#define vactija_error(errcode)                                        \
    char *errstr = strerror(errcode);                                 \
    printf("Err: %s\n", errstr);                                      \
    exit(EXIT_FAILURE)          
#endif 

/*
    Allocates a new vaktija to the heap. All pointers
    inside it are unassigned.

    Has to be freed manually once it is no longer useful.
*/
struct vaktija *create_vaktija()
{

    struct vaktija *v = malloc(sizeof *v);

    if (v == NULL) {

        printf("Could not allocate enough memory to store vaktija data!\n");

        int errcode = errno;

        char *errstr = strerror(errcode);
        printf("Err: %s\n", errstr);
        exit(EXIT_FAILURE);

    }

    return v;

}

/*
    Frees a vaktija from the heap under the assumption that it is not only
    partially initialised (i.e attempts to free all elements of the struct
    as well, which is the general scenario so far).
*/
void delete_vaktija(struct vaktija *vaktija)
{

	free(vaktija->location);
	free_array(vaktija->dates, DATUM_NUM);
	free_array(vaktija->prayers, PRAYER_TIME_NUM);
	
	free(vaktija);

}

struct mem_struct {
    char *mem;
    size_t size;
};

static size_t write_callback(char *contents, size_t size, size_t nmemb, char *userp)
{

    size_t realsize = size * nmemb;
    struct mem_struct *memory = (struct mem_struct *) userp;

    char *ptr = realloc(memory->mem, memory->size + (realsize + 1));
    if (ptr == NULL) {
        
        int errcode = errno;
        printf("Could not allocate enough memory to store JSON data. Download aborted!\n");
        vactija_error(errcode);

    }

    memory->mem = ptr;
    memcpy(&(memory->mem[memory->size]), contents, realsize);
    memory->size += realsize;
    memory->mem[realsize] = 0;

    return realsize;

}

/*
    Downloads the vaktija JSON data from the API based on provided parameters.

    loc holds the location ID for a particular city, which can be found
    on the API examples site. If it is not provided (i.e NULL is passed), then
    it takes the default value of 0.

    date holds the date, which can be provided in the format 'year/month/day',
    where month and day can be left off (in which case it will take current day's values
    in their place). If it is not present (i.e NULL is passed), then it shall
    download the particular vaktija data for the current day.

    Examples page: https://api.vaktija.ba/vaktija/v1
*/
char *download_vaktija(const char *loc, const char *date)
{

    CURL *curl = curl_easy_init();

    if (curl) {

        char *url;

        size_t apilen = strlen(VAKTIJA_API_URL);
        size_t loclen = strlen(loc);

        if (date != NULL) {

            size_t datelen = strlen(date);
            url = malloc(sizeof *url * (apilen + loclen + datelen + 2)); /* for extra / */

        } else {

            url = malloc(sizeof *url * (apilen + loclen + 1));

        }

        if (url == NULL) {

            int errcode = errno;
            printf("Could not allocate enough memory to store URL. Download aborted!\n");
            vactija_error(errcode);

        }
        url[0] = '\0';

        /* VAKTIJA_API_URL ends with /, so we can just append location ID */
        strncat(url, VAKTIJA_API_URL, apilen);
        strncat(url, loc, loclen);        
    
        if (date != NULL) {

            strncat(url, "/", 1); /* since ID doesn't end with / */
            strncat(url, date, strlen(date));

        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        /* callback will reallocate enough memory */
        struct mem_struct dw_json;
        dw_json.mem = malloc(sizeof(char *) * 1);
        dw_json.size = 0;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dw_json);

        char errbuf[CURL_ERROR_SIZE];
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        errbuf[0] = 0;

        CURLcode result = curl_easy_perform(curl);

        if (result == CURLE_OK) {
            
            curl_easy_cleanup(curl);

            return dw_json.mem;
            
        } else {
            
            size_t errlen = strlen(errbuf); 
            printf("Encountered an error with libcurl!\n");
            printf("libcurl error code: %d\n", result);
            
            if (errlen) {
            
                printf("libcurl error: %s%s", errbuf, 
                        (errbuf[errlen - 1] != '\n' ? "\n" : ""));
            
            } else {
            
                printf("libcurl (generic) error: %s\n", curl_easy_strerror(result));
            
            }
            
            curl_easy_cleanup(curl);
            exit(EXIT_FAILURE);

        }
        
    } else {
    
        printf("Could not initialise libcurl handle!\n");

        exit(EXIT_FAILURE);

    }

}

/*
    Uses jsmn by Serge Zaitsev to parse/tokenise the JSON from JSON
    string and then processes it with functions from jsmnutil.

    This function is highly dependent on the current JSON structure
    of the API, which can be changed in the future, therefore this
    function must be updated in such cases.

    (All values for the structure were determined experimentally.
    jsmn is a minimalist library, and while a fully generalised
    parser could be built on top of it, it'd be a waste of time
    for this project, which is why only the barebones have been
    done here.)
*/
struct vaktija *parse_data(const char *json)
{

    jsmn_parser pars;
    jsmntok_t tok[VACTIJA_JSMN_TOKENS]; /* Based on current JSON file structure */

    jsmn_init(&pars);
    int result = jsmn_parse(&pars, json, strlen(json), tok, VACTIJA_JSMN_TOKENS);

    if (result < 0) {
        
        char *errstr;
        switch (result) {
        
        case JSMN_ERROR_NOMEM:
            errstr = "JSMN_ERROR_NOMEM";

        case JSMN_ERROR_INVAL:
            errstr = "JSMN_ERROR_INVALID_JSON";
        
        case JSMN_ERROR_PART:
            errstr = "JSMN_ERROR_NOT_FULL_JSON_STRING";

        }

        printf("Encountered an error while parsing vaktija JSON (%s)!\n", errstr);
        exit(EXIT_FAILURE);

    }

    if (result != 17 && result != 23) {

        printf("Parsed %d JSON tokens (expected: 17 or 23)!\n", result);
        exit(EXIT_FAILURE);

    }

    jsmntok_t *loctok = find_by_key(json, "lokacija", tok, VACTIJA_JSMN_TOKENS);
    char *location = get_simple(json, loctok);

    int dati = find_idx_by_key(json, "datum", tok, VACTIJA_JSMN_TOKENS);
    char **datums = get_array(json, dati, tok, DATUM_NUM);

    int vakati = find_idx_by_key(json, "vakat", tok, VACTIJA_JSMN_TOKENS);
    char **vakats = get_array(json, vakati, tok, PRAYER_TIME_NUM);

    struct vaktija *v = create_vaktija();
    v->location = location;
    v->dates = datums;
    v->prayers = vakats;

    return v;

}

/*
    Returns the index of the next vakat based on provided time.
*/
int next_vakat(const struct vaktija *vaktija, struct tm time)
{

    for (int i = 0; i < PRAYER_TIME_NUM; i++) {

        char *vakatstr = vaktija->prayers[i];
        struct tm vakattime;
        parse_timestr(vakatstr, &vakattime);

        if (compare_time(time, vakattime) < 0) {

            return i;

        } else {

            /*
                This is in case of ish'a prayer which lasts 
                all the way up to fajr the next day, but we want
                to keep our vaktija running constantly.
            */
            if (i == (PRAYER_TIME_NUM - 1)) {

                return 0;

            }

        }

    }

    return -1; /* This should never get returned. */

}

/*
    Returns the index of the current vakat based on provided time.
*/
int current_vakat(const struct vaktija *vaktija, struct tm time)
{

    int next = next_vakat(vaktija, time);
    int remainder = (next - 1) % PRAYER_TIME_NUM;

    /* 
        Because it turns out that in C, -1 % 6 = +/- 1, not 5.
    */
    return (remainder < 0) ? remainder + PRAYER_TIME_NUM : remainder;

}

void calculate_midnight(const struct vaktija *vaktija, struct tm *midnight)
{
    
    struct tm fajr; 
    parse_timestr(vaktija->prayers[0], &fajr);

    struct tm maghrib;
    parse_timestr(vaktija->prayers[4], &maghrib);

    struct tm sub;
    subtract_time(fajr, maghrib, &sub);

    struct tm div;
    divide_time(sub, 2, &div);

    subtract_time(fajr, div, midnight);
    
} 

void calculate_third(const struct vaktija *vaktija, struct tm *third)
{
    
    struct tm fajr; 
    parse_timestr(vaktija->prayers[0], &fajr);

    struct tm maghrib;
    parse_timestr(vaktija->prayers[4], &maghrib);

    struct tm sub;
    subtract_time(fajr, maghrib, &sub);

    struct tm div;
    divide_time(sub, 3, &div);

    subtract_time(fajr, div, third);
    
}

#ifdef USE_ANSI_COLOR

#define ANSI_YELLOW(str) "\x1b[33m" str "\x1b[0m"
#define ANSI_CYAN(str) "\x1b[36m" str "\x1b[0m"
#define ANSI_GREEN(str) "\x1b[32m" str "\x1b[0m"
#define ANSI_RED(str) "\x1b[31m" str "\x1b[0m"

#endif

static char *vakat_names[PRAYER_TIME_NUM] = {
    "Dawn",
    "Sunrise",
    "Dhuhr",
    "Asr",
    "Maghrib",
    "Isha"
};

/*
    vakat has to be a valid vaktija index (0 - 5)

    if raw is 1, only the raw timestamp of the vakat shall be printed
*/
void print_vakat(const struct vaktija *vaktija, int vakat, int raw)
{

    if (vakat < 0 || vakat > (PRAYER_TIME_NUM - 1)) {
        printf("Could not print the current vakat. Invalid index passed! (idx: %d)\n", vakat);
        exit(EXIT_FAILURE);
    }

    if (raw) {
        
        printf("%s", vaktija->prayers[vakat]);

    } else {

	#ifdef USE_ANSI_COLOR

	printf(ANSI_CYAN("%s") ": " ANSI_YELLOW("%s") "\n", vakat_names[vakat], vaktija->prayers[vakat]);

	#else

        printf("%s: %s\n", vakat_names[vakat], vaktija->prayers[vakat]);

	#endif

    }

}

/*
    Prints the entire vaktija together with current time.

    This is the default action of the program.
*/
void print_vaktija(const struct vaktija *vaktija)
{

    time_t curr;
    time(&curr);
    struct tm current = *localtime(&curr);

    char currstr[6];
    currstr[0] = '\0';
    strftime(currstr, 6, "%H:%M", &current);
   
    #ifdef USE_ANSI_COLOR

    printf(ANSI_CYAN("Current time is") ": " ANSI_YELLOW("%s") "\n", currstr);
   
    #else
    
    printf("Current time is: %s\n", currstr);

    #endif

    printf("\n");

    #ifdef USE_ANSI_COLOR

    printf(ANSI_CYAN("Today's date is ") ANSI_RED("%s ") "(" ANSI_GREEN("%s") "):\n", 
		    vaktija->dates[0], vaktija->dates[1]);

    #else

    printf("Today's date is %s (%s):\n", vaktija->dates[0], vaktija->dates[1]);

    #endif

    printf("\n");

    #ifdef USE_ANSI_COLOR

    printf(ANSI_CYAN("Vaktija for") ": " ANSI_YELLOW("%s") "\n", vaktija->location); 

    #else

    printf("Vaktija for %s:\n", vaktija->location);

    #endif

    printf("\n");

    for (int i = 0; i < PRAYER_TIME_NUM; i++) {
        print_vakat(vaktija, i, 0);
    }

    printf("\n");

    struct tm midnight;
    parse_timestr("00:00", &midnight);
    calculate_midnight(vaktija, &midnight);

    char midstr[6];
    midstr[0] = '\0';
    strftime(midstr, 6, "%H:%M", &midnight);

    #ifdef USE_ANSI_COLOR

    printf(ANSI_CYAN("Midnight is on") ": " ANSI_YELLOW("%s") "\n", midstr);

    #else

    printf("Midnight is on: %s\n", midstr);

    #endif

    struct tm third;
    parse_timestr("00:00", &third);
    calculate_third(vaktija, &third);

    char thirdstr[6];
    thirdstr[0] = '\0';
    strftime(thirdstr, 6, "%H:%M", &third);

    #ifdef USE_ANSI_COLOR

    printf(ANSI_CYAN("Last third of the night is on") ": " ANSI_YELLOW("%s") "\n", thirdstr);

    #else

    printf("Last third of the night is on: %s\n", thirdstr);

    #endif


}
