#ifndef DATA_H
#define DATA_H

#include <pthread.h>
#include <stdio.h>

#define PROG_NAME "tcron"
#define PROG_VER "1.0.0"

#define true 1
#define false 0

#define CMD_SIZE 1024
#define LINE_SIZE 512

typedef enum {
  SCHEDULE_INTERVAL,
  SCHEDULE_TIME,

} ScheduleType;

typedef struct {
  char cmd[CMD_SIZE];
  int interval_sec;
  int counter;
  int hour;
  int minute;
  ScheduleType type;

} Layer;

typedef struct {
  char cfg_path_parent[128];
  char cfg_path_file[128];

  char log_path_parent[128];
  char log_access_path_file[128];
  char log_errors_path_file[128];

  FILE *f_log_access;
  FILE *f_log_errors;

} Files;

typedef struct {
  Layer lay[512];
  Files files;
  int current_ind;
  pthread_t reload_cfg;

} MAIN_CONTEXT;

#endif
