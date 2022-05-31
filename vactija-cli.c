#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "util/cachefile.h"
#include "vactija.h"
#include "config.h"

static char *pname = "vactija";
static char *pname_full = "Vactija";

static char *maintainer = "Mahir Salihbasic";
static char *email = "mahir.salihbasic@protonmail.com";
static char *website = "https://github.com/Salihbasic/vactija";

static struct option longopts[] = {

    {"help", no_argument, NULL, 'h'},
    {"update", no_argument, NULL, 'h'},
    {"directory", required_argument, NULL, 'd'},
    {"location", required_argument, NULL, 'l'},
    {"date", required_argument, NULL, 'y'},
    {"raw", no_argument, NULL, 'r'}

};

static void usage(int status);
static void print_raw_data(void);
static int validate_date(const char *date);

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("No action specified!\n");
        usage(EXIT_FAILURE);
    }

    int update_flag = 0;
    int raw_flag = 0;
    char *dir_path = NULL;
    char *loc = NULL;
    char *date = NULL;

    int c; 
    while((c = getopt_long(argc, argv, "hurd:l:y:", longopts, NULL)) != -1) {

        switch (c) {
        
        case 'h':
            usage(EXIT_SUCCESS);
            break;

        case 'u':
            update_flag = 1;
            break;

        case 'd':
            dir_path = strndup(optarg, strlen(optarg));
            update_flag = 1;
            break;

        case 'l':
            loc = strndup(optarg, strlen(optarg));
            update_flag = 1;
            break;
        
        case 'y':
            date = strndup(optarg, strlen(optarg));
            update_flag = 1;
            break;
        
        case 'r':
            raw_flag = 1;
            break;

        }

    }

    if (argv[optind] == NULL) {
        printf("No action specified!\n");
        usage(EXIT_FAILURE);
    }

    if (cfg_alwaysupdate) {
        update_flag = 1;
    }

    const char *location = (loc == NULL) ? cfg_loc : loc;
    const char *directory = (dir_path == NULL) ? cfg_cachedir : dir_path;

    char *vdata;
    if (cfg_nocache) {

        vdata = download_vaktija(location, date);

    } else {
    
        if (update_flag || cache_exists(directory) != 1 || cache_outdated(directory) == 1) {
            
            vdata = download_vaktija(location, date);
            write_cache(directory, vdata);

        } else {

            vdata = read_cache(directory);

        }

    }
    struct vaktija *v = parse_data(vdata);

    if (date != NULL) {

        if (validate_date(date) == 0) {

            printf("Invalid date provided!\n");
            printf("Date format: <yyyy>[/mm[/dd]]\n");

            exit(EXIT_FAILURE);
            
        }

    }

    char *action = argv[optind];

    if (strcmp(action, "print") == 0) {

        if (raw_flag) {
                
            printf("%s", vdata);

        } else {

            print_vaktija(v);
                
        }

    }

    if ((strlen(action) == 1) && isdigit(action[0])) {

        int num = action[0] - '0'; /* should be safe? */

        if (num < 0 || num > 5) {
        
            printf("Invalid vakat number! Expected a value in range of 0 to 5.\n");
            exit(EXIT_FAILURE);
        
        }

        print_vakat(v, num, raw_flag);

    }

    if (strcmp(action, "next") == 0) {

        time_t curr;
        time(&curr);
        struct tm current = *localtime(&curr);

        print_vakat(v, next_vakat(v, current), raw_flag); 

    }

    if (strcmp(action, "current") == 0) {

        time_t curr;
        time(&curr);
        struct tm current = *localtime(&curr);

        print_vakat(v, current_vakat(v, current), raw_flag); 

    }

    delete_vaktija(v);
    free(vdata);

    exit(EXIT_SUCCESS);

}

static void usage(int status)
{

    if (status != EXIT_SUCCESS) {

        printf("Try \"%s --help\" for more information.\n", pname);
        exit(status);

    }

    printf("Usage:\n");
    printf(" %s [options] <action>\n", pname);

    printf("\n");

    printf("%s options:\n", pname_full);
    printf(" -h, --help           prints this message\n");

    printf(" -u, --update         forces an update of vaktija data\n");

    printf(" -r, --raw            outputs raw data\n");

    printf(" -d, --directory      sets the directory used to download/search\n");
    printf("                      the vaktija data.\n");

    printf(" -l, --location       sets the location ID (found in locations.txt)\n");
    printf("                      for vaktija data from the API. This will redownload\n");
    printf("                      data regardless of caching.\n");

    printf(" -y, --date           sets the date for vaktija data, the required date format\n");
    printf("                      is <yyyy>[/mm[/dd]]. This will redownload data regardless\n");
    printf("                      of caching.\n");



    printf("%s actions:\n", pname_full);
    printf(" print                 prints the entire vaktija\n");
    printf(" #                     prints the specified vakat [# = (0 - 5)]\n");
    printf(" next                  prints the next vakat\n");
    printf(" current               prints the current vakat\n");

    printf("\n");

    printf("Examples:\n");
    printf("  %s -r -d /home/user/altcache -y 2020/04/01 -l 82 print\n", pname);
    printf("  %s -u 3\n", pname);

    printf("\n");

    printf("Maintainer: %s <%s>\n", maintainer, email);
    printf("Website: %s\n", website);

    exit(EXIT_SUCCESS);

}

static int validate_date(const char *date)
{

    /* date format <yyyy>[/mm[/dd]] */

    int len = strlen(date);

    if (len == 4 || len == 7 || len == 10) {

        for (int i = 0; date[i] != '\0'; i++) {

            if (i == 4 || i == 7) {
                
                if (date[i] != '/') {

                    return 0;

                }

            } else {

                if (!isdigit(date[i])) {

                    return 0;

                }

            }

        }

    } else {

        return 0;

    }

    return 1;

}
