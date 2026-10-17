// add -r (recreate config)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <unistd.h>

#include "DATA.h"
#include "args.h"
#include "cfg.h"
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
        if (event->len > 0 && !strcmp(event->name, "tcron")) {
          FILE *f = fopen(ctx->file_path_file, "r");
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

  read_cfg_file(&ctx, f);

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
