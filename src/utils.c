// add -r (recreate config)
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <time.h>

#include "DATA.h"
#include "logs.h"

int signal_instance()
{
  int fd = open("/tmp/tcron.lock", O_CREAT | O_RDWR, 0644);

  if (flock(fd, LOCK_EX | LOCK_NB)) {
    return 1;
  }

  return 0;
}

void get_time_now(char *out, size_t size)
{
  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);
  strftime(out, size, "[%Y-%m-%d %H:%M:%S]", &tm);
}

void run_command(MAIN_CONTEXT *ctx, char *cmd)
{
  pid_t pid = fork();

  if (pid < 0) {
    perror("error");
    exit(-1);
  }

  if (pid == 0) {
    // char *argv[32];
    // int i=0;

    // argv[i] = strtok(cmd, " ");
    //
    // while (argv[i] != NULL && i < 31) {
    //   argv[++i] = strtok(NULL, " ");
    // }

    int nul = open("/dev/null", O_RDONLY);
    if (nul >= 0) {
      dup2(nul, STDOUT_FILENO); // -> /dev/null
      dup2(nul, STDERR_FILENO); // -> /dev/null
      close(nul);
    }

    execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);

    write_error_log(ctx, cmd);// if failed command enter here
    _exit(127);
  }
}

void format(char *cmd, size_t size, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(cmd, size, fmt, args);
  va_end(args);
}
