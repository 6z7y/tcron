#ifndef LOGS_H
#define LOGS_H

#include "DATA.h"

void write_access_log(MAIN_CONTEXT *ctx, const char *msg);
void write_error_log(MAIN_CONTEXT *ctx, const char *msg, int ERR);
void init_logs(MAIN_CONTEXT *ctx);

#endif
