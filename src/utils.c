#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

#include "DATA.h"
#include "errors.h"
#include "logs.h"

int signal_instance()
{
  int fd = open("/tmp/tcron.lock", O_CREAT | O_RDWR, 0644);
  if (fd == -1) die("tcron: must run as root");

  if (flock(fd, LOCK_EX | LOCK_NB)) {
    close(fd);
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
  if (pid < 0) { write_error_log(ctx, "fork failed"); return; }

  if (pid == 0) {
    signal(SIGCHLD, SIG_DFL);

    pid_t p2 = fork();
    if (p2 < 0) _exit(1);

    if (p2 == 0) {
      int nul = open("/dev/null", O_WRONLY);
      if (nul >= 0) { dup2(nul, STDOUT_FILENO); dup2(nul, STDERR_FILENO); close(nul); }
      execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
      _exit(127);
    }

    int st = 0;
    if (waitpid(p2, &st, 0) > 0 && WIFEXITED(st) && WEXITSTATUS(st) != 0) {
      char msg[CMD_SIZE + 64];
      snprintf(msg, sizeof msg, "'%s' exited with code %d", cmd, WEXITSTATUS(st));
      write_error_log(ctx, msg);
    }
    _exit(0);
  }
}

void format(char *cmd, size_t size, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(cmd, size, fmt, args);
  va_end(args);
}
