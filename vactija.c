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