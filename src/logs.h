#ifndef LOGS_H
#define LOGS_H

#include "DATA.h"

void init_log_parent(MAIN_CONTEXT *ctx);
void init_log_file(MAIN_CONTEXT *ctx);
void write_access_log(MAIN_CONTEXT *ctx, const char *msg);
void write_error_log(MAIN_CONTEXT *ctx, const char *msg);
void check_logs(MAIN_CONTEXT *ctx);

#endif
