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

#include <devdrvd_communication.h>
#include <librs232/rs232.h>
#include "ldu_macro.h"


extern unsigned int packet_length;

extern volatile sig_atomic_t shutdown_flag;
extern void ser_settings(struct rs232_port_t *ser);

//<MJK
//extern unsigned int address;
//>

static int catch_cmd_reply(int fd)
{
char buf[64];
char *p=&buf[1];
int free_size=sizeof buf, len;
int retries=0;
  memset(buf,'\0',free_size);
  while(1){
	if( free_size<=0 ){
#ifdef DEBUG
	  printf("catch_cmd_reply: buffer overflow\n");
#endif
	  return -1;
	}
	usleep(10000);
	len=read(fd,p,free_size);
#ifdef DEBUG
	printf("read %d bytes\n",len);
	if( len>0 ){
	  for(int i=0;i<len;i++)
		printf("%02x ",p[i]);
	  printf("\n");
	}
#endif
	if( len<=0 ){
	  retries++;
	  if( retries>=5 ) return -1;
	  continue;
	}
	if( p[len-1]!='\r' ){
	  p+=len;
	  free_size-=len;
	  retries=0;
	  continue;
	}
	if( p[len-2]=='K' && p[len-3]=='O' )
	  return 0; // OK found
	if( p[len-2]=='R' && p[len-3]=='R' )
	  return 1; // ERR found
	else{ // this was a weight message end
	  p+=len;
	  free_size-=len;
	  retries=0;
	  continue;
	}
  }
}

static int get_cmd_reply2(int fd)
{
char buf[64];
char *p=buf;
int free_size=sizeof buf, len;
int i;
int retries=0;
//printf("get2\n");
  memset(buf,'\0',free_size);
  if( write(fd,"GW\r",3)<0 ){
#ifdef DEBUG
	printf("write GW failed\n");
#endif
	return -1;
  }
  while(1){
	if( free_size<=0 ){
#ifdef DEBUG
	  printf("get_cmd_reply2: buffer overflow\n");
#endif
	  return -1;
	}
	usleep(10000);
	len=read(fd,p,free_size);
	if( len<=0 ){
	  retries++;
	  if( retries>=5 ) return -1;
	  continue;
	}
	else retries=0;
#ifdef DEBUG
	printf("read %d bytes\n",len);
	if( len>0 ){
	  for(i=0;i<len;i++)
		printf("%02x ",p[i]);
	  printf("\n");
	}
#endif
	if( (p+len-buf)<3 ){
	  p+=len;
	  free_size-=len;
	  continue;
	}
	for(i=0;i<len;i++)
	  if( p[i]=='\r' ) break;
	if( i==len ) return -1;
	if( p[i-1]=='K' && p[i-2]=='O' )
	  return 0; // OK found
	if( p[i-1]=='R' && p[i-2]=='R' )
	  return 1; // ERR found
  }
}

//extern scaled_client_t client_pool[];
extern scale_client_t cli_array[];

static int write_calibr(char *num,int id)
{
FILE *fp;
char calfname[256];
unsigned int len;
  sprintf(calfname,"/var/tmp/calibr%d",id);
  fp=fopen(calfname,"w");
  if( fp==NULL ){
#ifdef DEBUG
	printf("Failed to open %s %d\n",calfname,errno);
#endif
	return -1;
  }
  len=strlen(num);
//printf("len=%u\n",len);
  if( fwrite(num,1,len,fp)<len ){
#ifdef DEBUG
	printf("Failed to write %d bytes to %s, err=%d\n",len,calfname,errno);
#endif
	return -1;
  }
  fflush(fp);
  fclose(fp);
  return 0;
}

static int init_cmd(int fd,int id)
{
char buf[16];
char *p=buf;
int retries=0;
int free_size=sizeof buf, len;
  if( write(fd,"CE\r",3)<0 ){
#ifdef DEBUG
	printf("writing CE failed\n");
#endif
	return -1;
  }
//sleep(1);
  while(1){
	usleep(20000);
	len=read(fd,p,free_size);
	if( len<=0 ){
	  retries++;
	  if( retries>=5 ){
#ifdef DEBUG
		printf("init_cmd: read %ld bytes\n",p-buf);
#endif
		return -1;
	  }
	}
	else retries=0;
//printf("len=%d\n",len);
	if( (p+len-buf)>=8 )
	  break;
	p+=len;
	free_size-=len;
  }
  if( buf[7]!='\r' ){
#ifdef DEBUG
	printf("init_cmd: invalid response\n");
#endif
	return -1;
  }
  buf[7]=0;
  p=&buf[2];
  while( *p=='0' ) p++;
  if( *p=='\0' ) p--;
#ifdef DEBUG
  printf("%s\n",p);
#endif
  if( write_calibr(p,id)<0 )
	return-1;

  if( write(fd,"SZ\r",3)<0 ){
#ifdef DEBUG
	printf("write SZ failed\n");
#endif
	return -1;
  }
  return 0;
}


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
	return NULL;
  }
//  syslog(LOG_INFO,"Open port with port settings: %s\n",rs232_to_string(s->ser));
  rs232_flush(s->ser);
  // configure serial port
  ser_settings(s->ser);
  s->fd=fd=rs232_fd(s->ser);

  // create buffer for messages from ldu
  lb=new_locbuf();
  packet_length=18; // it is so but just in case!!!

  init_cmd(fd,s->id);
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

