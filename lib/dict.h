#ifndef __DICT_H__
#define __DICT_H__

#include <stdint.h>


#define DICT_OK 0
#define DICT_ERR 1


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

    // number of iterators currently running
    unsigned long iterators; 
} dict;


/* ------------------------------- Macros ------------------------------------*/
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)


/* ------------------------------- APIs ------------------------------------*/
dict *dictCreate(dictType *type, void *privDataPtr);
dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing);
int dictAdd(dict *d, void *key, void *val);
int dictDelete(dict *d, const void *key);
void dictRelease(dict *d);


#endif