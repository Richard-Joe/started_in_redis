#include <stdio.h>
#include <CUnit/CUnit.h>

#include "sds.h"
#include "dlist.h"
#include "testcases.h"

static void printList(list *l, int direction) {
    listNode *node, *end;
    listIter *iter = listGetIterator(l, direction);

    end = (direction == AL_START_HEAD) ? listLast(l) : listFirst(l);

    while ((node = listNext(iter)) != NULL) {
        printf("%s", (sds)listNodeValue(node));
        printf("%s", (node != end) ? " <--> " : "\n");
    }

    listReleaseIterator(iter);
}

void dlistTest(void) {

    listNode *node;
    list *l;

    sds s1 = sdsnew("abcd");
    sds s2 = sdsnew("qwer");
    sds s3 = sdsnew("iop");
    sds s4 = sdsnew("jk");
    sds s5 = sdsnew("mn");
    sds s6 = sdsnew("uv");

    l = listCreate();

    printf("\n\n******* dlistTest output ********\n");

    (void)listAddNodeHead(l, s1);
    (void)listAddNodeHead(l, s2);
    (void)listAddNodeHead(l, s3);
    CU_ASSERT_EQUAL(listLength(l), 3);
    /* iop <--> qwer <--> abcd */
    printList(l, AL_START_HEAD);
    /* abcd <--> qwer <--> iop */
    printList(l, AL_START_TAIL);

    node = listIndex(l, 0);
    CU_ASSERT_STRING_EQUAL(listNodeValue(node), s3);

    (void)listAddNodeTail(l, s4);
    CU_ASSERT_EQUAL(listLength(l), 4);
    /* iop <--> qwer <--> abcd <--> jk */
    printList(l, AL_START_HEAD);
    /* jk <--> abcd <--> qwer <--> iop */
    printList(l, AL_START_TAIL);

    node = listIndex(l, 3);
    CU_ASSERT_STRING_EQUAL(listNodeValue(node), s4);

    (void)listInsertNode(l, node, s5, 0);
    (void)listInsertNode(l, node, s6, 1);
    CU_ASSERT_EQUAL(listLength(l), 6);
    /* iop <--> qwer <--> abcd <--> mn <--> jk <--> uv */
    printList(l, AL_START_HEAD);
    /* uv <--> jk <--> mn <--> abcd <--> qwer <--> iop */
    printList(l, AL_START_TAIL);

    node = listIndex(l, 7);
    CU_ASSERT_EQUAL(node, NULL);

    node = listIndex(l, 2);
    listDelNode(l, node);
    CU_ASSERT_EQUAL(listLength(l), 5);
    /* iop <--> qwer <--> mn <--> jk <--> uv */
    printList(l, AL_START_HEAD);

    node = listIndex(l, 3);
    listDelNode(l, node);
    CU_ASSERT_EQUAL(listLength(l), 4);
    /* iop <--> qwer <--> mn <--> uv */
    printList(l, AL_START_HEAD);

    list *o = listCreate();
    listJoin(o, l);
    CU_ASSERT_EQUAL(listLength(o), 4);
    /* iop <--> qwer <--> mn <--> uv */
    printList(o, AL_START_HEAD);

    listRelease(o);
    listRelease(l);
    sdsfree(s6);
    sdsfree(s5);
    sdsfree(s4);
    sdsfree(s3);
    sdsfree(s2);
    sdsfree(s1);

    printf("\n******* dlistTest ********\n\n");
}