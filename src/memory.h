inline void*
Align(void* ptr, u8 alignment)
{
  return (void*)(((umm)ptr + (alignment-1)) & ~(alignment-1));
}
