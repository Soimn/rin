// TODO: Tests

typedef union s128
{
  struct
  {
    u64 lo;
    u64 hi;
  };

  u32 e[4];
} s128;

static inline s128
S128_FromU64(u64 v)
{
  return (s128){ .lo = v, .hi = 0 };
}

static inline s128
S128_FromBits(u64 lo, u64 hi)
{
  return (s128){ .lo = lo, .hi = hi };
}

static inline s128
S128_FromS64(s64 v)
{
  return (s128){ .lo = (u64)v, .hi = (u64)(v >> 63) };
}

static inline u64
S128_ToU64(s128 v)
{
  return v.lo;
}

static inline s64
S128_ToS64(s128 v)
{
  return (s64)v.lo;
}

static inline s128
S128_Add(s128 a, s128 b, bool* overflow)
{
  u64 lo, hi;
  u8 c1 = _addcarryx_u64(0, a.lo, b.lo, &lo);
  u8 c2 = _addcarryx_u64(c1, a.hi, b.hi, &hi);

  *overflow = ((s64)(a.hi^b.hi) >= 0 && (s64)(a.hi^hi) < 0);

  return (s128){ .lo = lo, .hi = hi };
}

static inline s128
S128_Sub(s128 a, s128 b, bool* overflow)
{
  u64 lo, hi;
  u8 b1 = _subborrow_u64(0, a.lo, b.lo, &lo);
  u8 b2 = _subborrow_u64(b1, a.hi, b.hi, &hi);

  *overflow = ((s64)(a.hi^b.hi) < 0 && (s64)(a.hi^hi) < 0);

  return (s128){ .lo = lo, .hi = hi };
}

static inline s128
S128_Mul(s128 a, s128 b, bool* overflow)
{
  u64 a0b0_hi;
  u64 a0b0_lo = _umul128(a.lo, b.lo, &a0b0_hi);

  u64 a0b1_hi;
  u64 a0b1_lo = _umul128(a.lo, b.hi, &a0b1_hi);

  u64 a1b0_hi;
  u64 a1b0_lo = _umul128(a.hi, b.lo, &a1b0_hi);

  u64 lo = a0b0_lo;
  u64 hi;

  u8 c = _addcarry_u64(0, a0b1_lo, a1b0_lo, &hi);
  c = _addcarry_u64(c, hi, a0b0_hi, &hi);

  *overflow = (c != 0 || (a.hi != 0 && b.hi != 0));

  return (s128){ .lo = lo, .hi = hi };
}

static inline s128
S128_Div(s128 a, s128 b)
{
  NOT_IMPLEMENTED;
  s128 result = {0};
  return result;
}

static inline s128
S128_Shl(s128 n, u8 amount)
{
  amount &= 0x7F;
  return (s128){ .lo = n.lo << amount, .hi = (n.hi << amount) | (n.lo >> -amount) };
}

static inline s128
S128_Shr(s128 n, u8 amount)
{
  amount &= 0x7F;
  return (s128){ .lo = (n.lo >> amount) | (n.hi << -amount), .hi = (n.hi >> amount) };
}

static inline s128
S128_Sar(s128 n, u8 amount)
{
  amount &= 0x7F;
  return (s128){ .lo = (n.lo >> amount) | ((s64)n.hi << -amount), .hi = (s64)n.hi >> amount };
}
