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
static struct vaktija *create_vaktija()
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

    if (result != VACTIJA_JSMN_TOKENS) {

        printf("Parsed %d JSON tokens (expected: %d)!\n", result, VACTIJA_JSMN_TOKENS);
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

        if (compare_time(time, vakattime) <= 0) {

            /*
                Since 0 represents the starting time of fajr,
                if our time is earlier than fajr, we can assume it
                is still ish'a prayer.
            */
            if (i == 0 && (compare_time(time, vakattime) != 0)) {

                return (PRAYER_TIME_NUM - 1);

            } else {

                return i;

            }

            return i;

        } else {

            /*
                This is in case of ish'a prayer which lasts 
                all the way up to fajr the next day, but we want
                to keep our vaktija running constantly.
            */
            if (i == (PRAYER_TIME_NUM - 1)) {

                return i;

            }

        }

    }

    return -1; /* This should never get returned. */

}