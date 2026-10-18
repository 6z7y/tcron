// add -r (recreate config)
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <unistd.h>

#include "DATA.h"
#include "args.h"
#include "cfg.h"
#include "errors.h"
#include "logs.h"
#include "utils.h"

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
  inotify_add_watch(fd, ctx->files.cfg_path_parent, IN_CLOSE_WRITE);

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
        if (event->len > 0 && !strcmp(event->name, "tcron")) {
          FILE *f = fopen(ctx->files.cfg_path_file, "r");
          if (!f) {
            perror("file");
            continue;
          }

          read_cfg_file(ctx, f);

          fclose(f);
        }
      }
      i += sizeof(struct inotify_event) + event->len;
    }
  }
  return NULL;
}


int main(int argc, char **argv)
{
  MAIN_CONTEXT ctx = {0};

  get_path(&ctx);

  if (arg_handle(&ctx, argc, argv) > 0) return 0;

  signal(SIGCHLD, SIG_IGN);

  if (signal_instance() > 0)
    die("tcron: is already running!");


  check_cfg(&ctx);
  init_logs(&ctx);

  FILE *f = fopen(ctx.files.cfg_path_file, "r");
  if (!f) die("file:");

  read_cfg_file(&ctx, f);

  // thread for changing config when working
  pthread_create(&ctx.reload_cfg, NULL, reload_cfg_thread, &ctx);
  pthread_detach(ctx.reload_cfg);

  char copy_cmd[CMD_SIZE];
  while (1) {
    time_t now = time(NULL);
    struct tm *current = localtime(&now);

    for (int i=0; i<ctx.current_ind; i++) {
      ctx.lay[i].counter++;

      if (ctx.lay[i].type == SCHEDULE_INTERVAL) {
        if (ctx.lay[i].counter >= ctx.lay[i].interval_sec) {

          strcpy(copy_cmd, ctx.lay[i].cmd);
          run_command(&ctx, ctx.lay[i].cmd);
          write_access_log(&ctx, copy_cmd);

          ctx.lay[i].counter = 0;
        }
      }
      else if (ctx.lay[i].type == SCHEDULE_TIME) {

        if (current->tm_hour == ctx.lay[i].hour &&
          current->tm_min == ctx.lay[i].minute &&
          current->tm_sec == 0) {

          strcpy(copy_cmd, ctx.lay[i].cmd);
          run_command(&ctx, ctx.lay[i].cmd);
          write_access_log(&ctx, copy_cmd);
        }
      }
    }
    sleep(1);
  }

  fclose(f);
  return 0;
}
