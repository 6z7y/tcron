#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "DATA.h"

int signal_instance();
void get_time_now(char *out, size_t size);
void run_command(MAIN_CONTEXT *ctx, char *cmd);
void format(char *cmd, size_t size, const char *fmt, ...);

#endif
