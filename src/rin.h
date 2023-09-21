#define CONCAT__(A, B) A##B
#define CONCAT_(A, B) CONCAT__(A, B)
#define CONCAT(A, B) CONCAT_(A, B)

#define ASSERT(E) ((E) ? 1 : *(volatile int*)0)
#define STATIC_ASSERT(E) static struct { int static_ass : ((E) ? 1 : -1); } CONCAT(StaticAssert_, __LINE__)
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

STATIC_ASSERT(sizeof(s8)  == 1);
STATIC_ASSERT(sizeof(s16) == 2);
STATIC_ASSERT(sizeof(s32) == 4);
STATIC_ASSERT(sizeof(s64) == 8);

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

STATIC_ASSERT(sizeof(u8)  == 1);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(u64) == 8);

typedef s64 sint;
typedef u64 uint;

typedef s64 smm;
typedef u64 umm;

typedef float f32;
typedef double f64;

STATIC_ASSERT(sizeof(f32) == 4);
STATIC_ASSERT(sizeof(f64) == 8);

typedef u8 bool;
#define false 0
#define true 1

#define ARRAY_SIZE(A) (sizeof(A)/((sizeof(A)%sizeof(0[A]) == 0)*sizeof(0[A])))
#define OFFSETOF(T, E) (u8)(&((T*)0)->E)
#define ALIGNOF(T) OFFSETOF(struct { char c; T t; }, t)

#define IS_POW2(N) ((N) != 0 && (((N)-1)&(N)) == 0)

typedef struct String
{
	u8* data;
	umm size;
} String;

#define STRING(S) (String){ .data = (u8*)(S), .size = sizeof(S)-1 }
#define MSF_STRING(S) { .data = (u8*)(S), .size = sizeof(S)-1 }

bool
Char_IsAlpha(u8 c)
{
	return (u8)((c&0xDF) - 'A') <= (u8)('Z'-'A');
}

bool
Char_IsHexAlpha(u8 c)
{
	return (u8)((c&0xDF) - 'A') <= (u8)('F'-'A');
}

bool
Char_IsDigit(u8 c)
{
	return (u8)(c - '0') < (u8)(10);
}

umm
Strlen(u8* cstring)
{
	umm result = 0;
	if (cstring) for (; cstring[result] != 0; ++result);

	return result;
}

String
String_FromCString(u8* cstring)
{
	return (String){
		.data = cstring,
		.size = Strlen(cstring),
	};
}

bool
String_Match(String s0, String s1)
{
	bool result = (s0.size == s1.size);

	for (umm i = 0; i < s0.size && result; ++i) result = (s0.data[i] == s1.data[i]);

	return result;
}

String
Sting_EatN(String s, umm n)
{
	return (n >= s.size ? (String){0} : (String){ .data = s.data + n, .size = s.size - n });
}

String
String_ChopN(String s, umm n)
{
	return (n >= s.size ? (String){0} : (String){ .data = s.data, .size = s.size - n });
}

umm
AlignUp(umm n, u8 alignment)
{
	ASSERT(IS_POW2(alignment));
	return (n + (alignment-1)) & ~(alignment-1);
}

umm
AlignDown(umm n, u8 alignment)
{
	ASSERT(IS_POW2(alignment));
	return n & ~(alignment-1);
}

void*
Align(void* ptr, u8 alignment)
{
	return (void*)AlignUp((umm)ptr, alignment);
}

void
Copy(void* src, void* dst, umm size)
{
	for (umm i = 0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}

void
Move(void* src, void* dst, umm size)
{
	u8* bsrc = (u8*)src;
	u8* bdst = (u8*)dst;

	if (bsrc > bdst) for (umm i = 0;      i < size; ++i) bdst[i] = bsrc[i];
	else             for (umm i = size-1; i < size; --i) bdst[i] = bsrc[i];
}

void
Zero(void* base, umm size)
{
	for (umm i = 0; i < size; ++i) ((u8*)base)[i] = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(S))
#define ZeroArray(A, C) Zero((A), sizeof(0[A])*(C))

typedef union F32_Bits
{
	f32 flt;
	u32 bits;
} F32_Bits;

typedef union F64_Bits
{
	f64 flt;
	u64 bits;
} F64_Bits;

typedef union V2
{
	struct { f32 x, y; };
	f32 e[2];
} V2;

typedef union V3
{
	struct { f32 x, y, z; };
	struct { V2 xy; f32 _0; };
	struct { f32 _1; V2 yz; };
	f32 e[3];
} V3;

typedef struct V4
{
	struct { f32 x, y, z, w; };
	struct { V2 xy; V2 zw; };
	struct { V3 xyz; f32 _0; };
	struct { f32 _1; V3 yzw; };
	struct { f32 _2; V2 yz; };
	f32 e[4];
} V4;

typedef union V2S
{
	struct { s32 x, y; };
	s32 e[2];
} V2S;

typedef union V3S
{
	struct { s32 x, y, z; };
	struct { V2S xy; s32 _0; };
	struct { s32 _1; V2S yz; };
	s32 e[3];
} V3S;

typedef struct V4S
{
	struct { s32 x, y, z, w; };
	struct { V2S xy; V2S zw; };
	struct { V3S xyz; s32 _0; };
	struct { s32 _1; V3S yzw; };
	struct { s32 _2; V2S yz; };
	s32 e[4];
} V4S;

typedef union M2
{
	struct { V2 i, j; };
	V2 col[2];
	f32 e[4];
} M2;

typedef union M3
{
	struct { V3 i, j, k; };
	V3 col[3];
	f32 e[9];
} M3;

typedef union M4
{
	struct { V4 i, j, k, l; };
	V4 col[4];
	f32 e[16];
} M4;

#define V2(X, Y)        (V2){ .x = (X), .y = (Y) }
#define V3(X, Y, Z)     (V3){ .x = (X), .y = (Y), .z = (Z) }
#define V4(X, Y, Z, W)  (V4){ .x = (X), .y = (Y), .z = (Z), .w = (W) }
#define V2S(X, Y)       (V2S){ .x = (X), .y = (Y) }
#define V3S(X, Y, Z)    (V3S){ .x = (X), .y = (Y), .z = (Z) }
#define V4S(X, Y, Z, W) (V4S){ .x = (X), .y = (Y), .z = (Z), .w = (W) }

#define M2(I, J)        (M2){ .i = (I), .j = (J) }
#define M3(I, J, K)     (M3){ .i = (I), .j = (J), .k = (K) }
#define M4(I, J, K, L)  (M4){ .i = (I), .j = (J), .k = (K), .l = (L) }

V2  V2_Set1     (f32 n)          { return V2(n, n);                                                                           }
V3  V3_Set1     (f32 n)          { return V3(n, n, n);                                                                        }
V4  V4_Set1     (f32 n)          { return V4(n, n, n, n);                                                                     }
V2S V2S_Set1    (s32 n)          { return V2S(n, n);                                                                          }
V3S V3S_Set1    (s32 n)          { return V3S(n, n, n);                                                                       }
V4S V4S_Set1    (s32 n)          { return V4S(n, n, n, n);                                                                    }
V2  V2_FromV2S  (V2S v)          { return V2((f32)v.x, (f32)v.y);                                                             }
V3  V3_FromV3S  (V3S v)          { return V3((f32)v.x, (f32)v.y, (f32)v.z);                                                   }
V4  V4_FromV4S  (V4S v)          { return V4((f32)v.x, (f32)v.y, (f32)v.z, (f32)v.w);                                         }
V2S V2S_FromV2  (V2 v)           { return V2S((s32)v.x, (s32)v.y);                                                            }
V3S V3S_FromV3  (V3 v)           { return V3S((s32)v.x, (s32)v.y, (s32)v.z);                                                  }
V4S V4S_FromV4  (V4 v)           { return V4S((s32)v.x, (s32)v.y, (s32)v.z, (s32)v.w);                                        }
V2  V2_Add      (V2 v0, V2 v1)   { return V2(v0.x + v1.x, v0.y + v1.y);                                                       }
V3  V3_Add      (V3 v0, V3 v1)   { return V3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);                                          }
V4  V4_Add      (V4 v0, V4 v1)   { return V4(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w);                             }
V2S V2S_Add     (V2S v0, V2S v1) { return V2S(v0.x + v1.x, v0.y + v1.y);                                                      }
V3S V3S_Add     (V3S v0, V3S v1) { return V3S(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);                                         }
V4S V4S_Add     (V4S v0, V4S v1) { return V4S(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w);                            }
V2  V2_Sub      (V2 v0, V2 v1)   { return V2(v0.x - v1.x, v0.y - v1.y);                                                       }
V3  V3_Sub      (V3 v0, V3 v1)   { return V3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);                                          }
V4  V4_Sub      (V4 v0, V4 v1)   { return V4(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w);                             }
V2S V2S_Sub     (V2S v0, V2S v1) { return V2S(v0.x - v1.x, v0.y - v1.y);                                                      }
V3S V3S_Sub     (V3S v0, V3S v1) { return V3S(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);                                         }
V4S V4S_Sub     (V4S v0, V4S v1) { return V4S(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w);                            }
V2  V2_Hadamard (V2 v0, V2 v1)   { return V2(v0.x*v1.x, v0.y*v1.y);                                                           }
V3  V3_Hadamard (V3 v0, V3 v1)   { return V3(v0.x*v1.x, v0.y*v1.y, v0.z*v1.z);                                                }
V4  V4_Hadamard (V4 v0, V4 v1)   { return V4(v0.x*v1.x, v0.y*v1.y, v0.z*v1.z, v0.w*v1.w);                                     }
V2S V2S_Hadamard(V2S v0, V2S v1) { return V2S(v0.x*v1.x, v0.y*v1.y);                                                          }
V3S V3S_Hadamard(V3S v0, V3S v1) { return V3S(v0.x*v1.x, v0.y*v1.y, v0.z*v1.z);                                               }
V4S V4S_Hadamard(V4S v0, V4S v1) { return V4S(v0.x*v1.x, v0.y*v1.y, v0.z*v1.z, v0.w*v1.w);                                    }
V2  V2_Scale    (V2 v0, f32 n)   { return V2(v0.x*n, v0.y*n);                                                                 }
V3  V3_Scale    (V3 v0, f32 n)   { return V3(v0.x*n, v0.y*n, v0.z*n);                                                         }
V4  V4_Scale    (V4 v0, f32 n)   { return V4(v0.x*n, v0.y*n, v0.z*n, v0.w*n);                                                 }
V2S V2S_Scale   (V2S v0, s32 n)  { return V2S(v0.x*n, v0.y*n);                                                                }
V3S V3S_Scale   (V3S v0, s32 n)  { return V3S(v0.x*n, v0.y*n, v0.z*n);                                                        }
V4S V4S_Scale   (V4S v0, s32 n)  { return V4S(v0.x*n, v0.y*n, v0.z*n, v0.w*n);                                                }
V2S V2S_InvScale(V2S v0, s32 n)  { return V2S(v0.x/n, v0.y/n);                                                                }
V3S V3S_InvScale(V3S v0, s32 n)  { return V3S(v0.x/n, v0.y/n, v0.z/n);                                                        }
V4S V4S_InvScale(V4S v0, s32 n)  { return V4S(v0.x/n, v0.y/n, v0.z/n, v0.w/n);                                                }
f32 V2_Inner    (V2 v0, V2 v1)   { return v0.x*v1.x + v0.y*v1.y;                                                              }
f32 V3_Inner    (V3 v0, V3 v1)   { return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;                                                  }
f32 V4_Inner    (V4 v0, V4 v1)   { return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z + v0.w*v1.w;                                      }
s32 V2S_Inner   (V2S v0, V2S v1) { return v0.x*v1.x + v0.y*v1.y;                                                              }
s32 V3S_Inner   (V3S v0, V3S v1) { return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;                                                  }
s32 V4S_Inner   (V4S v0, V4S v1) { return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z + v0.w*v1.w;                                      }
V3  V3_Cross    (V3 v0, V3 v1)   { return V3(v0.y*v1.z - v0.z*v1.y, v0.z*v1.x - v0.x*v1.z, v0.x*v1.y - v0.y*v1.x);            }
V3S V3S_Cross   (V3S v0, V3S v1) { return V3S(v0.y*v1.z - v0.z*v1.y, v0.z*v1.x - v0.x*v1.z, v0.x*v1.y - v0.y*v1.x);           }
M2  V2_Outer    (V2 v0, V2 v1)   { return M2(V2_Scale(v1, v0.x), V2_Scale(v1, v0.y));                                         }
M3  V3_Outer    (V3 v0, V3 v1)   { return M3(V3_Scale(v1, v0.x), V3_Scale(v1, v0.y), V3_Scale(v1, v0.z));                     }
M4  V4_Outer    (V4 v0, V4 v1)   { return M4(V4_Scale(v1, v0.x), V4_Scale(v1, v0.y), V4_Scale(v1, v0.z), V4_Scale(v1, v0.w)); }

#define M2_Row(M, N) V2((M).i.e[N], (M).j.e[N])
#define M3_Row(M, N) V3((M).i.e[N], (M).j.e[N], (M).k.e[N])
#define M4_Row(M, N) V4((M).i.e[N], (M).j.e[N], (M).k.e[N], (M).l.e[N])

M2
M2_FromRows(V2 r0, V2 r1)
{
	return M2(V2(r0.x, r1.x),
						V2(r0.y, r1.y));
}

M3
M3_FromRows(V3 r0, V3 r1, V3 r2)
{
	return M3(V3(r0.x, r1.x, r2.x),
						V3(r0.y, r1.y, r2.y),
						V3(r0.z, r1.z, r2.z));
}

M4
M4_FromRows(V4 r0, V4 r1, V4 r2, V4 r3)
{
	return M4(V4(r0.x, r1.x, r2.x, r3.x),
						V4(r0.y, r1.y, r2.y, r3.y),
						V4(r0.z, r1.z, r2.z, r2.z),
						V4(r0.w, r1.w, r2.w, r2.w));
}

M2
M2_Transpose(M2 m)
{
	return M2_FromRows(m.i, m.j);
}

M3
M3_Transpose(M3 m)
{
	return M3_FromRows(m.i, m.j, m.k);
}

M4
M4_Transpose(M4 m)
{
	return M4_FromRows(m.i, m.j, m.k, m.l);
}

M2
M2_Add(M2 m0, M2 m1)
{
	return M2(V2_Add(m0.i, m1.i), V2_Add(m0.j, m1.j));
}

M3
M3_Add(M3 m0, M3 m1)
{
	return M3(V3_Add(m0.i, m1.i), V3_Add(m0.j, m1.j), V3_Add(m0.k, m1.k));
}

M4
M4_Add(M4 m0, M4 m1)
{
	return M4(V4_Add(m0.i, m1.i), V4_Add(m0.j, m1.j), V4_Add(m0.k, m1.k), V4_Add(m0.l, m1.l));
}

M2
M2_Sub(M2 m0, M2 m1)
{
	return M2(V2_Sub(m0.i, m1.i), V2_Sub(m0.j, m1.j));
}

M3
M3_Sub(M3 m0, M3 m1)
{
	return M3(V3_Sub(m0.i, m1.i), V3_Sub(m0.j, m1.j), V3_Sub(m0.k, m1.k));
}

M4
M4_Sub(M4 m0, M4 m1)
{
	return M4(V4_Sub(m0.i, m1.i), V4_Sub(m0.j, m1.j), V4_Sub(m0.k, m1.k), V4_Sub(m0.l, m1.l));
}

M2
M2_Hadamard(M2 m0, M2 m1)
{
	return M2(V2_Hadamard(m0.i, m1.i), V2_Hadamard(m0.j, m1.j));
}

M3
M3_Hadamard(M3 m0, M3 m1)
{
	return M3(V3_Hadamard(m0.i, m1.i), V3_Hadamard(m0.j, m1.j), V3_Hadamard(m0.k, m1.k));
}

M4
M4_Hadamard(M4 m0, M4 m1)
{
	return M4(V4_Hadamard(m0.i, m1.i), V4_Hadamard(m0.j, m1.j), V4_Hadamard(m0.k, m1.k), V4_Hadamard(m0.l, m1.l));
}

M2
M2_Scale(M2 m0, f32 n)
{
	return M2(V2_Scale(m0.i, n), V2_Scale(m0.j, n));
}

M3
M3_Scale(M3 m0, f32 n)
{
	return M3(V3_Scale(m0.i, n), V3_Scale(m0.j, n), V3_Scale(m0.k, n));
}

M4
M4_Scale(M4 m0, f32 n)
{
	return M4(V4_Scale(m0.i, n), V4_Scale(m0.j, n), V4_Scale(m0.k, n), V4_Scale(m0.l, n));
}

V2
M2_Mul(M2 m, V2 v)
{
	V2 r0 = M2_Row(m, 0);
	V2 r1 = M2_Row(m, 1);
	return V2(V2_Inner(v, r0), V2_Inner(v, r1));
}

V3
M3_Mul(M3 m, V3 v)
{
	V3 r0 = M3_Row(m, 0);
	V3 r1 = M3_Row(m, 1);
	V3 r2 = M3_Row(m, 2);
	return V3(V3_Inner(v, r0), V3_Inner(v, r1), V3_Inner(v, r2));
}

V4
M4_Mul(M4 m, V4 v)
{
	V4 r0 = M4_Row(m, 0);
	V4 r1 = M4_Row(m, 1);
	V4 r2 = M4_Row(m, 2);
	V4 r3 = M4_Row(m, 3);
	return V4(V4_Inner(v, r0), V4_Inner(v, r1), V4_Inner(v, r2), V4_Inner(v, r3));
}

M2
M2_Matmul(M2 m0, M2 m1)
{
	V2 r0 = M2_Row(m0, 0);
	V2 r1 = M2_Row(m0, 1);

	M2 result;
	result.i = V2(V2_Inner(m1.i, r0), V2_Inner(m1.i, r1));
	result.j = V2(V2_Inner(m1.j, r0), V2_Inner(m1.j, r1));

	return result;
}

M3
M3_Matmul(M3 m0, M3 m1)
{
	V3 r0 = M3_Row(m0, 0);
	V3 r1 = M3_Row(m0, 1);
	V3 r2 = M3_Row(m0, 2);

	M3 result;
	result.i = V3(V3_Inner(m1.i, r0), V3_Inner(m1.i, r1), V3_Inner(m1.i, r2));
	result.j = V3(V3_Inner(m1.j, r0), V3_Inner(m1.j, r1), V3_Inner(m1.j, r2));
	result.k = V3(V3_Inner(m1.k, r0), V3_Inner(m1.k, r1), V3_Inner(m1.k, r2));

	return result;
}

M4
M4_Matmul(M4 m0, M4 m1)
{
	V4 r0 = M4_Row(m0, 0);
	V4 r1 = M4_Row(m0, 1);
	V4 r2 = M4_Row(m0, 2);
	V4 r3 = M4_Row(m0, 3);

	M4 result;
	result.i = V4(V4_Inner(m1.i, r0), V4_Inner(m1.i, r1), V4_Inner(m1.i, r2), V4_Inner(m1.i, r3));
	result.j = V4(V4_Inner(m1.j, r0), V4_Inner(m1.j, r1), V4_Inner(m1.j, r2), V4_Inner(m1.j, r3));
	result.k = V4(V4_Inner(m1.k, r0), V4_Inner(m1.k, r1), V4_Inner(m1.k, r2), V4_Inner(m1.k, r3));
	result.l = V4(V4_Inner(m1.l, r0), V4_Inner(m1.l, r1), V4_Inner(m1.l, r2), V4_Inner(m1.l, r3));

	return result;
}
