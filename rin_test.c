#include "rin.h"

int
main()
{
  R_smm matched_chars = R_String_PatternMatch(R_STRING("Hello World"), "Hello");
  R_u32 u32, x32;
  R_u8 c;
  matched_chars = R_String_PatternMatch(R_STRING("Hello 567768 World, 0xBADF00d\n"), "Hello %u32 Worl%c, 0x%x32\n", &u32, &c, &x32);
  return 0;
}
