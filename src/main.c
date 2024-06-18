#include "rin.h"

#include <stdio.h>
#include <stdarg.h>

static void
AssertHandler(const char* file, int line, const char* expr, const char* message, ...)
{
  fprintf(stderr, "%s(%d): Assertion \"%s\" failed\n", file, line, expr);

  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
}

int
main(int argc, char** argv)
{
  return 0;
}
