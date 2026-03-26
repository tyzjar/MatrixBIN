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

#include <devdrvd_communication.h>
#include <librs232/rs232.h>


extern unsigned int packet_length;
extern unsigned int weight_pos;
extern unsigned int weight_len;
extern char *status[4];
extern unsigned int status_pos;
extern unsigned int status_len;

extern volatile sig_atomic_t shutdown_flag;
extern void ser_settings(struct rs232_port_t *ser);

extern scale_client_t cli_array[];

void *dis4_loop(void *context)
{
scale_sys_t *s=(scale_sys_t *)context;
int fd;
int ready;
fd_set readfds;
fd_set errfs;
char *pStart;
char *pEnd;
char buffer[64];

// time test
struct timeval ttv;

char *sweight=(char *)malloc((weight_len+1)*sizeof(char));

  memset(sweight,'\0',(weight_len+1));

  if( rs232_open(s->ser) ){
	syslog(LOG_CRIT, "Failed to open port: %s\n",s->ser->dev);
	printf("Failed to open port: %s\n",s->ser->dev);
	ser_settings(s->ser);
	rs232_end(s->ser);
	return NULL;
  }
//  syslog(LOG_INFO,"Open port with port settings: %s\n",rs232_to_string(s->ser));
  rs232_flush(s->ser);
  fd=rs232_fd(s->ser);

  pStart=pEnd=&buffer[0];

// time test
  gettimeofday(&ttv,NULL);

  // start pselect eternal loop
  while( shutdown_flag!=1 )
  {
	// check for pending command
	// !!!
	FD_ZERO(&readfds);
	FD_ZERO(&errfs);

	FD_SET(fd,&readfds);
	FD_SET(fd,&errfs);

	struct timespec tv;
	// timeout settings
	tv.tv_sec=3;
	tv.tv_nsec=0;	// nanoseconds
	ready=pselect(fd+1,&readfds,0,&errfs,&tv,0);
	if( ready==0 ){
	  syslog(LOG_CRIT,"device_daemon: Timeout reached reading from %s\n.",s->ser->dev);
#ifdef DEBUG
	  printf("device_daemon: Timeout reached reading from %s\n.",s->ser->dev);
#endif
//	  issue command to resume data flow
	}

	int errsv = errno;
	if( errno==EINTR && shutdown_flag ){
	  syslog(LOG_INFO, "Shutdown called");
	  break;
	}
	if( ready && FD_ISSET(fd,&readfds) ){
	  size_t len=0;
	  ioctl(fd,FIONREAD,&len);

	  /** @begin Comment for dummy pipe */
	  if( len==0 && errsv==EADDRNOTAVAIL ){
		/** Connection to channel crashed or closed
		* @todo send error code to client
		*/
	  }
	  /** @end Comment for dummy pipe */

	  volatile size_t read_len;
	  // while not all bytes are read
	  while( len>0 ){
		size_t b_size=pStart-&buffer[0]+1;
		if( len>(sizeof(buffer)-b_size))
		  read_len=read(fd,pStart,(sizeof(buffer)-b_size));
		else
		  read_len=read(fd,(void*)pStart,len);

		if( read_len==0 ) continue;
		b_size+=read_len;
		buffer[b_size-1]='\0';
#ifdef DEBUG
//printf("Read from device, recieved bytes: %d, bytes in buffer: %d, buffer: '%s'\n",(unsigned int)read_len,(unsigned int)len,buffer);
//printf("Read from device, bytes in buffer: %d\n",(unsigned int)len);
//printf("Read from device, buffer: %s\n",buffer);
#endif
		char *tmp=strchr(pStart,'\r');
		// terminating symbol found
		if( tmp!=0 ) pStart=tmp;
		else{ // terminating symbol not found
		  pStart=&buffer[b_size-1];
		  len-=read_len;
		  continue;
		}
		do{
		  size_t buffer_len=pStart-pEnd+1;
#ifdef DEBUG
		  char tbuffer[64]={0};
		  strncpy(tbuffer,pEnd,buffer_len);
		  tbuffer[buffer_len]='\0';
		  printf("device %s, buffer[%zu](%d): %s\n",s->ser->dev,buffer_len, packet_length, tbuffer);
#endif
		  // a whole packet from terminal received
		  // create message in our internal protocol format
		  // and distribute it among clients
		  if( buffer_len==packet_length ){
			// Parse packet
			struct weight_struct w_s;
			int rc=gettimeofday(&w_s.ts,NULL);
			if( rc!=0 ){
#ifdef DEBUG
			  printf("gettimeofday failed!\n");
#endif
			}
			strncpy(sweight,&buffer[weight_pos],weight_len);
			w_s.weight=atof(sweight);
			w_s.status=0;
// time test
long unsigned msec=(1000*w_s.ts.tv_sec+w_s.ts.tv_usec/1000)-(1000*ttv.tv_sec+ttv.tv_usec/1000);
ttv=w_s.ts;
printf("%f  %lu\n",w_s.weight,msec);
/*
			// BIT STATUS
			enum estatus istatus=0;
			for(unsigned int i=0;i<status_len;i++){
			  if( status[i][0]==buffer[status_pos] ) istatus=i;
			}
			switch( istatus ){
			  case STATUS_STABLE:
				SET_BIT(w_s.status, STABLE);
				break;
			  case STATUS_UNSTABLE:
			  case STATUS_OVERLOAD:
			  case STATUS_UNDERLOAD:
				break;
			}
//<MJK
//                        w_s.channel = address; //channel from args
			w_s.channel=s->id; // this scale system id from cfg file
//>
			w_s.tare=0;
*/
			char *package=0;
			size_t package_len=0;
			package_len=pack_weight(&package,&w_s);
			int i;
			for(i=0;i<MAX_CLIENTS;++i){
			  scale_client_t *c=&cli_array[i];
			  int wrfd=c->wrfd[s->idx];
//printf("%d : %d\n",i,wrfd);
			  if( wrfd==-1 ) continue;
			  if( c->fin ){
				close(wrfd);
				c->wrfd[s->idx]=-1;
//printf("Close wrdf fin: %d %d\n",s->idx,c->wrfd[s->idx]);
				continue;
			  }
			  if( write(wrfd,package,package_len)<0 ){
				close(wrfd);
				c->wrfd[s->idx]=-1;
//printf("Close wrdf: %d %d\n",s->idx,c->wrfd[s->idx]);
			  }
			}
			free(package);
			package_len = 0;
		  }//buffer_len==packet_length
		  else{
#ifdef DEBUG
			printf("Read from device, discarded bad packet buffer[%zu]: %s\n",buffer_len,buffer);
#endif
		  }
		  pEnd=++pStart;
		  pStart=strchr(pStart,'\r');
		  if( pStart==0 ){
			size_t left=0;
			if( *pEnd=='\n') pEnd++;
			if( *pEnd!='\0' ){
			  // Some unterminated data left
			  left=strlen(buffer)-(pEnd-&buffer[0]);
			  memmove(&buffer[0],pEnd,left);
			  //memset(&buffer[left], '\0', (sizeof(buffer) - left));
			}
			pEnd=&buffer[0];
			pStart=&buffer[left];
		  }
		} while( pEnd!=&buffer[0] );
		len-=read_len;
	  }
	}
  }
#ifdef DEBUG
  printf("scale[%d] finished\n",s->id);
#endif
  return 0;
}

