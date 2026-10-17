#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "DATA.h"
#include "utils.h"

void write_access_log(MAIN_CONTEXT *ctx, const char *msg)
{
  char ts[32];
  get_time_now(ts, sizeof ts);

  if (ctx->files.f_log_access) fprintf(ctx->files.f_log_access, "%s-- %s\n", ts, msg);
}

void write_error_log(MAIN_CONTEXT *ctx, const char *msg)
{
  char ts[32];
  get_time_now(ts, sizeof ts);
  if (ctx->files.f_log_errors) fprintf(ctx->files.f_log_errors, "%s-- %s: %s\n", ts, msg, strerror(errno));
}

void init_logs(MAIN_CONTEXT *ctx)
{
  if (access(ctx->files.log_path_parent, F_OK) != 0) {
    char cmd[256];
    format(cmd, sizeof cmd, "mkdir -p -m 755 %s", ctx->files.log_path_parent);
    FILE *f = popen(cmd, "r");
    pclose(f);
  }

  ctx->files.f_log_access = fopen(ctx->files.log_access_path_file, "a");
  ctx->files.f_log_errors = fopen(ctx->files.log_errors_path_file, "a");

  if (!ctx->files.f_log_access || !ctx->files.f_log_errors) {
    perror("log files");
    exit(-1);
  }

  setvbuf(ctx->files.f_log_access, NULL, _IOLBF, 0);
  setvbuf(ctx->files.f_log_errors, NULL, _IOLBF, 0);

}
