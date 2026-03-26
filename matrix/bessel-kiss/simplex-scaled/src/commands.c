#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <syslog.h>

#include "scaled.h"
#include "commands.h"
/*
External protocol

command template:
----------------
0x02 S C ; x x ; z z 0x0d 0x0a --- 11 symbols
0x02 - Mettler protocol message start symbol
; - delimiter
SC - scale control
; - delimiter
xx - scale system id in ascii form with leading zero
     example "01" for scale id=1 (numeric)
zz - command code - literal:
   SZ - set zero
   ST - set tare
   RT - reset tare
Example: 0x02 S C ; 0 1 ; S Z 0x0d 0x0a - set zero for scale id=1

# NOTE: extension for future - scale system includes multiple terminals
0x02 S C ; x x ; y y ; z z 0x0d 0x0a --- 14 symbols
#yy - terminal addr within scale system
#     example "01" for addr=1 (numeric)

reply template:
--------------
0x02 S C ; x x ; y y 0x0d 0x0a --- 11 symbols
xx - return code from transmit level:
  "00" - OK
  "01" - command unrecognized (message damaged)
  "02" - scale system with given id not found
  "03" - for this scale system this command not supported
  Note: "yy" is not used for responses from transmit level
yy - return code from application level
  "00" - OK
  "01" - command delivered, but no response from terminal within timeout
  "02" - ERR
*/

//extern volatile sig_atomic_t shutdown_flag;
extern volatile int shutdown_flag;
extern int scale_cnt;
extern scale_sys_t *scale[MAX_SCALES];

static char cmdbuf[16]={ 0x02,'S','C',';','x','x',';','x','x',0x0d,0x0a, 0 };
static char repbuf[16]={ 0x02,'S','C',';','0','0',';','0','0',0x0d,0x0a, 0 };
static cmd_pending_t cur_cmd;

int parse_buf(char *buf,int *scale_id,enum cmd_e *cmd)
{
  if( scale_id )
	*scale_id=(buf[4]-'0')*10+buf[5]-'0';
  if( cmd==NULL ) return 0;
  if( buf[7]=='S' ){
	if( buf[8]=='Z' ) *cmd=set_zero;
	else{
	  *cmd = ( buf[8]=='T' ) ? set_tare:max_cmd;
	}
  }
  else if( buf[7]=='R' ){
	*cmd = ( buf[8]=='T' ) ? reset_tare:max_cmd;
  }
  return (*cmd==max_cmd) ? -1:0;
}

void *runcommand(void *context)
{
int sock=*((int*)context);
struct sockaddr_storage addr;
socklen_t addr_len=sizeof addr;
struct timespec ts;
int n, size;
enum cmd_e cmd;
int id;

  size=strlen(cmdbuf);
  while( shutdown_flag!=1 ){
 	n=recvfrom(sock,cmdbuf,size,MSG_WAITALL,(struct sockaddr *)&addr,&addr_len);
//printf("recvfrom %d\n",n);
	if( n<size || cmdbuf[0]!=0x02 || parse_buf(cmdbuf,&id,&cmd)<0 ){
	  repbuf[4]='0'; repbuf[5]='1';
	  goto rep;
	}

	// find scale system and its device
	scale_sys_t *s=NULL;
	for(int i=0;i<scale_cnt;i++)
	  if( scale[i]->id==id ) s=scale[i];
	if( s==NULL ){
	  repbuf[4]='0'; repbuf[5]='2';
	  goto rep;
	}
	// in this version we sssume single terminal indexed by 0
	model_cmd_t *m=s->st[0]->model;
	if( !(m && m->cmds[cmd]) ){
	  repbuf[4]='0'; repbuf[5]='3';
	  goto rep;
	}
	repbuf[4]='0'; repbuf[5]='0';	// OK with id and command

	// prepare call
	cur_cmd.cmdbuf=m->cmds[cmd];
	cur_cmd.rc=-1;
	clock_gettime(CLOCK_REALTIME,&ts);
	ts.tv_sec+=3; // set timeout =2 seconds
	s->cmd=&cur_cmd;
	sem_timedwait(&s->sem,&ts);
	// analyse ser_loop response
	repbuf[7]='0';
	switch(cur_cmd.rc){
	  case -1:	// no reply within timeout
	  default:
		repbuf[8]='1';
		break;
	  case 0: // OK
		repbuf[8]='0';
		break;
	  case 1: // ERR
		repbuf[8]='2';
		break;
	}
rep:
//	if( sendto(sock,repbuf,size,MSG_CONFIRM,(struct sockaddr *)&addr,addr_len)<0 ){
	if( sendto(sock,repbuf,size,0,(struct sockaddr *)&addr,addr_len)<0 ){
	  syslog(LOG_CRIT,"commands: sendto err=%d\n.",errno);
//#ifdef DEBUG
	  perror("ctrl sendto");
//#endif
	}
  }
  return NULL;
}

