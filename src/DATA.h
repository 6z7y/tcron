#ifndef DATA_H
#define DATA_H

#include <pthread.h>

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
  Layer lay[512];
  char file_path_parent[128];
  char file_path_file[128];
  int current_ind;
  pthread_t reload_cfg;
  pthread_mutex_t lock;

} MAIN_CONTEXT;

#endif
