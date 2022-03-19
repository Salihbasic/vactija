#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"

#include "../util/temporal.h"
#include "../util/jsmnutil.h"
#include "../vactija.h"

static int passed_test = 0;
static int failed_test = 0;

int timestr_parsing(void);
int minute_comparison(void);
int jsonparse_test(void);
int jsonsearch_test(void);

void test(int (*testf)(void), char *name)
{

    int res = testf();

    if (res == 0) {
        passed_test++;
    } else {
        failed_test++;
        printf("Failed test \"%s\" (at line: %d)\n", name, res);
    }

}

int timestr_parsing(void) {

    char *tstr1 = "00:00";
    char *tstr2 = "11:23";
    char *tstr3 = "03:07";

    struct tm res1;
    parse_timestr(tstr1, &res1);
    check((res1.tm_hour == 0) && (res1.tm_min == 0));

    struct tm res2;
    parse_timestr(tstr2, &res2);
    check((res2.tm_hour == 11) && (res2.tm_min == 23));

    struct tm res3;
    parse_timestr(tstr3, &res3);
    check((res3.tm_hour == 3) && (res3.tm_min == 7));

    done();

}

int minute_comparison(void) 
{

    char *time1 = "14:23";
    struct tm res1;
    parse_timestr(time1, &res1);

    char *time2 = "14:21";
    struct tm res2;
    parse_timestr(time2, &res2);

    char *time3 = "10:00";
    struct tm res3;
    parse_timestr(time3, &res3);

    char *time4 = "14:23";
    struct tm res4;
    parse_timestr(time4, &res4);

    check(compare_time(res1, res2) == 1);
    check(compare_time(res2, res3) == 1);
    check(compare_time(res2, res1) == -1);
    check(compare_time(res1, res4) == 0);

    done();

}

int jsonsearch_test(void)
{

    char *json = read_cache("test/vactijacache");

    jsmn_parser pars;
    jsmntok_t tok[VACTIJA_JSMN_TOKENS]; /* Based on current JSON file structure */

    jsmn_init(&pars);
    int result = jsmn_parse(&pars, json, strlen(json), tok, VACTIJA_JSMN_TOKENS);

    check(result == 15);

    jsmntok_t *loctok = find_by_key(json, "lokacija", tok, VACTIJA_JSMN_TOKENS);
    char *location = get_simple(json, loctok);

    check(strcmp(location, "Sarajevo") == 0);

    int i = find_idx_by_key(json, "vakat", tok, VACTIJA_JSMN_TOKENS);
    char **vakats = get_array(json, i, tok, 6);

    check(strcmp(vakats[0], "4:59") == 0);
    check(strcmp(vakats[1], "6:35") == 0);
    check(strcmp(vakats[2], "12:01") == 0);
    check(strcmp(vakats[3], "14:52") == 0);
    check(strcmp(vakats[4], "17:27") == 0);
    check(strcmp(vakats[5], "18:51") == 0);

    for (int i = 0; i < 6; i++) {
        free(vakats[i]);
    }
    free(vakats);
    free (location);

    done();

}

int jsonparse_test(void)
{

    char *json = read_cache("test/vactijacache");

    struct vaktija *v = parse_cache(json);
    

    /*
        Based on values from the test/vactijacache file
    */
    check(strcmp(v->location, "Sarajevo") == 0);
    check(strcmp(v->dates[0], "18. redžeb 1443") == 0);
    check(strcmp(v->dates[1], "subota, 19. februar 2022") == 0);
    
    check(strcmp(v->prayers[0], "4:59") == 0);
    check(strcmp(v->prayers[1], "6:35") == 0);
    check(strcmp(v->prayers[2], "12:01") == 0);
    check(strcmp(v->prayers[3], "14:52") == 0);
    check(strcmp(v->prayers[4], "17:27") == 0);
    check(strcmp(v->prayers[5], "18:51") == 0);
    
    free(json);

    done();

}

int main(void) {

    test(timestr_parsing, "parsing timestrings");
    test(minute_comparison, "comparing minutes");
    test(jsonsearch_test, "searching json test");
    test(jsonparse_test, "parsing cache json");

    printf("Ran %d tests. Failed %d.\n", (passed_test + failed_test), failed_test);

}