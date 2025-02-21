#pragma once

#include <stdint.h>
#include <stddef.h>

namespace CE
{
    
}

typedef bool b8;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef char c8;
typedef char16_t c16;
typedef char32_t c32;
typedef bool b8;

typedef intptr_t IntPtr;
typedef uintptr_t UintPtr;

typedef intptr_t PtrDiff;

#if IS_64BIT
    typedef size_t SIZE_T;
    typedef s64 SSIZE_T;
#else
    typedef size_t SIZE_T;
    typedef s32 SSIZE_T;
#endif
