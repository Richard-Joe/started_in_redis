#include <CUnit/CUnit.h>

#include "dict.h"
#include "sds.h"
#include "testcases.h"


uint64_t hashCallback(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

int compareCallback(void *privdata, const void *key1, const void *key2) {
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void freeCallback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);

    sdsfree(val);
}


dictType type = {
    hashCallback,
    NULL,
    NULL,
    compareCallback,
    freeCallback,
    NULL
};


void dictTest(void) {
    long j, count = 10;

    dict *d = dictCreate(&type, NULL);

    for (j = 0; j < count; j++) {
        CU_ASSERT_EQUAL(
            dictAdd(d, sdsfromlonglong(j), (void*)j),
            DICT_OK
        );
    }
    CU_ASSERT_EQUAL(dictSize(d), count);

    /* Wait for rehashing. */
    while (dictIsRehashing(d)) {
        dictRehashMilliseconds(d, 100);
    }

    for (j = 0; j < count; j++) {
        sds key = sdsfromlonglong(j);
        dictEntry *de = dictFind(d, key);
        CU_ASSERT_NOT_EQUAL(de, NULL);
        CU_ASSERT_STRING_EQUAL(dictGetKey(de), key);
        CU_ASSERT_EQUAL(dictGetSignedIntegerVal(de), j);
        sdsfree(key);
    }

    dictRelease(d);
}