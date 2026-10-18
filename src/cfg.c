#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "DATA.h"

void get_path(MAIN_CONTEXT *ctx)
{
  // const char *home_path = getenv("HOME");
  // if (home_path == NULL) exit(-1);

  // cfg paths
  sprintf(ctx->files.cfg_path_parent, "/etc/tcron/");
  sprintf(ctx->files.cfg_path_file, "%stcron", ctx->files.cfg_path_parent);

  // logs paths
  sprintf(ctx->files.log_path_parent, "/var/log/tcron/");
  sprintf(ctx->files.log_access_path_file, "%saccess.log", ctx->files.log_path_parent);
  sprintf(ctx->files.log_errors_path_file, "%serrors.log", ctx->files.log_path_parent);
}

int to_sec(char *interval)
{
  int total = 0;
  char *part = strtok(interval, ",");

  while (part != NULL) {
    int value = atoi(part);
    char unit = part[strlen(part) - 1];

    switch (unit) {
      case 's': total += value; break;
      case 'm': total += value * 60; break;
      case 'h': total += value * 3600; break;
      case 'd': total += value * 86400; break;
      case 'w': total += value * 604800; break;
      default: return -1;
    }

    part = strtok(NULL, ",");
  }
  return total;
}

void read_cfg_file(MAIN_CONTEXT *ctx, FILE *f)
{
  ctx->current_ind = 0;
  char line[LINE_SIZE];
  char schedule[16];
  char cmd[CMD_SIZE];

  while (fgets(line, LINE_SIZE, f)) {
    if (line[0] == '#' || line[0] == '\n') continue;

    if (sscanf(line, "%15s %1023[^\n]", schedule, cmd) != 2) continue;

    if (strchr(schedule, ':')) {
      ctx->lay[ctx->current_ind].type = SCHEDULE_TIME;

      sscanf(schedule, "%d:%d", &ctx->lay[ctx->current_ind].hour, &ctx->lay[ctx->current_ind].minute);

      // printf("TIME: %02d:%02d , cmd: %s\n", ctx->lay[ctx->current_ind].hour, ctx->lay[ctx->current_ind].minute, cmd);
    }
    else {
      ctx->lay[ctx->current_ind].type = SCHEDULE_INTERVAL;
      ctx->lay[ctx->current_ind].interval_sec = to_sec(schedule);
      // printf("Interval: %s, cmd: %s\n", schedule, cmd);

    }
    strcpy(ctx->lay[ctx->current_ind].cmd, cmd);
    ctx->current_ind++;
  }
}

void init_cfg_parent(MAIN_CONTEXT *ctx)
{
  char cmd[256];
  snprintf(cmd, sizeof cmd, "mkdir -p -m 755 '%s'", ctx->files.cfg_path_parent);
  FILE *f = popen(cmd, "r");
  pclose(f);
}

void init_cfg_file(MAIN_CONTEXT *ctx)
{
  FILE *f = fopen(ctx->files.cfg_path_file, "w");

  if (!f) {
    perror("something happend");
    exit(-1);
  }

  fprintf(f, 
      "# Interval type: 1s, 1m, 1h, 1d, 1w\n"
      "# You can combine intervals: 1h,30m,20s\n"
      "# Example: run ls every 2 hours and 30 minutes\n"
      "2h,30m ls\n\n"
      "# Time type: HH:MM (24-hour format)\n"
      "# Example: run pwd at 08:30\n"
      "08:30 pwd\n"
      );
  fchmod(fileno(f), 0666);

  fclose(f);
}

void check_cfg(MAIN_CONTEXT *ctx)
{
  if (access(ctx->files.cfg_path_parent, F_OK) != 0)
    init_cfg_parent(ctx);

  if (access(ctx->files.cfg_path_file, F_OK) != 0)
    init_cfg_file(ctx);

}
