#ifndef __ZMALLOC_H__
#define __ZMALLOC_H__

#include <malloc.h>

#ifndef zrealloc
#define zrealloc realloc
#endif

#ifndef zmalloc
#define zmalloc malloc
#endif

#ifndef zfree
#define zfree free
#endif

#endif