#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "DATA.h"
#include "errors.h"
#include "utils.h"

void init_log_parent(MAIN_CONTEXT *ctx)
{
  char cmd[256];
  snprintf(cmd, sizeof cmd, "mkdir -p -m 755 %s", ctx->files.log_path_parent);
  FILE *f = popen(cmd, "r");
  pclose(f);
}

void init_log_file(MAIN_CONTEXT *ctx)
{
  ctx->files.f_log_access = fopen(ctx->files.log_access_path_file, "a");
      
  ctx->files.f_log_errors = fopen(ctx->files.log_errors_path_file, "a");

  if (!ctx->files.f_log_access || !ctx->files.f_log_errors)
    die("log files:");

  setvbuf(ctx->files.f_log_access, NULL, _IOLBF, 0);
  setvbuf(ctx->files.f_log_errors, NULL, _IOLBF, 0);
}

void check_logs(MAIN_CONTEXT *ctx)
{
  // check from folder
  if (access(ctx->files.log_path_parent, F_OK) != 0) { // if it dosen't exist
    init_log_parent(ctx);
  }

  if (!ctx->files.f_log_access || !ctx->files.f_log_errors)
    init_log_file(ctx);
}

void write_access_log(MAIN_CONTEXT *ctx, const char *msg)
{
  check_logs(ctx);

  char ts[32];
  get_time_now(ts, sizeof ts);

  if (ctx->files.f_log_access) fprintf(ctx->files.f_log_access, "%s-- %s\n", ts, msg);
}

void write_error_log(MAIN_CONTEXT *ctx, const char *msg)
{
  check_logs(ctx);

  char ts[32];
  get_time_now(ts, sizeof ts);

  if (ctx->files.f_log_errors) {
    fprintf(ctx->files.f_log_errors, "%s-- %s\n", ts, msg);
  }
}
