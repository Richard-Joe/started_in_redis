#ifndef __DICT_H__
#define __DICT_H__

#include <stdint.h>


#define DICT_OK 0
#define DICT_ERR 1

#define DICT_NOTUSED(V) ((void) V)


// 哈希表节点
typedef struct dictEntry {
    // 键
    void *key;

    // 值
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;

    // 指向下个哈希表节点，形成链表，以此解决键冲突（collision）的问题
    struct dictEntry *next;
} dictEntry;


// 哈希表
typedef struct dictht {
    // 哈希表数组
    dictEntry **table;

    // 哈希表大小
    unsigned long size;

    // 哈希表大小掩码，用于计算索引值
    // 总是等于size-1
    unsigned long sizemask;

    // 该哈希表已有节点的数量
    unsigned long used;
} dictht;


// 类型特定函数结构
typedef struct dictType {
    // 计算哈希值的函数
    uint64_t (*hashFunction)(const void *key);

    // 复制键的函数
    void *(*keyDup)(void *privdata, const void *key);

    // 复制值的函数
    void *(*valDup)(void *privdata, const void *obj);

    // 对比键的函数
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);

    // 销毁键的函数
    void (*keyDestructor)(void *privdata, void *key);

    // 销毁值的函数
    void (*valDestructor)(void *privdata, void *obj);
} dictType;



// 字典
typedef struct dict {
    // 类型特定函数
    dictType *type;

    // 私有数据
    void *privdata;
    
    // 哈希表，
    // 一般情况下，字典只使用ht[0]哈希表，ht[1]哈希表只会在对ht[0]哈希表进行rehash时使用。
    dictht ht[2];

    // rehash索引
    // 当rehash不在进行时，值为-1
    long rehashidx;

    // 正在遍历该字典的迭代器个数
    unsigned long iterators; 
} dict;


// 字典迭代器
// TODO 目前先只实现安全版本的迭代器
typedef struct dictIterator {
    dict *d;

    // 迭代器当前指向哈希表数组的索引
    long index;

    // table 哈希表号
    int table;

#ifdef SUPPORT_SAFE_ITER
    // safe 标记迭代器是否安全，1表示安全，0表示非安全
    int safe;
#endif

    // entry表示当前迭代节点
    dictEntry *entry;
    // nextEntry表示当前迭代节点的下一节点，非安全迭代器中，当前节点可能被修改，需要记录下一节点。
    dictEntry *nextEntry;

#ifdef SUPPORT_SAFE_ITER
    // 指纹是一个64位的数字
    // 表示在给定的时间内字典的状态，它只是将一些dict属性合并在一起。
    // 初始化不安全的迭代器时，我们将获得dict指纹，并在释放迭代器时再次检查指纹。
    // 如果两个指纹不同，这意味着迭代器的用户在迭代时对字典执行禁止操作。
    long long fingerprint;
#endif
} dictIterator;


// 哈希表的初始大小
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/

#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)

#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)

#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dictHashKey(d, key) (d)->type->hashFunction(key)
#define dictIsRehashing(d) ((d)->rehashidx != -1)


/* ------------------------------- APIs ------------------------------------*/
dict *dictCreate(dictType *type, void *privDataPtr);
dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing);
int dictAdd(dict *d, void *key, void *val);
int dictDelete(dict *d, const void *key);
void dictRelease(dict *d);

int dictExpand(dict *d, unsigned long size);
int dictRehash(dict *d, int n);

dictEntry *dictFind(dict *d, const void *key);

dictIterator *dictGetIterator(dict *d);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);

uint64_t dictGenHashFunction(const void *key, int len);

long long timeInMilliseconds(void);
int dictRehashMilliseconds(dict *d, int ms);

#endif