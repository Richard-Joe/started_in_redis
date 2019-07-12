#include <CUnit/CUnit.h>
#include "sds.h"

void sds_test(void) {
    sds s = sdsnew("abcd");
    CU_ASSERT_EQUAL(sdslen(s), 4);
    CU_ASSERT_STRING_EQUAL(s, "abcd");

    s = sdscat(s, "ef");
    CU_ASSERT_EQUAL(sdslen(s), 6);
    CU_ASSERT_STRING_EQUAL(s, "abcdef");

    s = sdscatlen(s, "www", 2);
    CU_ASSERT_EQUAL(sdslen(s), 8);
    CU_ASSERT_STRING_EQUAL(s, "abcdefww");

    sds o = sdsempty();
    o = sdscpy(o, "abcd");
    CU_ASSERT_EQUAL(sdslen(o), 4);
    CU_ASSERT_STRING_EQUAL(o, "abcd");

    s = sdscatsds(s, o);
    CU_ASSERT_EQUAL(sdslen(s), 12);
    CU_ASSERT_STRING_EQUAL(s, "abcdefwwabcd");

    sdsfree(o);
    sdsfree(s);
}