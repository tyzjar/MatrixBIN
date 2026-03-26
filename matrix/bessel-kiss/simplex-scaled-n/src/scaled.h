#ifndef _SCALE_H_INCLUDED
#define _SCALE_H_INCLUDED

#define MAX_SCALES 4
#define MAX_DEVS_INSCALE 6
#define MAX_TERM_CMDS 4
#define MAX_MODELS 4
#define MAX_CLIENTS 4
#ifndef SCALED_MAX_CLIENTS
  #define SCALED_MAX_CLIENTS 8
#endif

#include <semaphore.h>

struct rs232_port_t;
struct cmd_pending_str;

enum cmd_e { set_zero, set_tare, reset_tare, max_cmd };

typedef struct {
  char model[12];
  char *cmds[MAX_TERM_CMDS];
} model_cmd_t;

typedef struct {
  unsigned int addr;
//< MJK 12.01.2026
  int datatype_raw;
//>
  model_cmd_t *model;
} scale_term_t;

typedef struct {
  pthread_t thr; // -1 if empty
  int go;	// while =1 continue operation
  int sock;
  int fin;	// request for scale to close wrfd pipe end
  int wrfd[MAX_SCALES];	// pipe ends for scales to write data
  int rdfd[MAX_SCALES];	// pipe ends to read data from scales
} scale_client_t;

typedef struct {
  struct rs232_port_t *ser;
//<15.06.2025
  int fd;
//>
  sem_t sem;
  int id;	// id for matrix (may be any number)
  int idx;	// internal id (=0,1,..,scale_cnt-1)
  int dev_cnt;
  int calibr_mode;
  struct cmd_pending_str *cmd;
  scale_term_t *st[MAX_DEVS_INSCALE];
//  int ctlfd[2];	// pipe to read cmds and write responses
} scale_sys_t;


#endif

