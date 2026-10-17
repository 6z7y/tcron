// check if file modify
// add -r (recreate config)
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libs/arg_match.h"

#define PROG_NAME "tcron"
#define PROG_VER "1.0.0"

#define CMD_SIZE 1024
#define LINE_SIZE 512

#define true 1
#define false 0

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
  char file_path_parent[128];
  char file_path_file[128];
  int current_ind;
  pthread_t reload_cfg;
  pthread_mutex_t lock;

} MAIN_CONTEXT;

void get_path(MAIN_CONTEXT *ctx)
{
  const char *home_path = getenv("HOME");
  if (home_path == NULL) exit(-1);
  sprintf(ctx->file_path_parent, "%s/.local/share/tcron/", home_path);
  sprintf(ctx->file_path_file, "%s.tcron", ctx->file_path_parent);
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
  FILE *f = fopen("/tmp/tcron.err", "a");
  fprintf(f, "error in '%s': %s\n\n", cmd, strerror(errno));
  fclose(f);
}

void run_command(char *cmd)
{
  pid_t pid = fork();

  if (pid < 0) {
    perror("error");
    exit(-1);
  }

  if (pid == 0) {
    char *argv[32];
    int i=0;

    argv[i] = strtok(cmd, " ");

    while (argv[i] != NULL && i < 31) {
      argv[++i] = strtok(NULL, " ");
    }

    execvp(argv[0], argv);

    error_log(cmd);// if failed command enter here
    _exit(127);
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

void help()
{
    printf(
        "Usage: tcron [OPTION]\n"
        "\n"
        "Options:\n"
        "  -e              Edit tcron configuration\n"
        //"  -r              Reinit Config\n"
        "  -h, --help      Show this help message\n"
        "  -v, --version   Show version information\n"
    );
}

int arg_handle(MAIN_CONTEXT *ctx, int argc, char **argv)
{
  const char *help_opts[] = {"help", "-h", "--help"};
  const char *ver_opts[] = {"version", "-v", "--version"};


  if (argc > 1) {
    for (int i=1; i<argc; i++) {
      if (arg_match_opt(&argc, argv, "-e") == 1) {
        char *editor = getenv("EDITOR");

        if (!editor) editor = "vi";

        char cmd[128];
        snprintf(cmd, 128, "%s %s", editor, ctx->file_path_parent);
        int status = system(cmd);
        if (status < 0) perror("system");
        if (status != 0) perror("editior failed");
        return 1;
      }
      if (arg_match_opts(&argc, argv, help_opts) == 1) help(); return 1;
      if (arg_match_opts(&argc, argv, ver_opts) == 1) printf("%s: %s\n", PROG_NAME, PROG_VER); return 1;
    }
  }
  return 0;
}

void read_cfg(MAIN_CONTEXT *ctx, FILE *f)
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

      printf("TIME: %02d:%02d , cmd: %s\n", ctx->lay[ctx->current_ind].hour, ctx->lay[ctx->current_ind].minute, cmd);
    }
    else {
      ctx->lay[ctx->current_ind].type = SCHEDULE_INTERVAL;
      ctx->lay[ctx->current_ind].interval_sec = to_sec(schedule);
      printf("Interval: %s, cmd: %s\n", schedule, cmd);
    }
    strcpy(ctx->lay[ctx->current_ind].cmd, cmd);
    ctx->current_ind++;
  }
}

void *reload_cfg_thread(void *arg)
{
  MAIN_CONTEXT *ctx = arg;

  char buf[4096];

  // init inotify
  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
    exit(-1);
  }

  // put watcher types
  inotify_add_watch(fd, ctx->file_path_parent, IN_CLOSE_WRITE);

  // 4. while loop
  while (true)
  {
    ssize_t len = read(fd, &buf, sizeof(buf));
    if (len < 0) {
      perror("read");
      continue;
    }

    for (size_t i=0; i<len;) {
      struct inotify_event *event = (struct inotify_event*)&buf[i];

      // the checker
      if (event->mask & IN_CLOSE_WRITE || event->mask & IN_MODIFY) {
        if (event->len > 0 && !strcmp(event->name, ".tcron")) {
          FILE *f = fopen(ctx->file_path_file, "r");
          if (!f) {
            perror("file");
            continue;
          }

          read_cfg(ctx, f);

          fclose(f);
        }
      }
      i += sizeof(struct inotify_event) + event->len;
    }
  }

  return NULL;
}

void check_cfg(MAIN_CONTEXT *ctx)
{
  if (access(ctx->file_path_parent, F_OK) != 0) {

    char cmd[128];
    snprintf(cmd, 128, "mkdir -m 755 %s", ctx->file_path_parent);
    run_command(cmd);
  }

  if (access(ctx->file_path_file, F_OK) != 0) {
    FILE *f = fopen(ctx->file_path_file, "w");

    if (!f) {
      perror("something happend");
      exit(-1);
    }

    fprintf(f, 
        "# Interval type: 1s, 1m, 1h, 1d, 1w\n"
        "# Example: run ls every 10 sec\n"
        "#10s ls\n\n"
        "# Time type: HH:MM (24-hour format)\n"
        "# Example: run pwd at 08:30\n"
        "#08:30 pwd\n"
        );

    fclose(f);
  }

}

int main(int argc, char **argv)
{
  MAIN_CONTEXT ctx = {0};
  pthread_mutex_init(&ctx.lock, NULL);

  get_path(&ctx);

  if (arg_handle(&ctx, argc, argv) > 0) return 0;

  if (signal_instance() < 0) {
    printf("tcron is already running!\n");
    return 0;
  }

  check_cfg(&ctx);

  FILE *f = fopen(ctx.file_path_file, "r");
  if (!f) {
    perror("file");
    return -1;
  }

  read_cfg(&ctx, f);


  // thread for changing config when working
  pthread_create(&ctx.reload_cfg, NULL, reload_cfg_thread, &ctx);
  pthread_detach(ctx.reload_cfg);

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

  fclose(f);
  return 0;
}
