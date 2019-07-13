#include <limits.h>
#include <string.h>

#include "sds.h"
#include "zmalloc.h"

/*
 * 获取sds头部大小
 *
 * @param type 类型
 * @return 头部大小
 */
static inline int sdsHdrSize(char type) {
    switch (type & SDS_TYPE_MASK) {
        case SDS_TYPE_8:
            return SIZEOF_SDS_HDR(8);
        case SDS_TYPE_16:
            return SIZEOF_SDS_HDR(16);
        case SDS_TYPE_32:
            return SIZEOF_SDS_HDR(32);
        case SDS_TYPE_64:
            return SIZEOF_SDS_HDR(64);
    }
    return 0;
}


/*
 * 根据字符串的大小获取头部类型
 *
 * @param string_size 字符串的大小
 * @return 头部类型
 */
static inline char sdsReqType(size_t string_size) {
    if (string_size < 1<<8)
        return SDS_TYPE_8;
    if (string_size < 1<<16)
        return SDS_TYPE_16;
#if (LONG_MAX == LLONG_MAX)
    if (string_size < 1ll<<32)
        return SDS_TYPE_32;
    return SDS_TYPE_64;
#else
    return SDS_TYPE_32;
#endif
}


/*
 * 根据字符串创建sds字符串
 *
 * @param init 初始字符串
 * @param initlen 初始字符串的长度
 * @return sds
 */
sds sdsnewlen(const void *init, size_t initlen) {
    char type = sdsReqType(initlen);
    int hdrlen = sdsHdrSize(type);
    int memsize = hdrlen + initlen + 1;

    void *ptr = zmalloc(memsize);
    if (ptr == NULL)
        return NULL;

    if (init == SDS_NOINIT)
        init = NULL;
    else if (!init)
        memset(ptr, 0, memsize);

    sds s = (char *)ptr + hdrlen;
    unsigned char *fp = ((unsigned char *)s - 1); /* flags pointer. */

    switch (type) {
        case SDS_TYPE_8: {
            SDS_HDR_VAR(8, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case SDS_TYPE_16: {
            SDS_HDR_VAR(16, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case SDS_TYPE_32: {
            SDS_HDR_VAR(32, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
        case SDS_TYPE_64: {
            SDS_HDR_VAR(64, s);
            sh->len = initlen;
            sh->alloc = initlen;
            *fp = type;
            break;
        }
    }

    if (initlen && init)
        memcpy(s, init, initlen);
    s[initlen] = '\0';
    return s;
}


/*
 * 根据字符串创建sds字符串
 *
 * @param init 初始字符串
 * @return sds
 */
sds sdsnew(const char *init) {
    size_t initlen = (init == NULL) ? 0: strlen(init);
    return sdsnewlen(init, initlen);
}


/*
 * 创建空的sds字符串
 *
 * @param void
 * @return sds
 */
sds sdsempty(void) {
    return sdsnewlen("", 0);
}



/*
 * 释放sds字符串
 *
 * @param s sds字符串
 * @return
 */
void sdsfree(sds s) {
    if (s == NULL)
        return;
    zfree((char *)s - sdsHdrSize(s[-1]));
}


/*
 * 扩充sds的空间
 *
 * @param s sds字符串
 * @param addlen 扩充长度
 * @return
 */
sds sdsMakeRoomFor(sds s, size_t addlen) {
    size_t avail = sdsavail(s);
    if (avail >= addlen)
        return s;

    size_t len = sdslen(s);
    size_t newlen = len + addlen;
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;

    void *newsh;
    char oldtype = s[-1] & SDS_TYPE_MASK;
    void *sh = (char *)s - sdsHdrSize(oldtype);
    char type = sdsReqType(newlen);
    int hdrlen = sdsHdrSize(type);
    if (oldtype == type) {
        newsh = zrealloc(sh, hdrlen + newlen + 1);
        if (newsh == NULL)
            return NULL;
        s = (char *)newsh + hdrlen;
    } else {
        newsh = zmalloc(hdrlen + newlen + 1);
        if (newsh == NULL)
            return NULL;
        memcpy(newsh + hdrlen, s, len + 1);
        zfree(sh);
        s = (char *)newsh + hdrlen;
        s[-1] = type;
        sdssetlen(s, len);
    }
    sdssetalloc(s, newlen);
    return s;
}


/*
 * 拼接字符串
 *
 * @param s sds字符串
 * @param t binary-safe字符串
 * @param len 字符串长度
 * @return
 */
sds sdscatlen(sds s, const void *t, size_t len) {
    size_t curlen = sdslen(s);

    s = sdsMakeRoomFor(s, len);
    if (s == NULL)
        return NULL;

    memcpy(s + curlen, t, len);
    sdssetlen(s, curlen + len);
    s[curlen + len] = '\0';
    return s;
}


/*
 * 拼接字符串
 *
 * @param s sds字符串
 * @param t binary-safe字符串
 * @return
 */
sds sdscat(sds s, const char *t) {
    return sdscatlen(s, t, strlen(t));
}


/*
 * 拼接字符串
 *
 * @param s sds字符串
 * @param t sds字符串
 * @return
 */
sds sdscatsds(sds s, const sds t) {
    return sdscatlen(s, t, sdslen(t));
}


/*
 * 拷贝字符串
 *
 * @param s sds字符串
 * @param t binary-safe字符串
 * @param len 字符串长度
 * @return
 */
sds sdscpylen(sds s, const char *t, size_t len) {
    if (sdsalloc(s) < len) {
        s = sdsMakeRoomFor(s, len);
        if (s == NULL)
            return NULL;
    }

    memcpy(s, t, len);
    s[len] = '\0';
    sdssetlen(s, len);
    return s;
}


/*
 * 拷贝字符串
 *
 * @param s sds字符串
 * @param t binary-safe字符串
 * @return
 */
sds sdscpy(sds s, const char *t) {
    return sdscpylen(s, t, strlen(t));
}


sds sdscatvprintf(sds s, const char *fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = zmalloc(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) zfree(buf);
            buflen *= 2;
            buf = zmalloc(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SDS string and return it. */
    t = sdscat(s, buf);
    if (buf != staticbuf) zfree(buf);
    return t;
}


sds sdscatprintf(sds s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = sdscatvprintf(s,fmt,ap);
    va_end(ap);
    return t;
}

#define SDS_LLSTR_SIZE 21
int sdsll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}


/**
 * [sdsfromlonglong 从长整型数创建sds字符串]
 * @param  value [长整型数]
 * @return       [sds字符串]
 */
sds sdsfromlonglong(long long value) {
    char buf[SDS_LLSTR_SIZE];
    int len = sdsll2str(buf, value);

    return sdsnewlen(buf, len);
}