typedef union i128
{
	struct
	{
		u64 hi;
		u64 lo;
	};
	u32 e[4];
} i128;
