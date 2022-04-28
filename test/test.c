#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"

#include "../util/temporal.h"
#include "../util/jsmnutil.h"
#include "../util/cachefile.h"
#include "../vactija.h"

static int passed_test = 0;
static int failed_test = 0;

static int timestr_parsing(void);
static int minute_comparison(void);
static int jsonparse_test(void);
static int jsonsearch_test(void);
static int nextvakat_test(void);
static int currentvakat_test(void);

static void test(int (*testf)(void), char *name)
{

    int res = testf();

    if (res == 0) {
        passed_test++;
    } else {
        failed_test++;
        printf("Failed test \"%s\" (at line: %d)\n", name, res);
    }

}

static int timestr_parsing(void) {

    char *tstr1 = "00:00";
    char *tstr2 = "11:23";
    char *tstr3 = "03:07";
    char *tstr4 = "4:35";

    struct tm res1;
    parse_timestr(tstr1, &res1);
    check((res1.tm_hour == 0) && (res1.tm_min == 0));

    struct tm res2;
    parse_timestr(tstr2, &res2);
    check((res2.tm_hour == 11) && (res2.tm_min == 23));

    struct tm res3;
    parse_timestr(tstr3, &res3);
    check((res3.tm_hour == 3) && (res3.tm_min == 7));

    struct tm res4;
    parse_timestr(tstr4, &res4);
    check((res4.tm_hour == 4) && (res4.tm_min == 35));

    done();

}

static int minute_comparison(void) 
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

static int jsonsearch_test(void)
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
    free(location);

    done();

}

static int jsonparse_test(void)
{

    char *json = read_cache("test/vactijacache");

    struct vaktija *v = parse_data(json);

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
    free(v);

    done();

}

static int nextvakat_test(void)
{

    char *json = read_cache("test/vactijacache");
    struct vaktija *v = parse_data(json);

    /* Expect index 0 */
    char *time1 = "4:50";
    struct tm tm1;
    parse_timestr(time1, &tm1);
    check(next_vakat(v, tm1) == 0);

    /* Expect index 1 */
    char *time2 = "4:59";
    struct tm tm2;
    parse_timestr(time2, &tm2);
    check(next_vakat(v, tm2) == 1);

    /* Expect index 1 */
    char *time3 = "5:20";
    struct tm tm3;
    parse_timestr(time3, &tm3);
    check(next_vakat(v, tm3) == 1);

    /* Expect index 2 */
    char *time4 = "11:59";
    struct tm tm4;
    parse_timestr(time4, &tm4);
    check(next_vakat(v, tm4) == 2);

    /* Expect index 3 */
    char *time5 = "12:02";
    struct tm tm5;
    parse_timestr(time5, &tm5);
    check(next_vakat(v, tm5) == 3);

    /* Expect index 4 */
    char *time6 = "15:00";
    struct tm tm6;
    parse_timestr(time6, &tm6);
    check(next_vakat(v, tm6) == 4);

    /* Expect index 5 */
    char *time7 = "18:00";
    struct tm tm7;
    parse_timestr(time7, &tm7);
    check(next_vakat(v, tm7) == 5);

    /* Expect index 5 */
    char *time8 = "20:20";
    struct tm tm8;
    parse_timestr(time8, &tm8);
    check(next_vakat(v, tm8) == 0);

    free(json);
    free(v);

    done();

}

static int currentvakat_test(void)
{

    char *json = read_cache("test/vactijacache");
    struct vaktija *v = parse_data(json);

    /* Expect index 5 */
    char *time1 = "4:50";
    struct tm tm1;
    parse_timestr(time1, &tm1);
    check(current_vakat(v, tm1) == 5);

    /* Expect index 0 */
    char *time2 = "4:59";
    struct tm tm2;
    parse_timestr(time2, &tm2);
    check(current_vakat(v, tm2) == 0);

    /* Expect index 0 */
    char *time3 = "5:20";
    struct tm tm3;
    parse_timestr(time3, &tm3);
    check(current_vakat(v, tm3) == 0);

    /* Expect index 1 */
    char *time4 = "11:59";
    struct tm tm4;
    parse_timestr(time4, &tm4);
    check(current_vakat(v, tm4) == 1);

    /* Expect index 2 */
    char *time5 = "12:02";
    struct tm tm5;
    parse_timestr(time5, &tm5);
    check(current_vakat(v, tm5) == 2);

    /* Expect index 3 */
    char *time6 = "15:00";
    struct tm tm6;
    parse_timestr(time6, &tm6);
    check(current_vakat(v, tm6) == 3);

    /* Expect index 4 */
    char *time7 = "18:00";
    struct tm tm7;
    parse_timestr(time7, &tm7);
    check(current_vakat(v, tm7) == 4);

    /* Expect index 5 */
    char *time8 = "20:20";
    struct tm tm8;
    parse_timestr(time8, &tm8);
    check(current_vakat(v, tm8) == 5);

    free(json);
    free(v);

    done();

}

int main(void) {

    test(timestr_parsing, "parsing timestrings");
    test(minute_comparison, "comparing minutes");
    test(jsonsearch_test, "searching json test");
    test(jsonparse_test, "parsing cache json");
    test(nextvakat_test, "getting next vakat");
    test(currentvakat_test, "getting current vakat");

    printf("Ran %d tests. Failed %d.\n", (passed_test + failed_test), failed_test);

    return 0;

}
