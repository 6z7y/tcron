#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

#define CMD_SIZE 1024
#define LINE_SIZE 512

typedef enum {
  SCHEDULE_INTERVAL,
  SCHEDULE_TIME,

} ScheduleType;

typedef struct {
  char cmd[CMD_SIZE];
  int interval_sec;
  int counter;
  int hour;
  int minute;
  ScheduleType type;

} Layer;

typedef struct {
  Layer lay[512];
  FILE *f;
  char file_path[64];
  int current_ind;

} MAIN_CONTEXT;

void get_path(MAIN_CONTEXT *ctx)
{
  const char *home_path = getenv("HOME");
  if (home_path == NULL) exit(-1);
  sprintf(ctx->file_path, "%s/.tcron", home_path);
  // printf("file_path: %s\n", ctx->file_path);
}

int to_sec(char *interval)
{
  int value = atoi(interval);
  char unit = interval[strlen(interval) - 1];

  switch (unit) {
    case 's': return value;
    case 'm': return value * 60;
    case 'h': return value * 3600;
    case 'd': return value * 86400;
    case 'w': return value * 604800;
  }

  return -1;
}

void error_log(const char *cmd)
{
  FILE *f = fopen("/tmp/tcron_err_log", "a");
  fprintf(f, "error in '%s': %s\n\n", cmd, strerror(errno));
  fclose(f);
}

void run_command(const char *cmd)
{
  pid_t pid = fork();

  if (pid < 0) {
    perror("error");
    exit(-1);
  }

  if (pid == 0) {
    execlp("sh", "sh", "-c", cmd, (char *)NULL);

    error_log(cmd);// if failed command enter here
  }
}

int signal_instance()
{
  int fd = open("/tmp/tcron.lock", O_CREAT | O_RDWR, 0644);

  if (flock(fd, LOCK_EX | LOCK_NB)) {
    return -1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  MAIN_CONTEXT ctx = {0};

  if (signal_instance() < 0) {
    printf("tcron is already running!\n");
    return 0;
  }

  get_path(&ctx);

  if (access(ctx.file_path, F_OK) != 0) {

    ctx.f = fopen(ctx.file_path, "w");

    if (!ctx.f) {
      perror("something happend");
      return -1;
    }

    fprintf(ctx.f, 
      "# Interval type: 1s, 1m, 1h, 1d, 1w\n"
      "# Example: run ls every 10 sec\n"
      "#10s ls\n\n"
      "# Time type: HH:MM (24-hour format)\n"
      "# Example: run pwd at 08:30\n"
      "#08:30 pwd\n"
    );

    fclose(ctx.f);
  }

  ctx.f = fopen(ctx.file_path, "r");
  if (!ctx.f) {
    perror("file");
    return -1;
  }

  char line[LINE_SIZE];
  char schedule[16];
  char cmd[CMD_SIZE];

  while (fgets(line, LINE_SIZE, ctx.f)) {
    if (line[0] == '#' || line[0] == '\n') continue;

    if (sscanf(line, "%15s %1023[^\n]", schedule, cmd) != 2) continue;

    if (strchr(schedule, ':')) {
      ctx.lay[ctx.current_ind].type = SCHEDULE_TIME;

      sscanf(schedule, "%d:%d", &ctx.lay[ctx.current_ind].hour, &ctx.lay[ctx.current_ind].minute);

      printf("TIME: %02d:%02d , cmd: %s\n", ctx.lay[ctx.current_ind].hour, ctx.lay[ctx.current_ind].minute, cmd);
    }
    else {
      ctx.lay[ctx.current_ind].type = SCHEDULE_INTERVAL;
      ctx.lay[ctx.current_ind].interval_sec = to_sec(schedule);
      printf("Interval: %s, cmd: %s\n", schedule, cmd);
    }
    strcpy(ctx.lay[ctx.current_ind].cmd, cmd);
    ctx.current_ind++;
  }

  while (1) {
    time_t now = time(NULL);
    struct tm *current = localtime(&now);

    for (int i=0; i<ctx.current_ind; i++) {
      ctx.lay[i].counter++;

      if (ctx.lay[i].type == SCHEDULE_INTERVAL) {
        if (ctx.lay[i].counter >= ctx.lay[i].interval_sec) {

          run_command(ctx.lay[i].cmd);

          ctx.lay[i].counter = 0;
        }
      }
      else if (ctx.lay[i].type == SCHEDULE_TIME) {

        if (current->tm_hour == ctx.lay[i].hour &&
          current->tm_min == ctx.lay[i].minute &&
          current->tm_sec == 0) {

            run_command(ctx.lay[i].cmd);
        }
      }
    }
    sleep(1);
  }

  fclose(ctx.f);
  return 0;
}
