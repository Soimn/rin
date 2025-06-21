typedef struct U128
{
	u64 lo;
	u64 hi;
} U128;

U128
U128_Add(U128 a, U128 b, u8* carry_out)
{
	U128 result;
	u8 c = _addcarry_u64(0, a.lo, b.lo, &result.lo);
	*carry_out = _addcarry_u64(c, a.hi, b.hi, &result.hi);
	return result;
}

U128
U128_Mul10(U128 a, bool* overflow)
{
	u64 l_lo, l_hi;
	l_lo = _umul128(a.lo, 10, &l_hi);

	u64 h_lo, h_hi;
	h_lo = _umul128(a.hi, 10, &h_hi);

	U128 result;
	result.lo = l_lo;
	u8 c = _addcarry_u64(0, l_hi, h_lo, &result.hi);

	*overflow = (c || h_hi);

	return result;
}
