#ifndef __DLIST_H__
#define __DLIST_H__

typedef struct listNode {
    struct listNode *prev;
    struct listNode *next;
    void *value;
} listNode;


typedef struct listIter {
    listNode *next;
    int direction;
} listIter;


typedef struct list {
    listNode *head;
    listNode *tail;

    // 节点值复制函数
    void *(*dup)(void *ptr);

    // 节点值释放函数
    void (*free)(void *ptr);

    // 节点值对比函数
    int (*match)(void *ptr, void *key);

    unsigned long len;
} list;


/* Functions implemented as macros */
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l, m) ((l)->dup = (m))
#define listSetFreeMethod(l, m) ((l)->free = (m))
#define listSetMatchMethod(l, m) ((l)->match = (m))


list *listCreate(void);
void listRelease(list *list);
void listEmpty(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listNode *listIndex(list *list, long index);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

listIter *listGetIterator(list *list, int direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);

void listJoin(list *l, list *o);

#endif
