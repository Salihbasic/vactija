#ifndef VACTIJA_TEST_H
#define VACTIJA_TEST_H

#define fail() return __LINE__
#define done() return 0

#define check(cond)           \
    do {                      \
        if (!(cond)) {        \
            fail();           \
        }                     \
    } while (0)

static void test(int (*testf)(void), char *name);

#endif