#include <sys/select.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>	// atof etc
#include <errno.h>
#include <unistd.h>	// write
#include <string.h>
#include <signal.h>
#include <syslog.h>

#include "config.h"
#include "scaled.h"
#include "commands.h"
#include "locbuf.h"
#include "json_stuff.h"

#include <devdrvd_communication.h>
#include <librs232/rs232.h>
#include "ldu_macro.h"

extern int started_cnt;
extern unsigned int packet_length;
//extern scaled_client_t client_pool[];
extern scale_client_t cli_array[];

extern volatile sig_atomic_t shutdown_flag;
extern void ser_settings(struct rs232_port_t *ser);

extern char *get_resp(int fd,locbuf_t *l,int *size);
extern int catch_cmd_reply(int fd);
extern int get_cmd_reply2(int fd);
extern int init_ldu(int fd,int id,locbuf_t *l);


void *ldu_loop(void *context)
{
scale_sys_t *s=(scale_sys_t *)context;
int fd;
int ready;
fd_set readfds;
fd_set errfs;

// New
locbuf_t *lb;			// buffer for data

size_t sys_buf_bytes=0;	// bytes on system buffer of pselect
//int n_msgs;				// estimated number of mesages
int m, n;				// miscelaneous auxiliary variables
char *wtmsg;			// weight terminal message pointer

  if( rs232_open(s->ser) ){
	syslog(LOG_CRIT, "Failed to open port: %s\n",s->ser->dev);
	printf("Failed to open port: %s\n",s->ser->dev);
	ser_settings(s->ser);
	rs232_end(s->ser);
	m=-2;
  }
  else{
//  syslog(LOG_INFO,"Open port with port settings: %s\n",rs232_to_string(s->ser));
	rs232_flush(s->ser);
	// configure serial port
	ser_settings(s->ser);
	s->fd=fd=rs232_fd(s->ser);

	// create buffer for messages from ldu
	lb=new_locbuf();
	m=init_ldu(fd,s->id,lb);
  }
  jobj_add_int(s->idx,"calibration",m);
  started_cnt++;
  if( m<0 ){
#ifdef DEBUG
	printf("LDU on %s is not ready\n",s->ser->dev);
#endif
	delete_locbuf(lb);
	return NULL;
  } 
  packet_length=18; // it is so but just in case!!!

  write(fd,"SW\r",3);
  // start pselect eternal loop
  while( shutdown_flag!=1 )
  {
//<14.06.2025
	if( s->calibr_mode ){
	// do nothing, only check shutdown flag
	  sleep(1);
	  continue;
	}
//>
	// check for pending command
//<<MJK 08.07.2025
//	if( s->cmd ){
	if( s->cmd && s->cmd->rc==-1 ){ // old format command!
//>
	  if( write(fd,s->cmd->cmdbuf,strlen(s->cmd->cmdbuf))<0 ){
		syslog(LOG_INFO, "Writing command failed");
#ifdef DEBUG
		printf("Command write failed\n");
#endif
	  }
	  else{
		if( (s->cmd->rc=catch_cmd_reply(fd))==-1 )
		  s->cmd->rc=get_cmd_reply2(fd);
	  }
	  sem_post(&s->sem);
	  s->cmd=NULL;
	  continue;
	}

// read bytes
	FD_ZERO(&readfds); FD_ZERO(&errfs);
	FD_SET(fd,&readfds); FD_SET(fd,&errfs);

	struct timespec tv;
	// timeout settings 3 secs; pselect modifies timeout, so
	// it is necessary to set it before each call
	tv.tv_sec=3;
	tv.tv_nsec=0;	// nanoseconds
	ready=pselect(fd+1,&readfds,0,&errfs,&tv,0);
	if( ready<=0 ){
	  if( ready==0 ){
		syslog(LOG_CRIT,"device_daemon: Timeout reached reading from %s\n.",s->ser->dev);
#ifdef DEBUG
		printf("device_daemon: Timeout reached reading from %s\n.",s->ser->dev);
#endif
//<MJK 08.07.2025
		if( !s->calibr_mode )
//>
		  write(fd,"SW\r",3);
	  }
	  else if( errno==EINTR && shutdown_flag ){
		syslog(LOG_INFO, "Shutdown called");
		break;
	  }
	  continue;
	}
	if( !FD_ISSET(fd,&readfds) ) continue;
	// bytes must be, but just in case ask one more time
	ioctl(fd,FIONREAD,&sys_buf_bytes);
	if( sys_buf_bytes<=0 ){
#ifdef DEBUG
	  printf("system bufer %zd bytes\n",sys_buf_bytes);
#endif
	  continue;
	}
//	printf("system bufer %zd bytes %d\n",sys_buf_bytes,packet_length);
	// now read system buffer
	// get_fromfd rtrievs bytes from system buffer to local
	while( (n=get_fromfd(lb,fd,sys_buf_bytes))>0 ){
//printf("n=%d\n",n);
	  sys_buf_bytes-=n;
	  while( (wtmsg=next_by_sym(lb,'\r',&m))!=NULL ){
		if( (size_t)m<packet_length ){
#ifdef DEBUG
		  printf("A packet of %d btyes discarded\n",m);
		  for(int i=0;i<m;i++) printf("%c",wtmsg[i]);
		  printf("\n");
#endif
//<<MJK 08.07.2025
		  if( s->cmd && s->cmd->rc==-2 ){ // mqtt command!
			memcpy(s->cmd->cmdbuf,wtmsg,m);
			s->cmd->rc=0;
			sem_post(&s->sem);
			s->cmd=NULL;
		  }
//>
		  continue;
		}
		wtmsg[m-1]=0;
//		  printf("%s\n",wtmsg);
		DISTRIBUTE_WEIGHT(wtmsg);
//		  memset(lb->spc,'0',sizeof(lb->spc));
	  }// parse internal buffer
	}// read data from system bufffer
//	sleep(1);
  }// main cycle until shutdown
#ifdef DEBUG
  printf("scale[%d] finished\n",s->id);
#endif
  return 0;
}

