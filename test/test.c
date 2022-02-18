#include <stdio.h>
#include "test.h"
#include "../util/temporal.h"

static int passed_test = 0;
static int failed_test = 0;

int timestr_parsing(void);

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

int main(void) {

    test(timestr_parsing, "parsing timestrings");
    test(minute_comparison, "comparing minutes");

    printf("Ran %d tests. Failed %d.\n", (passed_test + failed_test), failed_test);

}