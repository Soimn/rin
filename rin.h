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

#include <stdarg.h>
#define R_Arg_List va_list
#define R_VA_START(ARG_LIST, BEFORE_FIRST_ARG) va_start(ARG_LIST, BEFORE_FIRST_ARG)
#define R_VA_ARG(ARG_LIST, ARG_TYPE) va_arg(ARG_LIST, ARG_TYPE)
#define R_VA_END(ARG_LIST) va_end(ARG_LIST)

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
///                      Stings
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

// NOTE: negative past_end indices are mapped to string.size + past_end + 1
R_String
R_String_Slice(R_String string, R_umm start, R_smm past_end)
{
  R_umm chop_amount = string.size - R_MIN(string.size, past_end);
  if (past_end < 0) chop_amount = (R_umm)(-past_end - 1);

  return R_String_ChopN(R_String_EatN(string, start), chop_amount);
}

R_smm // NOTE: -1: no, or erroneous, match, x: length of matched string
R_String_PatternMatch(R_String string, const char* format, ...) // NOTE: Var arg is a list of pointers to locations receiving the parsed input, format is limited to %%, %c, %x[size], %u[size] and %i[size] for now
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
      else if (*scan == 'x' || *scan == 'u' || *scan == 'i')
      {
        R_bool is_signed = (*scan == 'd');
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
            case 8:  *R_VA_ARG(arg_list, R_s8*)  = (R_s8)ivalue; break;
            case 16: *R_VA_ARG(arg_list, R_s16*) = (R_s16)ivalue; break;
            case 32: *R_VA_ARG(arg_list, R_s32*) = (R_s32)ivalue; break;
            case 64: *R_VA_ARG(arg_list, R_s64*) = (R_s64)ivalue; break;
          }
        }
        else
        {
          switch (bit_width)
          {
            case 8:  *R_VA_ARG(arg_list, R_u8*)  = (R_u8)value; break;
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

#endif
