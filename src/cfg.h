#ifndef CFG_H
#define CFG_H

#include "DATA.h"
#include <stdio.h>

void get_path(MAIN_CONTEXT *ctx);
void check_cfg(MAIN_CONTEXT *ctx);
void read_cfg_file(MAIN_CONTEXT *ctx, FILE *f);
void init_cfg_parent(MAIN_CONTEXT *ctx);
void init_cfg_file(MAIN_CONTEXT *ctx);

#endif
