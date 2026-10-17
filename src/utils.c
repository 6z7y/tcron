// add -r (recreate config)
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "errors.h"

int signal_instance()
{
  int fd = open("/tmp/tcron.lock", O_CREAT | O_RDWR, 0644);

  if (flock(fd, LOCK_EX | LOCK_NB)) {
    return -1;
  }

  return 0;
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

void format(char *cmd, size_t size, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(cmd, size, fmt, args);
  va_end(args);
}
