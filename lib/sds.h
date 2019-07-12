#ifndef __SDS_H__
#define __SDS_H__

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>


#define SDS_MAX_PREALLOC (1024*1024)
const char *SDS_NOINIT;

typedef char *sds;

/* __attribute__ ((__packed__))
 * 让结构体按照紧凑排列的方式，占用内存，达到节省内存的占用
*/
#define SDS_STRUCT(T) \
    struct __attribute__ ((__packed__)) sdshdr##T { \
        uint##T##_t len; \
        uint##T##_t alloc; \
        unsigned char flags; \
        char buf[]; \
    };

/*
 * 定义四种类型sds头部结构
 */
SDS_STRUCT(8)
SDS_STRUCT(16)
SDS_STRUCT(32)
SDS_STRUCT(64)

#define SDS_TYPE_8  1
#define SDS_TYPE_16 2
#define SDS_TYPE_32 3
#define SDS_TYPE_64 4
#define SDS_TYPE_MASK 7

#define SIZEOF_SDS_HDR(T) (sizeof(struct sdshdr##T))
#define SDS_HDR(T, s) ((struct sdshdr##T *)((s) - SIZEOF_SDS_HDR(T)))
#define SDS_HDR_VAR(T, s) struct sdshdr##T *sh = (void*)((s) - SIZEOF_SDS_HDR(T));


static inline size_t sdslen(const sds s) {
    unsigned char flags = s[-1];
    switch (flags & SDS_TYPE_MASK) {
        case SDS_TYPE_8:
            return SDS_HDR(8, s)->len;
        case SDS_TYPE_16:
            return SDS_HDR(16, s)->len;
        case SDS_TYPE_32:
            return SDS_HDR(32, s)->len;
        case SDS_TYPE_64:
            return SDS_HDR(64, s)->len;
    }
    return 0;
}


static inline size_t sdssetlen(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & SDS_TYPE_MASK) {
        case SDS_TYPE_8:
            return SDS_HDR(8, s)->len = newlen;
        case SDS_TYPE_16:
            return SDS_HDR(16, s)->len = newlen;
        case SDS_TYPE_32:
            return SDS_HDR(32, s)->len = newlen;
        case SDS_TYPE_64:
            return SDS_HDR(64, s)->len = newlen;
    }
    return 0;
}


static inline size_t sdsalloc(const sds s) {
    unsigned char flags = s[-1];
    switch (flags & SDS_TYPE_MASK) {
        case SDS_TYPE_8:
            return SDS_HDR(8, s)->alloc;
        case SDS_TYPE_16:
            return SDS_HDR(16, s)->alloc;
        case SDS_TYPE_32:
            return SDS_HDR(32, s)->alloc;
        case SDS_TYPE_64:
            return SDS_HDR(64, s)->alloc;
    }
    return 0;
}


static inline size_t sdssetalloc(sds s, size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & SDS_TYPE_MASK) {
        case SDS_TYPE_8:
            return SDS_HDR(8, s)->alloc = newlen;
        case SDS_TYPE_16:
            return SDS_HDR(16, s)->alloc = newlen;
        case SDS_TYPE_32:
            return SDS_HDR(32, s)->alloc = newlen;
        case SDS_TYPE_64:
            return SDS_HDR(64, s)->alloc = newlen;
    }
    return 0;
}


static inline size_t sdsavail(const sds s) {
    unsigned char flags = s[-1];
    switch (flags & SDS_TYPE_MASK) {
        case SDS_TYPE_8: {
            SDS_HDR_VAR(8, s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_16: {
            SDS_HDR_VAR(16, s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_32: {
            SDS_HDR_VAR(32, s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_64: {
            SDS_HDR_VAR(64, s);
            return sh->alloc - sh->len;
        }
    }
    return 0;
}


sds sdsnewlen(const void *init, size_t initlen);
sds sdsnew(const char *init);
sds sdsempty(void);
void sdsfree(sds s);
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);
sds sdscatsds(sds s, const sds t);
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
/*
 * format属性告诉编译器，按照printf, scanf等标准C函数参数格式规则对该函数的参数进行检查
 */
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif


#endif