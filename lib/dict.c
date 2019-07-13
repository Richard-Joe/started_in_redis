#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <sys/time.h>

#include "dict.h"
#include "hash.h"
#include "zmalloc.h"


static int dict_can_resize = 1;
static unsigned int dict_force_resize_ratio = 5;


/* -------------------------- hash functions -------------------------------- */

static uint8_t dict_hash_function_seed[16];

uint64_t dictGenHashFunction(const void *key, int len) {
    return siphash(key, len, dict_hash_function_seed);
}

/* ----------------------------- API implementation ------------------------- */

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

/**
 * rehash操作
 * @param  d  字典
 * @param  n  
 */
int dictRehash(dict *d, int n) {
    int empty_visits = n * 10;

    if (!dictIsRehashing(d)) {
        return 0;
    }

    while(n-- && d->ht[0].used != 0) {
        dictEntry *de, *nextde;

        assert(d->ht[0].size > (unsigned long)d->rehashidx);
        while(d->ht[0].table[d->rehashidx] == NULL) {
            d->rehashidx++;
            if (--empty_visits == 0) {
                return 1;
            }
        }
        de = d->ht[0].table[d->rehashidx];
        // 移动当前索引上的所有节点到新的哈希表上
        while(de) {
            uint64_t h;

            nextde = de->next;
            // 计算当前key在新表的索引
            h = dictHashKey(d, de->key) & d->ht[1].sizemask;
            de->next = d->ht[1].table[h];
            d->ht[1].table[h] = de;
            d->ht[0].used--;
            d->ht[1].used++;
            de = nextde;
        }
        d->ht[0].table[d->rehashidx] = NULL;
        d->rehashidx++;
    }

    // 全部移动到新表后，将ht[1]设置为ht[0]
    if (d->ht[0].used == 0) {
        zfree(d->ht[0].table);
        d->ht[0] = d->ht[1];
        _dictReset(&d->ht[1]);
        d->rehashidx = -1;
        return 0;
    }

    // 需要继续进行rehash操作
    return 1;
}

static void _dictRehashStep(dict *d) {
    // 如果有迭代器在进行，则不进行rehash操作。
    // 防止迭代结果元素缺少或重复
    if (d->iterators == 0) {
        dictRehash(d, 1);
    }
}


// 扩充后的哈希表大小
static unsigned long _dictNextPower(unsigned long size)
{
    unsigned long i = DICT_HT_INITIAL_SIZE;

    if (size >= LONG_MAX) {
        return LONG_MAX + 1LU;
    }

    while(1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

// 扩充或创建哈希表
int dictExpand(dict *d, unsigned long size) {
    if (dictIsRehashing(d) || d->ht[0].used > size) {
        return DICT_ERR;
    }

    dictht n; // 新表
    unsigned long realsize = _dictNextPower(size);

    if (realsize == d->ht[0].size) {
        return DICT_ERR;
    }

    n.size = realsize;
    n.sizemask = realsize - 1;
    n.table = zmalloc(realsize * sizeof(dictEntry *));
    n.used = 0;

    if (d->ht[0].table == NULL) {
        d->ht[0] = n;
        return DICT_OK;
    }

    d->ht[1] = n;
    d->rehashidx = 0;
    return DICT_OK;
}

// 如果需要，则重新调整哈希表大小
static int _dictExpandIfNeeded(dict *d) {
    if (dictIsRehashing(d)) {
        return DICT_OK;
    }

    // 如果哈希表为空，则进行初始化
    if (d->ht[0].size == 0) {
        return dictExpand(d, DICT_HT_INITIAL_SIZE);
    }

    // 需要判断dict_can_resize，或者空间使用比例
    if (d->ht[0].used >= d->ht[0].size &&
        (dict_can_resize || d->ht[0].used / d->ht[0].size > dict_force_resize_ratio)) {
        return dictExpand(d, d->ht[0].size * 2);
    }
    return DICT_OK;
}

/**
 * 获取一个键的索引
 */
static long _dictKeyIndex(dict *d, const void *key, uint64_t hash, dictEntry **existing) {
    unsigned long idx, table;
    dictEntry *he;

    if (_dictExpandIfNeeded(d) == DICT_ERR) {
        return -1;
    }

    for (table = 0; table <= 1; table++) {
        idx = hash & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while(he) {
            if (key == he->key || dictCompareKeys(d, key, he->key)) {
                if (existing) {
                    *existing = he;
                }
                return -1;
            }
            he = he->next;
        }
        if (!dictIsRehashing(d)) {
            break;
        }
    }
    return idx;
}

// 添加值
dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing) {
    long index;
    dictEntry *entry;
    dictht *ht;

    // 先判断是否有rehash操作在进行
    if (dictIsRehashing(d)) {
        _dictRehashStep(d);
    }

    if ((index = _dictKeyIndex(d, key, dictHashKey(d, key), existing)) == -1) {
        return NULL;
    }

    ht = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];

    entry = zmalloc(sizeof(*entry));
    entry->next = ht->table[index];
    ht->table[index] = entry;
    ht->used++;

    dictSetKey(d, entry, key);

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
    if (!entry) {
        return DICT_ERR;
    }

    dictSetVal(d, entry, val);
    return DICT_OK;
}


// 删除键
static dictEntry *dictGenericDelete(dict *d, const void *key, int nofree) {
    uint64_t hash, idx;
    dictEntry *he, *prevHe;
    int table;

    if (d->ht[0].used == 0 && d->ht[1].used == 0) {
        return NULL;
    }

    if (dictIsRehashing(d)) {
        _dictRehashStep(d);
    }

    hash = dictHashKey(d, key);

    for (table = 0; table <= 1; table++) {
        idx = hash & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        prevHe = NULL;

        while(he) {
            if (key == he->key || dictCompareKeys(d, key, he->key)) {
                if (prevHe) {
                    prevHe->next = he->next;
                } else {
                    d->ht[table].table[idx] = he->next;
                }
                if (!nofree) {
                    dictFreeKey(d, he);
                    dictFreeVal(d, he);
                    zfree(he);
                }
                d->ht[table].used--;
                return he;
            }
            prevHe = he;
            he = he->next;
        }
        if (!dictIsRehashing(d)) {
            break;
        }
    }
    return NULL;
}


/**
 * 将给定的键值从字典中删除
 * @param  d    字典
 * @param  key  键
 * @return
 */
int dictDelete(dict *d, const void *key) {
    return dictGenericDelete(d, key, 0) ? DICT_OK : DICT_ERR;
}


// 清空字典的哈希表
static int _dictClear(dict *d, dictht *ht, void(callback)(void *)) {
    unsigned long i;

    // 释放所有元素
    for (i = 0; i < ht->size && ht->used > 0; i++) {
        dictEntry *he, *nextHe;

        if (callback && (i & 65535) == 0) {
            callback(d->privdata);
        }

        if ((he = ht->table[i]) == NULL) {
            continue;
        }

        while(he) {
            nextHe = he->next;
            dictFreeKey(d, he);
            dictFreeVal(d, he);
            zfree(he);
            ht->used--;
            he = nextHe;
        }
    }

    zfree(ht->table);
    _dictReset(ht);

    return DICT_OK;
}


/**
 * 释放字典，以及字典中包含的所有键值对
 * @param  d  字典指针
 * @return void
 */
void dictRelease(dict *d) {
    _dictClear(d, &d->ht[0], NULL);
    _dictClear(d, &d->ht[1], NULL);
    zfree(d);
}


/**
 * [dictFind 查找键]
 * @param  d   [字典指针]
 * @param  key [键]
 * @return     [节点]
 */
dictEntry *dictFind(dict *d, const void *key) {
    dictEntry *he;
    uint64_t h, idx, table;

    if (d->ht[0].used + d->ht[1].used == 0) {
        return NULL;
    }

    if (dictIsRehashing(d)) {
        _dictRehashStep(d);
    }

    h = dictHashKey(d, key);
    for (table = 0; table <= 1; table++) {
        idx = h & d->ht[table].sizemask;
        he = d->ht[table].table[idx];
        while (he) {
            if (key == he->key || dictCompareKeys(d, key, he->key)) {
                return he;
            }
            he = he->next;
        }
        if (!dictIsRehashing(d)) {
            return NULL;
        }
    }
    return NULL;
}


/**
 * 创建一个字典迭代器
 * @param  d  字典指针
 * @return
 */
dictIterator *dictGetIterator(dict *d) {
    dictIterator *iter = zmalloc(sizeof(*iter));

    iter->d = d;
    iter->table = 0;
    iter->index = -1;
    iter->entry = NULL;
    iter->nextEntry = NULL;
    return iter;
}


/**
 * 获取下一个节点
 * @param  iter  字典迭代器指针
 * @return
 */
dictEntry *dictNext(dictIterator *iter) {
    while (1) {
        if (iter->entry == NULL) {
            dictht *ht = &iter->d->ht[iter->table];
            if (iter->index == -1 && iter->table == 0) {
                iter->d->iterators++;
            }
            iter->index++;
            if (iter->index >= ht->size) {
                if (dictIsRehashing(iter->d) && iter->table == 0) {
                    iter->table++;
                    iter->index = 0;
                    ht = &iter->d->ht[1];
                } else {
                    break;
                }
            }
            iter->entry = ht->table[iter->index];
        } else {
            iter->entry = iter->nextEntry;
        }

        if (iter->entry) {
            iter->nextEntry = iter->entry->next;
            return iter->entry;
        }
    }
    return NULL;
}

/**
 * 释放字典迭代器
 * @param  iter  字典迭代器指针
 * @return void
 */
void dictReleaseIterator(dictIterator *iter) {
    if (!(iter->index == -1 && iter->table == 0)) {
        iter->d->iterators--;
    }
    zfree(iter);
}


long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

/* Rehash for an amount of time between ms milliseconds and ms+1 milliseconds */
int dictRehashMilliseconds(dict *d, int ms) {
    long long start = timeInMilliseconds();
    int rehashes = 0;

    while(dictRehash(d,100)) {
        rehashes += 100;
        if (timeInMilliseconds()-start > ms) break;
    }
    return rehashes;
}