#include <stddef.h>
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

/* 
    Errcode needs to be equal to whetever errno value
    the error is supposed to display.
*/
#define vactija_error(errcode)                                        \
    char *errstr = strerror(errcode);                                 \
    printf("Err: %s\n", errstr);                                      \
    exit(EXIT_FAILURE)           

/*
    Returns 1 iff the cache file exists.
*/
int cache_exists(const char *path)
{

    return (access(path, F_OK) == 0);

}

/*
    Checks whether the cache file which holds all the prayer data, 
    is outdated.

    Returns 1 iff the cache file is outdated (i.e its metadata date
    is earlier than the current date), and 0 otherwise.

    In case the file does not exist, it is treated as outdated.
*/
int cache_outdated(const char *path)
{

    if (cache_exists(path) != 1) {
        return 1;
    }

    struct stat meta;

    if (stat(path, &meta) == 0) {

        time_t current;
        time(&current);

        time_t mtime = meta.st_mtim.tv_sec;

        /* 
           localtime stores results in a buffer, so we need to
           store them in a local struct before calling it again

           (could have used localtime_r, but no point in going
           GNU specific without multithreading)
        */
        struct tm curr = *localtime(&current);
        struct tm mt = *localtime(&mtime);

        return compare_date(curr, mt) > 0;

    } else {
    
        int errcode = errno;
        printf("Encountered an error while checking cache metadata.\n");
        vactija_error(errcode);

    }

}

void download_latest(const char *path)
{

    CURL *curl = curl_easy_init();

    if (curl) {

        FILE *cache = fopen(path, "w");

        if (cache) {

            char errbuf[CURL_ERROR_SIZE];

            curl_easy_setopt(curl, CURLOPT_URL, VAKTIJA_API_URL);
            
            /* write to cache using the default write to file function */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, cache);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
            errbuf[0] = 0;

            CURLcode result = curl_easy_perform(curl);

            if (result == CURLE_OK) {

                curl_easy_cleanup(curl);
                fclose(cache);
                return;

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
                fclose(cache);

                exit(EXIT_FAILURE);

            }

        } else {

            int errcode = errno;
            printf("Encountered an error while opening cache file!\n");
            vactija_error(errcode);

        }

    } else {
    
        printf("Could not initialise libcurl handle!\n");

        exit(EXIT_FAILURE);

    }

}

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
    Reads the data from the cache file into a string.

    The returned string will be null-terminated and dynamically
    allocated, therefore it has to be freed once it's no longer used.
*/
char *read_cache(const char *path)
{

    FILE *cache = fopen(path, "r");

    if (cache) {

        /* 
           We have to null-terminate the string, 
           so finding out its size is necessary 
        */
        fseek(cache, 0, SEEK_END);
        unsigned long file_size = ftell(cache);
        rewind(cache);

        char *json_prayer_buf = malloc(sizeof *json_prayer_buf * (file_size + 1));

        if (json_prayer_buf == NULL) {

            printf("Could not allocate enough memory to read contents of cache file!\n");

            int errcode = errno;
            vactija_error(errcode);

        }

        int read = fread(json_prayer_buf, sizeof(char), file_size, cache);

        if (read != file_size) {
            
            printf("Could not read the entire file.\n");
            printf("Filesize: %lu bytes.\n Read: %d bytes.\n", file_size, read);

            exit(EXIT_FAILURE);
        }

        json_prayer_buf[file_size] = '\0';

        return json_prayer_buf;

    } else {
    
        int errcode = errno;
        printf("Encountered an error while opening the JSON file!\n");
        vactija_error(errcode);

    }

}

/*
    Uses jsmn by Serge Zaitsev to parse/tokenise the JSON from cache
    file and then processes it with functions from jsmnutil.

    This function is highly dependent on the current JSON structure
    of the API, which can be changed in the future, therefore this
    function must be updated in such cases.

    (All values for the structure were determined experimentally.
    jsmn is a minimalist library, and while a fully generalised
    parser could be built on top of it, it'd be a waste of time
    for this project, which is why only the barebones have been
    done here.)
*/
struct vaktija *parse_cache(const char *json)
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