#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "temporal.h"
#include "cachefile.h"

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

        char *errstr = strerror(errcode);
        printf("Err: %s\n", errstr);
        exit(EXIT_FAILURE);

    }

}

void write_cache(const char *path, const char *json)
{

    FILE *cache = fopen(path, "w");

    if (cache) {

        int res = fprintf(cache, "%s", json);

        if (res < 0) {
            printf("An error has occurred while writing to the cache file. Aborting!\n");
            fclose(cache);
            exit(EXIT_FAILURE);
        }

        fclose(cache);

    } else {
    
        int errcode = errno;
        printf("Encountered an error while opening cache file!\n");
        vactija_error(errcode);

    }

}

/*
    Reads the data from the cache file into a string.

    The returned string will be null-terminated and dynamically
    allocated, therefore it has to be freed once it's no longer used.
*/
char *read_cache(const char *path)
{

    if (cache_exists(path) == 0) {

        printf("Attempted to find cache file at path: %s\n", path);
        printf("The cache file does not exist. Could not read data!\n");
        exit(EXIT_FAILURE);

    }

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