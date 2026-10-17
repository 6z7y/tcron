#include <errno.h>
#include <stdio.h>
#include <string.h>


void error_log(const char *cmd)
{
  FILE *f = fopen("/tmp/tcron.err", "a");
  fprintf(f, "error in '%s': %s\n\n", cmd, strerror(errno));
  fclose(f);
}
