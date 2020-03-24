#ifndef _TYPES_H
#define _TYPES_H

#include <assert.h>
#define NES_ASSERT(n) assert(n)
#define nes_printf(...) fprintf(stderr, __VA_ARGS__)

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long int s64;
typedef unsigned long long int u64;

#endif /* _TYPES_H */
