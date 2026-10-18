#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void verr(const char *fmt, va_list args)
{
  vfprintf(stderr, fmt, args);
  if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
    fputc(' ', stderr);
    perror(NULL);
  }
  else { fputc('\n', stderr); }
}

int warn(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  verr(fmt, args);
  va_end(args);


  return -1;
}

void die(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  verr(fmt, args);
  va_end(args);
  exit(-1);

}
