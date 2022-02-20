#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>

#include "vactija.h"
#include "util/temporal.h"
#include "microjson/mjson.h"

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
    Reads the data from the cache file which is in the JSON format.

    It then feeds the JSON data into the microjson parser (authored
    by Eric S. Raymond) in order to get the data out of the JSON
    and then stores it all in the result vaktija struct.

    In case the JSON structure changes within the API, this function
    will certainly fail. Therefore, any changes to the API need to be
    addressed here.
*/
void read_cache(const char *path, struct vaktija *result)
{

    /*
       NOTE: After a while, I've finally figured out how this damn
       parser works.

       Basically, the *_store will store all the strings from the
       array as if they were one massive concatenated string, all
       separated with a \0 character.
       These will then be sent into appropriate array indices.

       In other words, JSON_PARSER_STORELENGTH should be a large
       enough number to store an entire array-worth of strings.
    */

    const size_t loclen = 20;
    char location[loclen];

    char *prayers[PRAYER_TIME_NUM];
    char prayer_store[JSON_PARSER_STORELENGTH];
    int prayer_count;

    char *dates[2];
    char date_store[JSON_PARSER_STORELENGTH];
    int date_count;

    const struct json_attr_t vaktija_attrs[] = {

        {"lokacija",    t_string,      .addr.string = location,
                                       .len = loclen},

        {"vakat",       t_array,       .addr.array.element_type = t_string,
                                       .addr.array.arr.strings.ptrs = prayers,
                                       .addr.array.arr.strings.store = prayer_store,
                                       .addr.array.arr.strings.storelen = JSON_PARSER_STORELENGTH,
                                       .addr.array.count = &prayer_count,
                                       .addr.array.maxlen = PRAYER_TIME_NUM},

        {"datum",       t_array,       .addr.array.element_type = t_string,
                                       .addr.array.arr.strings.ptrs = dates,
                                       .addr.array.arr.strings.store = date_store,
                                       .addr.array.arr.strings.storelen = JSON_PARSER_STORELENGTH,
                                       .addr.array.count = &date_count,
                                       .addr.array.maxlen = 2}

    };

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

        int read = fread(json_prayer_buf, sizeof(char), file_size, cache);

        if (read != file_size) {
            
            printf("Could not read the entire file.\n");
            printf("Filesize: %lu bytes.\n Read: %d bytes.\n", file_size, read);

            exit(EXIT_FAILURE);
        }

        json_prayer_buf[file_size] = '\0';

        int status = json_read_object(json_prayer_buf, vaktija_attrs, NULL);

        if (status != 0) {

            printf("An error has occurred during JSON parsing!\n");
            printf("Err: %s (code: %d)\n", json_error_string(status), status);

            exit(EXIT_FAILURE);

        }

        for (int i = 0; i < PRAYER_TIME_NUM; i++) {
            result->prayers[i] = prayers[i];
        }
        result->location = location;
        result->date_hijra = dates[0];
        result->date_greg = dates[1];

        free(json_prayer_buf);

    } else {
    
        int errcode = errno;
        printf("Encountered an error while opening the JSON file!\n");
        vactija_error(errcode);

    }

}