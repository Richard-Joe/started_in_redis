#include "skiplist.h"
#include "zmalloc.h"


/**
 * 创建跳跃表节点
 */
skiplistNode *slCreateNode(int level, double score, sds ele) {
    skiplistNode *node = zmalloc(sizeof(*node) + level * sizeof(struct skiplistLevel));
    node->ele = ele;
    node->score = score;
    return node;
}


/**
 * 释放跳跃表节点
 */
void slFreeNode(skiplistNode *node) {
    sdsfree(node->ele);
    zfree(node);
}


/**
 * 创建跳跃表
 */
skiplist *slCreate(void) {
    int i;
    skiplist *sl;

    sl = zmalloc(sizeof(*sl));
    sl->level = 1;
    sl->length = 0;
    sl->head = slCreateNode(SKIPLIST_MAXLEVEL, 0, NULL);
    for (i = 0; i < SKIPLIST_MAXLEVEL; i++) {
        sl->head->level[i].forward = NULL;
        sl->head->level[i].span = 0;
    }
    sl->head->backward = NULL;
    sl->tail = NULL;

    return sl;
}


/**
 * 释放跳跃表
 */
void slFree(skiplist *sl) {
    skiplistNode *node = sl->head->level[0].forward, *next;

    while (node) {
        next = node->level[0].forward;
        slFreeNode(node);
        node = next;
    }
    zfree(sl);
}


/**
 * 向跳跃表插入新的节点
 */
skiplistNode *slInsert(skiplist *sl, double score, sds ele) {
    return NULL;
}
