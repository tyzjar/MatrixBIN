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

void *dis_loop(void *context)
{
scale_sys_t *s=(scale_sys_t *)context;
int fd;
int ready;
fd_set readfds;
fd_set errfs;
char *pStart;
char *pEnd;
char buffer[64];

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

  ser_settings(s->ser);

  pStart=pEnd=&buffer[0];

  // start pselect eternal loop
  while( shutdown_flag!=1 )
  {
	// check for pending command
	if( s->cmd ){
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
	  sem_post(s->cmd->sem);
	  s->cmd=NULL;
	  continue;
	}
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
//	  int writeLen=write(fd,"SW\r",3);
//	  UNUSED(writeLen);
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
		  // replace for config packet size
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
printf("%f\n",w_s.weight);
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

