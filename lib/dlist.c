#include <stdlib.h>

#include "dlist.h"
#include "zmalloc.h"


/*
 * 创建新的链表
 *
 * @param void
 * @return
 */
list *listCreate(void) {
    struct list *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;

    return list;
}


/*
 * 移除链表所有元素
 *
 * @param list 链表指针
 * @return
 */
void listEmpty(list *list) {
    if (list == NULL)
        return;

    listNode *cur, *next;
    unsigned long len;

    cur = list->head;
    len = list->len;
    while (len--) {
        next = cur->next;
        if (list->free)
            list->free(cur->value);
        zfree(cur);
        cur = next;
    }

    list->head = list->tail = NULL;
    list->len = 0;
}


/*
 * 释放链表
 *
 * @param list 链表指针
 * @return
 */
void listRelease(list *list) {
    if (list == NULL)
        return;

    listEmpty(list);
    zfree(list);
}


/*
 * 添加一个节点到链表头
 *
 * @param list 链表指针
 * @param value 节点数据指针
 * @return
 */
list *listAddNodeHead(list *list, void *value) {
    struct listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;

    node->value = value;
    if (list->len == 0) {
        /*
         *    head    tail
         *      \      /
         *        node
         */
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        /*
         *    head            tail
         *      \              /
         *       node === oldnode
         */
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;

    return list;
}


/*
 * 添加一个节点到链表尾
 *
 * @param list 链表指针
 * @param value 节点数据指针
 * @return
 */
list *listAddNodeTail(list *list, void *value) {
    struct listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;

    node->value = value;
    if (list->len == 0) {
        /*
         *    head    tail
         *      \      /
         *        node
         */
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        /*
         *    head            tail
         *      \              /
         *      oldnode === node
         */
        node->next = NULL;
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;

    return list;
}


/*
 * 添加一个新节点到链表的指定节点之前或之后
 *
 * @param list 链表指针
 * @param old_node 指定节点指针
 * @param value 节点数据指针
 * @param after 前或后
 * @return
 */
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    struct listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;

    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->prev = old_node->prev;
        node->next = old_node;
        if (list->head == old_node) {
            list->head = node;
        }
    }

    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }

    list->len++;
    return list;
}


/*
 * 从链表删除指定节点
 *
 * @param list 链表指针
 * @param node 指定节点指针
 * @return
 */
void listDelNode(list *list, listNode *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    if (list->free) {
        list->free(node->value);
    }
    zfree(node);
    list->len--;
}



/*
 * 获取索引n位置的节点
 *
 * @param list 链表指针
 * @param index 索引
 * @return
 */
listNode *listIndex(list *list, long index) {
    listNode *n;

    if (index < 0) {
        index = (-index) - 1;
        n = list->tail;
        while (index-- && n) {
            n = n->prev;
        }
    } else {
        n = list->head;
        while (index-- && n) {
            n = n->next;
        }
    }

    return n;
}


/*
 * 创建迭代器
 *
 * @param list 链表指针
 * @param direction 方向
 * @return
 */
listIter *listGetIterator(list *list, int direction) {
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL)
        return NULL;

    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}


/*
 * 获取下一个节点
 *
 * @param iter 迭代器
 * @return 节点
 */
listNode *listNext(listIter *iter) {
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}


/*
 * 释放迭代器
 *
 * @param iter 迭代器
 * @return
 */
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}


/*
 * 合并链表（将链表o所有的元素添加到链表l的尾部，并将链表o置空）
 *
 * @param l 链表
 * @param o 链表
 * @return
 */
void listJoin(list *l, list *o) {
    if (o->head) {
        o->head->prev = l->tail;
    }

    if (l->tail) {
        l->tail->next = o->head;
    } else {
        l->head = o->head;
    }

    if (o->tail) {
        l->tail = o->tail;
    }

    l->len += o->len;

    /* Setup other as an empty list. */
    o->head = o->tail = NULL;
    o->len = 0;
}