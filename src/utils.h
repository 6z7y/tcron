#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

int signal_instance();
void run_command(char *cmd);
void format(char *cmd, size_t size, const char *fmt, ...);

#endif
