void*
Align(void* ptr, u8 alignment)
{
	return (void*)(((umm)ptr + (alignment-1)) & ~(alignment-1));
}

void
Zero(void* ptr, umm size)
{
	for (umm i = 0; i < size; ++i) ((u8*)ptr)[i] = 0;
}

void
Copy(void* src, void* dst, umm size)
{
	u8* bsrc = (u8*)src;
	u8* bdst = (u8*)dst;

	for (umm i = 0; i < size; ++i) bdst[i] = bsrc[i];
}

void
Move(void* src, void* dst, umm size)
{
	u8* bsrc = (u8*)src;
	u8* bdst = (u8*)dst;

	if (bdst < bsrc) for (umm i = 0;      i < size; ++i) bdst[i] = bsrc[i];
	else             for (umm i = size-1; i < size; --i) bdst[i] = bsrc[i];
}
