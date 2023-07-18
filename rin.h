#ifndef RIN_H
#define RIN_H

#if _WIN32
#  define R_PLATFORM_WINDOWS 1
#elif __linux__
#  define R_PLATFORM_LINUX 1
#else
#  error NOT IMPLEMENTED
#endif

#if R_PLATFORM_WINDOWS
#  define STRICT
#  define UNICODE
#  define WIN32_LEAN_AND_MEAN 1
#  define NOMINMAX            1
#  include <windows.h>
#  undef STRICT
#  undef UNICODE
#  undef WIN32_LEAN_AND_MEAN
#  undef NOMINMAX
#  undef far
#  undef near
#endif

#if _MSC_VER
#  define R_COMPILER_MSVC 1
#elif __clang__
#  define R_COMPILER_CLANG 1
#elif __GNUC__
#  define R_COMPILER_GCC 1
#else
#  error NOT IMPLEMENTED
#endif

#if R_COMPILER_MSVC
#  if _M_IX86
#    define R_ARCH_X86 1
#  elif _M_AMD64
#    define R_ARCH_X64 1
#  else
#    error NOT IMPLEMENTED
#  endif
#elif R_COMPILER_GCC || R_COMPILER_CLANG
#  if __i386__
#    define R_ARCH_X86 1
#  elif __x86_64__
#    define R_ARCH_X64 1
#  else
#    error NOT IMPLEMENTED
#  endif
#else
#  error NOT IMPLEMENTED
#endif

#if R_COMPILER_MSVC
#  include <immintrin.h>
#elif R_COMPILER_GCC || R_COMPILER_CLANG
#  include <immintrin.h>
#else
#  error NOT IMPLEMENTED
#endif


#if R_COMPILER_MSVC
typedef signed __int8  R_s8;
typedef signed __int16 R_s16;
typedef signed __int32 R_s32;
typedef signed __int64 R_s64;

typedef unsigned __int8  R_u8;
typedef unsigned __int16 R_u16;
typedef unsigned __int32 R_u32;
typedef unsigned __int64 R_u64;

typedef float R_f32;
typedef double R_f64;
#elif R_COMPILER_GCC || R_COMPILER_CLANG
typedef __INT8_TYPE__  R_s8;
typedef __INT16_TYPE__ R_s16;
typedef __INT32_TYPE__ R_s32;
typedef __INT64_TYPE__ R_s64;

typedef __UINT8_TYPE__  R_u8;
typedef __UINT16_TYPE__ R_u16;
typedef __UINT32_TYPE__ R_u32;
typedef __UINT64_TYPE__ R_u64;

typedef float R_f32;
typedef double R_f64;
#else
#  error NOT IMPLEMENTED
#endif

#define R_U8_MAX  ((R_u8)0xFF)
#define R_U16_MAX ((R_u16)0xFFFF)
#define R_U32_MAX ((R_u32)0xFFFFFFFFUL)
#define R_U64_MAX ((R_u64)0xFFFFFFFFFFFFFFFFULL)

typedef R_u8 R_bool;
#define R_true 1
#define R_false 0

#if R_ARCH_X86
typedef R_s32 R_int;
typedef R_u32 R_uint;
typedef R_s32 R_smm;
typedef R_u32 R_umm;
#  define R_INT_BITS 32
#  define R_UINT_BITS 32
#  define R_SMM_BITS 32
#  define R_UMM_BITS 32
#elif R_ARCH_X64
typedef R_s64 R_int;
typedef R_u64 R_uint;
typedef R_s64 R_smm;
typedef R_u64 R_umm;
#  define R_INT_BITS 64
#  define R_UINT_BITS 64
#  define R_SMM_BITS 64
#  define R_UMM_BITS 64
#else
#  error NOT IMPLEMENTED
#endif

#ifdef R_ENABLE_ASSERT
#  define R_ASSERT(EX) ((EX) ? 1 : *(volatile int*)0)
#else
#  define R_ASSERT(EX)
#endif

#define R_NOT_IMPLEMENTED R_ASSERT(!"R_NOT_IMPLEMENTED")

#define R_OFFSETOF(T, E) ((R_umm)(&((T*)0)->E))
#define R_ALIGNOF(T) R_OFFSETOF(struct { char c; T t; }, t)

#define R_STATIC_ARRAY_SIZE(A) (sizeof(A) / sizeof(0[A]))

#define R_IS_POW2(N) ((N) != 0 && ((N) & ((N) - 1)) == 0)

#define R_KB(N) ((N)*1024ULL)
#define R_MB(N) ((N)*1024ULL*1024ULL)
#define R_GB(N) ((N)*1024ULL*1024ULL*1024ULL)

#define R_MIN(A, B) ((A) < (B) ? (A) : (B))
#define R_MAX(A, B) ((A) > (B) ? (A) : (B))
#define R_ABS(N) ((N) < 0 ? -(N) : (N))

#include <stdarg.h>
#define R_Arg_List va_list
#define R_VA_START(ARG_LIST, BEFORE_FIRST_ARG) va_start(ARG_LIST, BEFORE_FIRST_ARG)
#define R_VA_ARG(ARG_LIST, ARG_TYPE) va_arg(ARG_LIST, ARG_TYPE)
#define R_VA_END(ARG_LIST) va_end(ARG_LIST)

/// ------------------------------------------------------
///                     Bit Hacking
/// ------------------------------------------------------

#if R_COMPILER_MSVC
#  define R_RotL32(N, I) _rotl((N), (I))
#  define R_RotL64(N, I) _rotl64((N), (I))
#else
#  error NOT IMPLEMENTED
#endif

/// ------------------------------------------------------
///                        OS
/// ------------------------------------------------------

// TODO: Rework
R_umm R_OS_PageSize     (void);
void* R_OS_ReserveMemory(void* base, R_umm size);
void* R_OS_CommitMemory (void* base, R_umm size);
void  R_OS_FreeMemory   (void* base, R_umm size);

#if R_PLATFORM_WINDOWS

R_umm
R_OS_PageSize(void)
{
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  return sys_info.dwPageSize;
}

void*
R_OS_ReserveMemory(void* base, R_umm size)
{
  return VirtualAlloc(base, size, MEM_RESERVE, PAGE_READWRITE);
}

void*
R_OS_CommitMemory(void* base, R_umm size)
{
  return VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);
}

void
R_OS_FreeMemory(void* base, R_umm size)
{
  VirtualFree(base, size, MEM_RELEASE);
}

#else
#  error NOT IMPLEMENTED
#endif

/// ------------------------------------------------------
///                      Memory
/// ------------------------------------------------------

R_umm
R_AlignUp(R_umm n, R_umm alignment)
{
  R_ASSERT(R_IS_POW2(alignment));
  return (n + (alignment - 1)) & ~(alignment - 1);
}

R_umm
R_AlignDown(R_umm n, R_umm alignment)
{
  R_ASSERT(R_IS_POW2(alignment));
  return n & ~(alignment - 1);
}

void*
R_Align(void* ptr, R_umm alignment)
{
  return (void*)R_AlignUp((R_umm)ptr, alignment);
}

void
R_MemCopy(void* src, void* dst, R_umm size)
{
  for (R_umm i = 0; i < size; ++i) ((R_u8*)dst)[i] = ((R_u8*)src)[i];
}

void
R_MemMove(void* src, void* dst, R_umm size)
{
  R_u8* bsrc = src;
  R_u8* bdst = dst;

  if (bsrc < bdst) for (R_umm i = 0; i < size; ++i) bdst[(size -  1) - i] = bsrc[(size - 1) - i];
  else             for (R_umm i = 0; i < size; ++i) bdst[i] = bsrc[i];
}

void
R_MemZero(void* ptr, R_umm size)
{
  for (R_umm i = 0; i < size; ++i) ((R_u8*)ptr)[i] = 0;
}

#define R_ZeroStruct(ptr) R_Zero((ptr), sizeof(0[ptr]))
#define R_ZeroArray(ptr, count) R_Zero((ptr), sizeof(0[ptr])*(count))

void
R_MemSet(void* ptr, R_umm size, R_u8 val)
{
  for (R_umm i = 0; i < size; ++i) ((R_u8*)ptr)[i] = val;
}

R_bool
R_MemCmp(void* p0, void* p1, R_umm size)
{
  R_bool result = R_true;

  for (R_umm i = 0; i < size && result; ++i) result = (((R_u8*)p0)[i] == ((R_u8*)p1)[i]);

  return result;
}

typedef struct R_Arena
{
  R_umm cursor;
  R_umm commit_watermark;
  R_umm commit_pace;
  R_umm reserve_size;
  R_umm page_size;
} R_Arena;

typedef R_umm R_Arena_Marker;

R_Arena*       R_Arena_Create     (R_umm reserve_size, R_umm commit_pace);
void           R_Arena_Destroy    (R_Arena* arena);
void*          R_Arena_Push       (R_Arena* arena, R_uint size, R_uint alignment);
void           R_Arena_Pop        (R_Arena* arena, R_uint size);
R_Arena_Marker R_Arena_GetMarker  (R_Arena* arena);
void           R_Arena_PopToMarker(R_Arena* arena, R_Arena_Marker marker);
void           R_Arena_Clear      (R_Arena* arena);
R_umm          R_Arena_Watermark  (R_Arena* arena);

// TODO: Handle allocation failure

R_Arena*
R_Arena_Create(R_umm reserve_size, R_umm commit_pace)
{
  R_umm page_size = R_OS_PageSize();

  reserve_size = R_AlignUp(reserve_size, page_size);
  commit_pace  = R_AlignUp(commit_pace, page_size);

  R_Arena* arena = R_OS_ReserveMemory(0, reserve_size);
  R_OS_CommitMemory(arena, commit_pace);

  *arena = (R_Arena){
    .cursor       = 0,
    .commit_pace  = commit_pace,
    .reserve_size = reserve_size,
    .page_size    = page_size,
  };

  return arena;
}

void
R_Arena_Destroy(R_Arena* arena)
{
  R_OS_FreeMemory(arena, arena->reserve_size);
}

void*
R_Arena_Push(R_Arena* arena, R_uint size, R_uint alignment)
{
  R_ASSERT(alignment <= arena->page_size);

  R_umm adj_cursor = R_AlignUp(arena->cursor, alignment);

  if (size > arena->commit_watermark - adj_cursor)
  {
    R_OS_CommitMemory((R_u8*)arena + arena->commit_watermark, arena->commit_pace);

    arena->commit_watermark += arena->commit_pace;

    R_ASSERT(arena->commit_watermark <= arena->reserve_size);
  }

  arena->cursor = adj_cursor + size;

  return (R_u8*)arena + adj_cursor;
}

void
R_Arena_Pop(R_Arena* arena, R_uint size)
{
  R_ASSERT(size <= arena->cursor);
  arena->cursor -= size;
}

R_Arena_Marker
R_Arena_GetMarker(R_Arena* arena)
{
  return arena->cursor;
}

void
R_Arena_PopToMarker(R_Arena* arena, R_Arena_Marker marker)
{
  R_ASSERT(marker <= arena->cursor);
  arena->cursor = marker;
}

void
R_Arena_Clear(R_Arena* arena)
{
  arena->cursor = 0;
}

R_umm
R_Arena_Watermark(R_Arena* arena)
{
  return arena->commit_watermark;
}

/// ------------------------------------------------------
///                      Strings
/// ------------------------------------------------------

R_bool
R_Char_IsAlpha(R_u8 c)
{
  return ((R_u8)((c & 0xDF) - 'A') <= (R_u8)('Z' - 'A'));
}

R_bool
R_Char_IsAlphaHex(R_u8 c)
{
  return ((R_u8)((c & 0xDF) - 'A') <= (R_u8)('F' - 'A'));
}

R_bool
R_Char_IsDigit(R_u8 c)
{
  return ((R_u8)(c - '0') < 10);
}

R_bool
R_Char_IsUpper(R_u8 c) // NOTE: Assumes the character is alphabetical
{
  return (c < 'a');
}

R_bool
R_Char_IsAlphaUpper(R_u8 c)
{
  return ((R_u8)(c - 'A') <= (R_u8)('Z' - 'A'));
}

R_bool
R_Char_ToUpper(R_u8 c) // NOTE: Assumes the character is alphabetical
{
  return c & 0xDF;
}

R_bool
R_Char_ToAlphaUpper(R_u8 c)
{
  if ((R_u8)(c - 'a') <= (R_u8)('z' - 'a')) c = c & 0xDF;
  return c;
}

R_bool
R_Char_IsLower(R_u8 c) // NOTE: Assumes the character is alphabetical
{
  return (c >= 'a');
}

R_bool
R_Char_IsAlphaLower(R_u8 c)
{
  return ((R_u8)(c - 'a') <= (R_u8)('z' - 'a'));
}

R_bool
R_Char_ToLower(R_u8 c) // NOTE: Assumes the character is alphabetical
{
  return c | 0x20;
}

R_bool
R_Char_ToAlphaLower(R_u8 c)
{
  if ((R_u8)(c - 'a') <= (R_u8)('z' - 'a')) c = c | 0x20;
  return c;
}

typedef struct R_String
{
  R_u8* data;
  R_umm size;
} R_String;

#define R_STRING(S) (R_String){ .data = (S), .size = sizeof(S) - 1 }

R_umm
R_CString_Length(char* cstring)
{
  R_umm length = 0;

  for (char* scan = cstring; *scan != 0; ++scan) ++length;

  return length;
}

R_bool
R_CString_Match(char* cs0, char* cs1)
{
  while (*cs0 != 0 && *cs1 != 0 && *cs0 == *cs1) ++cs0, ++cs1;

  return (*cs0 == *cs1);
}

R_String
R_StringFromCString(char* cstring)
{
  return (R_String){
    .data = cstring,
    .size = R_CString_Length(cstring),
  };
}

R_bool
R_String_Match(R_String s0, R_String s1)
{
  R_bool result;

  if      (s0.size != s1.size) result = R_false;
  else if (s0.data == s1.data) result = R_true;
  else                         result = R_MemCmp(s0.data, s1.data, s0.size);

  return result;
}

R_smm
R_String_FindFirstChar(R_String string, R_u8 c)
{
  R_smm index = -1;

  for (R_umm i = 0; i < string.size; ++i)
  {
    if (string.data[i] == c)
    {
      index = i;
      break;
    }
  }

  return index;
}

R_smm
R_String_FindLastChar(R_String string, R_u8 c)
{
  R_smm index = -1;

  for (R_umm i = 0; i < string.size; ++i)
  {
    if (string.data[(string.size - 1) - i] == c)
    {
      index = i;
      break;
    }
  }

  return index;
}

R_String
R_String_FirstN(R_String string, R_umm n_chars)
{
  return (R_String){
    .data = string.data,
    .size = R_MIN(string.size, n_chars),
  };
}

R_String
R_String_ChopN(R_String string, R_umm n_chars)
{
  return (R_String){
    .data = string.data,
    .size = R_MAX(string.size, n_chars) - n_chars,
  };
}

R_String
R_String_LastN(R_String string, R_umm n_chars)
{
  R_umm new_n_chars = R_MIN(string.size, n_chars);

  return (R_String){
    .data = string.data + (string.size - new_n_chars),
    .size = new_n_chars,
  };
}

R_String
R_String_EatN(R_String string, R_umm n_chars)
{
  return (R_String){
    .data = string.data + n_chars,
    .size = R_MAX(string.size, n_chars) - n_chars,
  };
}

R_smm // NOTE: -1: no, or erroneous, match, x: length of matched string
R_String_PatternMatch(R_String string, const char* format, ...) // NOTE: Var arg is a list of pointers to locations receiving the parsed input, format is limited to %%, %c, %x[size], %u[size] and %s[size] for now
{
  R_Arg_List arg_list;
  R_VA_START(arg_list, format);

  R_smm matched_chars = 0;
  for (char* scan = (char*)format; *scan != 0 && matched_chars != -1; )
  {
    if (scan[0] != '%' || scan[0] != 0 && scan[1] == '%')
    {
      if ((R_umm)matched_chars < string.size && string.data[matched_chars] == *scan)
      {
        matched_chars += 1;
        scan          += 1 + (*scan == '%');
      }
      else
      {
        //// ERROR: Missing characters
        matched_chars = -1;
        break;
      }
    }
    else
    {
      scan += 1;
      R_ASSERT(*scan != 0);

      if (*scan == 'c')
      {
        scan += 1;

        if ((R_umm)matched_chars < string.size)
        {
          *R_VA_ARG(arg_list, R_u8*) = string.data[matched_chars];

          matched_chars += 1;
        }
        else
        {
          //// ERROR: Missing characters
          matched_chars = -1;
          break;
        }
      }
      else if (*scan == 'x' || *scan == 'u' || *scan == 's')
      {
        R_bool is_signed = (*scan == 's');
        R_bool is_hex    = (*scan == 'x');
        R_uint bit_width;

        scan += 1;
        R_ASSERT(*scan != 0);

        if (*scan == '8')
        {
          bit_width = 8;
          scan += 1;
        }
        else if (*scan == '1')
        {
          R_ASSERT(scan[1] == '6');
          bit_width = 16;
          scan += 2;
        }
        else if (*scan == '3')
        {
          R_ASSERT(scan[1] == '2');
          bit_width = 32;
          scan += 2;
        }
        else
        {
          R_ASSERT(scan[0] == '6' && scan[1] == '4');
          bit_width = 64;
          scan += 2;
        }

        R_int sign   = 1;
        R_uint value = 0;

        if ((R_umm)matched_chars < string.size && string.data[matched_chars] == '-')
        {
          if (is_signed)
          {
            sign           = -1;
            matched_chars += 1;
          }
          else
          {
            //// ERROR: Sign mismatch
            matched_chars = -1;
            break;
          }
        }

        R_uint digit_count  = 0;
        R_bool did_overflow = R_false;
        for (;;)
        {
          R_u8 c = 0;
          if ((R_umm)matched_chars >= string.size) break;
          else
          {
            c = string.data[matched_chars];

            R_uint digit;
            if      (R_Char_IsDigit(c))    digit = c & 0xF;
            else if (R_Char_IsAlphaHex(c)) digit = (c & 0x1F) + 9;
            else                           break;

            matched_chars += 1;
            digit_count   += 1;

            if (is_hex)
            {
              did_overflow = (did_overflow || value >> (R_UMM_BITS - 4) != 0);
              value = (value << 4) | digit;
            }
            else
            {
              R_uint value_x10 = value*10;
              R_uint new_value = value_x10 + digit;

              did_overflow = (did_overflow || value_x10 / 10 != value || new_value < value);
              value = new_value;
            }
          }
        }

        if (did_overflow || value >= (1ULL << (bit_width - 1 - is_signed)))
        {
          //// ERROR: Input number too large in magnitude
          matched_chars = -1;
          break;
        }

        if (is_signed)
        {
          R_int ivalue = (sign == 1 ? (R_int)value : -(R_int)value);
          switch (bit_width)
          {
            case 8:  *R_VA_ARG(arg_list, R_s8*)  = (R_s8)ivalue;  break;
            case 16: *R_VA_ARG(arg_list, R_s16*) = (R_s16)ivalue; break;
            case 32: *R_VA_ARG(arg_list, R_s32*) = (R_s32)ivalue; break;
            case 64: *R_VA_ARG(arg_list, R_s64*) = (R_s64)ivalue; break;
          }
        }
        else
        {
          switch (bit_width)
          {
            case 8:  *R_VA_ARG(arg_list, R_u8*)  = (R_u8)value;  break;
            case 16: *R_VA_ARG(arg_list, R_u16*) = (R_u16)value; break;
            case 32: *R_VA_ARG(arg_list, R_u32*) = (R_u32)value; break;
            case 64: *R_VA_ARG(arg_list, R_u64*) = (R_u64)value; break;
          }
        }
      }
      else R_NOT_IMPLEMENTED;
    }
  }

  R_VA_END(arg_list);

  return matched_chars;
}

/// ------------------------------------------------------
///                      Math
/// ------------------------------------------------------

typedef union R_F32_Bits
{
  R_f32 f;
  R_u32 bits;
} R_F32_Bits;

typedef union R_F64_Bits
{
  R_f64 f;
  R_u64 bits;
} R_F64_Bits;

#define R_SQUARED(X) ((X)*(X))
#define R_CUBED(X) ((X)*(X)*(X))

// TODO: Test
inline R_f32
R_Abs(R_f32 n)
{
#if 0
  return (n < 0 ? -n : n);
#else
  return (R_F32_Bits){ .bits = (R_F32_Bits){ .f = n }.bits & 0x7FFFFFFF }.f;
#endif
}

inline R_f32
R_LogFloor(R_f32 n)
{
  R_u32 biased_exponent = ((R_F32_Bits){ .f = n }.bits >> 23) & 0x7FFFFF;
  return (biased_exponent == 0 ? -126 : (R_f32)biased_exponent);
}

inline R_f32
R_Sqrt(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

inline R_f32
R_RSqrt(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

inline R_f32
R_Sin(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

inline R_f32
R_Cos(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

// TODO: SinCos?

inline R_f32
R_Tan(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

inline R_f32
R_Tan2(R_f32 n)
{
  R_NOT_IMPLEMENTED;
}

typedef union R_V2
{
  struct { R_f32 x, y; };
  R_f32 e[2];
} R_V2;

#define R_V2(X, Y) (R_V2){ .x = (X), .y = (Y) }

inline R_V2
R_V2_Add(R_V2 a, R_V2 b)
{
  return R_V2(a.x + b.x, a.y + b.y);
}

inline R_V2
R_V2_Sub(R_V2 a, R_V2 b)
{
  return R_V2(a.x - b.x, a.y - b.y);
}

inline R_V2
R_V2_Scale(R_V2 v, R_f32 n)
{
  return R_V2(v.x*n, v.y*n);
}

inline R_V2
R_V2_Hadamard(R_V2 a, R_V2 b)
{
  return R_V2(a.x*b.x, a.y*b.y);
}

inline R_f32
R_V2_Inner(R_V2 a, R_V2 b)
{
  return a.x*b.x + a.y*b.y;
}

inline R_f32
R_V2_LengthSq(R_V2 v)
{
  return R_SQUARED(v.x) + R_SQUARED(v.y);
}

inline R_f32
R_V2_Length(R_V2 v)
{
  return R_RSqrt(R_V2_LengthSq(v));
}

inline R_V2
R_V2_Normalize(R_V2 v)
{
  return R_V2_Scale(v, R_V2_Length(v));
}

typedef union R_V2S
{
  struct { R_s32 x, y; };
  R_s32 e[2];
} R_V2S;

#define R_V2S(X, Y) (R_V2S){ .x = (X), .y = (Y) }

inline R_V2S
R_V2S_Add(R_V2S a, R_V2S b)
{
  return R_V2S(a.x + b.x, a.y + b.y);
}

inline R_V2S
R_V2S_Sub(R_V2S a, R_V2S b)
{
  return R_V2S(a.x - b.x, a.y - b.y);
}

inline R_V2S
R_V2S_Scale(R_V2S v, R_s32 n)
{
  return R_V2S(v.x*n, v.y*n);
}

inline R_V2S
R_V2S_Hadamard(R_V2S a, R_V2S b)
{
  return R_V2S(a.x*b.x, a.y*b.y);
}

inline R_s32
R_V2S_Inner(R_V2S a, R_V2S b)
{
  return a.x*b.x + a.y*b.y;
}

inline R_u64
R_V2S_PackToU64(R_V2S v)
{
  return ((R_u64)v.x << 32) | (R_u64)v.y;
}

inline R_V2S
R_V2S_UnpackFromU64(R_u64 n)
{
  return R_V2S((R_s32)(n >> 32), (R_s32)(n & R_U32_MAX));
}

typedef union R_V3
{
  struct { R_f32 x, y, z; };
  struct { R_V2 xy; R_f32 _0; };
  struct { R_f32 _1; R_V2 yz; };
  struct { R_f32 r, g, b; };
  struct { R_V2 rg; R_f32 _2; };
  struct { R_f32 _3; R_V2 gb; };
  R_f32 e[3];
} R_V3;

#define R_V3(X, Y, Z) (R_V3){ .x = (X), .y = (Y), .z = (Z) }

inline R_V3
R_V3_Add(R_V3 a, R_V3 b)
{
  return R_V3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline R_V3
R_V3_Sub(R_V3 a, R_V3 b)
{
  return R_V3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline R_V3
R_V3_Scale(R_V3 v, R_f32 n)
{
  return R_V3(v.x*n, v.y*n, v.z*n);
}

inline R_V3
R_V3_Hadamard(R_V3 a, R_V3 b)
{
  return R_V3(a.x*b.x, a.y*b.y, a.z*b.z);
}

inline R_f32
R_V3_Inner(R_V3 a, R_V3 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline R_V3
R_V3_Cross(R_V3 a, R_V3 b)
{
  return R_V3(a.y*b.z - a.z*b.y, a.x*b.z - a.z*b.x, a.x*b.y - a.y*b.x);
}

inline R_f32
R_V3_LengthSq(R_V3 v)
{
  return R_SQUARED(v.x) + R_SQUARED(v.y) + R_SQUARED(v.z);
}

inline R_f32
R_V3_Length(R_V3 v)
{
  return R_RSqrt(R_V3_LengthSq(v));
}

inline R_f32
R_V3_TripleProduct(R_V3 a, R_V3 b, R_V3 c)
{
  return R_V3_Inner(a, R_V3_Cross(b, c));
}

inline R_V3
R_V3_Normalize(R_V3 v)
{
  return R_V3_Scale(v, R_V3_Length(v));
}

typedef union R_V4
{
  struct { R_f32 x, y, z, w; };
  struct { R_V3 xyz; R_f32 _0; };
  struct { R_f32 _1; R_V3 yzw; };
  struct { R_V2 xy; R_V2 zw; };
  struct { R_f32 _2; R_V2 yz; R_f32 _3; };
  struct { R_f32 r, g, b, a; };
  struct { R_V3 rgb; R_f32 _4; };
  struct { R_f32 _5; R_V3 gba; };
  struct { R_V2 rg; R_V2 ba; };
  struct { R_f32 _6; R_V2 gb; R_f32 _7; };
  R_f32 e[4];
} R_V4;

#define R_V4(X, Y, Z, W) (R_V4){ .x = (X), .y = (Y), .z = (Z), .w = (W) }

inline R_V4
R_V4_Add(R_V4 a, R_V4 b)
{
  return R_V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline R_V4
R_V4_Sub(R_V4 a, R_V4 b)
{
  return R_V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline R_V4
R_V4_Scale(R_V4 v, R_f32 n)
{
  return R_V4(v.x*n, v.y*n, v.z*n, v.w*n);
}

inline R_V4
R_V4_Hadamard(R_V4 a, R_V4 b)
{
  return R_V4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}

inline R_f32
R_V4_Inner(R_V4 a, R_V4 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

inline R_f32
R_V4_LengthSq(R_V4 v)
{
  return R_SQUARED(v.x) + R_SQUARED(v.y) + R_SQUARED(v.z) + R_SQUARED(v.w);
}

inline R_f32
R_V4_Length(R_V4 v)
{
  return R_RSqrt(R_V4_LengthSq(v));
}

inline R_V4
R_V4_Normalize(R_V4 v)
{
  return R_V4_Scale(v, R_V4_Length(v));
}

/// ------------------------------------------------------
///                     Hash Functions
/// ------------------------------------------------------

// NOTE: The following function (R_MurmurHash3_x64_128) is a copy of MurmurHash3
//       with slight modifications (mostly renaming types and inlining utility functions).
//       The license notice for MurmurHash3 is included below
//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

void
R_MurmurHash3_x64_128(R_String key, R_u32 seed, R_u64 out[2])
{
  R_u8* data = key.data;
  R_int nblocks = key.size / 16;

  R_u64 h1 = seed;
  R_u64 h2 = seed;

  R_u64 c1 = 0x87C37B91114253D5ULL;
  R_u64 c2 = 0x4CF5AD432745937FULL;

  //----------
  // body

  R_u64* blocks = (R_u64*)data;

  for(R_uint i = 0; i < nblocks; ++i)
  {
    R_u64 k1 = blocks[i*2+0];
    R_u64 k2 = blocks[i*2+1];

    k1 *= c1; k1 = R_RotL64(k1,31); k1 *= c2; h1 ^= k1;

    h1 = R_RotL64(h1,27); h1 += h2; h1 = h1*5+0x52DCE729;

    k2 *= c2; k2  = R_RotL64(k2,33); k2 *= c1; h2 ^= k2;

    h2 = R_RotL64(h2,31); h2 += h1; h2 = h2*5+0x38495AB5;
  }

  //----------
  // tail

  R_u8* tail = (R_u8*)(data + nblocks*16);

  R_u64 k1 = 0;
  R_u64 k2 = 0;

  switch(key.size & 15)
  {
  case 15: k2 ^= ((R_u64)tail[14]) << 48;
  case 14: k2 ^= ((R_u64)tail[13]) << 40;
  case 13: k2 ^= ((R_u64)tail[12]) << 32;
  case 12: k2 ^= ((R_u64)tail[11]) << 24;
  case 11: k2 ^= ((R_u64)tail[10]) << 16;
  case 10: k2 ^= ((R_u64)tail[ 9]) << 8;
  case  9: k2 ^= ((R_u64)tail[ 8]) << 0;
           k2 *= c2; k2  = R_RotL64(k2,33); k2 *= c1; h2 ^= k2;

  case  8: k1 ^= ((R_u64)tail[ 7]) << 56;
  case  7: k1 ^= ((R_u64)tail[ 6]) << 48;
  case  6: k1 ^= ((R_u64)tail[ 5]) << 40;
  case  5: k1 ^= ((R_u64)tail[ 4]) << 32;
  case  4: k1 ^= ((R_u64)tail[ 3]) << 24;
  case  3: k1 ^= ((R_u64)tail[ 2]) << 16;
  case  2: k1 ^= ((R_u64)tail[ 1]) << 8;
  case  1: k1 ^= ((R_u64)tail[ 0]) << 0;
           k1 *= c1; k1  = R_RotL64(k1,31); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= key.size; h2 ^= key.size;

  h1 += h2;
  h2 += h1;


  h1 ^= h1 >> 33;
  h1 *= 0xFF51AFD7ED558CCDULL;
  h1 ^= h1 >> 33;
  h1 *= 0xC4CEB9FE1A85EC53ULL;
  h1 ^= h1 >> 33;

  h2 ^= h2 >> 33;
  h2 *= 0xFF51AFD7ED558CCDULL;
  h2 ^= h2 >> 33;
  h2 *= 0xC4CEB9FE1A85EC53ULL;
  h2 ^= h2 >> 33;

  h1 += h2;
  h2 += h1;

  out[0] = h1;
  out[1] = h2;
}

#if 0
/// ------------------------------------------------------
///                     Hash Table
/// ------------------------------------------------------

typedef struct R_Hash_Table_U64_U64__Slot
{
  R_u64 hash;
  R_u64 value;
} R_Hash_Table_U64_U64__Slot;

typedef struct R_Hash_Table_U64_U64
{
  R_Virtual_Array* keys;
  R_Virtual_Array* slots;
} R_Hash_Table_U64_U64;

R_Hash_Table_U64_U64
R_HashTableU64U64_Create(R_uint init_size)
{
}

void
R_HashTableU64U64_Destroy(R_HashTableU64U64* table)
{
}

R_u64
R_HashTableU64U64_Get(R_HashTableU64U64* table, R_u64 key)
{
}

void
R_HashTableU64U64_Put(R_HashTableU64U64* table, R_u64 key, R_u64 value)
{
}

R_uint
R_HashTableU64U64_KeyCount(R_Hash_Table_U64_U64* table)
{
  return R_VArray_Size(table->keys);
}
#endif

#endif
