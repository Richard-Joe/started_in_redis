#include "dict.h"
#include "zmalloc.h"


// 重置哈希表
static void _dictReset(dictht *ht)
{
    ht->table = NULL;
    ht->size = 0;
    ht->sizemask = 0;
    ht->used = 0;
}


// 初始化字典
static int _dictInit(dict *d, dictType *type, void *privDataPtr) {
    _dictReset(&d->ht[0]);
    _dictReset(&d->ht[1]);
    d->type = type;
    d->privdata = privDataPtr;
    d->rehashidx = -1;
    d->iterators = 0;
    return DICT_OK;
}


/**
 * 创建一个新的字典
 * @param  type         类型特定函数
 * @param  privDataPtr  私有数据
 * @return
 */
dict *dictCreate(dictType *type, void *privDataPtr) {
    dict *d = zmalloc(sizeof(*d));
    _dictInit(d, type, privDataPtr);
    return d;
}


dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing) {
    dictEntry *entry;

    entry = zmalloc(sizeof(*entry));

    return entry;
}

/**
 * 将给定的键值对添加到字典
 * @param  d    字典
 * @param  key  键
 * @param  val  值
 * @return
 */
int dictAdd(dict *d, void *key, void *val) {
    dictEntry *entry = dictAddRaw(d, key, NULL);

    if (!entry) return DICT_ERR;
    dictSetVal(d, entry, val);
    return DICT_OK;
}



// 清空一个字典
static int _dictClear(dict *d, dictht *ht, void(callback)(void *)) {
    return DICT_OK;
}


/**
 * 释放字典，以及字典中包含的所有键值对
 * @param  dict         字典指针
 * @return void
 */
void dictRelease(dict *d) {
    _dictClear(d, &d->ht[0], NULL);
    _dictClear(d, &d->ht[1], NULL);
    zfree(d);
}