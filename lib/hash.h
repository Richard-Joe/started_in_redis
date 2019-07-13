#ifndef __HASH_H__
#define __HASH_H__

#include <stdint.h>
#include <stddef.h>

uint64_t siphash(const uint8_t *in, const size_t inlen, const uint8_t *k);

#endif