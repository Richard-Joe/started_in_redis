#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include "sds.h"

#define SKIPLIST_MAXLEVEL 64

// 跳跃表节点
typedef struct skiplistNode {
    // 元素
    sds ele;

    // 分值
    double score;

    // 后退指针
    struct skiplistNode *backward;

    // 层
    struct skiplistLevel {
        // 前进指针
        struct skiplistNode *forward;
        // 跨度
        unsigned long span;
    } level[];
};


// 跳跃表
typedef struct skiplist {
    // 表头，表尾
    struct skiplistNode *head, *tail;
    
    // 节点数量
    unsigned long length;

    // 表中层数最大偶的节点的层数
    int level;
};

#endif