#include <stdio.h>
#include <stdlib.h>

#include "../libs/arg_match.h"
#include "DATA.h"

void help()
{
    printf(
        "Usage: tcron [OPTION]\n"
        "\n"
        "Options:\n"
        "  -e              Edit tcron configuration\n"
        "  -h, --help      Show this help message\n"
        "  -v, --version   Show version information\n"
    );
}

int arg_handle(MAIN_CONTEXT *ctx, int argc, char **argv)
{
  if (argc <= 1) return 0;

  const char *help_opts[] = {"help", "-h", "--help", NULL};
  const char *ver_opts[] = {"version", "-v", "--version", NULL};

  if (arg_match_opt(&argc, argv, "-e") == 1) {
    char *editor = getenv("EDITOR");

    if (!editor) editor = "vi";

    char cmd[128];
    snprintf(cmd, 128, "%s %s", editor, ctx->files.cfg_path_file);
    int status = system(cmd);
    if (status < 0) perror("system");
    if (status != 0) perror("editior failed");
    return 1;
  }
  if (arg_match_opts(&argc, argv, help_opts) == 1) {
    help(); return 1;
  }

  if (arg_match_opts(&argc, argv, ver_opts) == 1) {
    printf("%s: %s\n", PROG_NAME, PROG_VER); return 1; 
  }

  printf("unknown option '%s'.\n", argv[1]);
  return 1;
}
