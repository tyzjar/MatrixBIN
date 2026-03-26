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

//extern unsigned int weight_pos;
extern unsigned int weight_len;
//extern unsigned int gross_pos;	// nonzero value we interpret as indication to use gross weight

extern char *gross[2];	// GROSS=0, NET=1
// gross[NET & gros[GROSS]	msg
//		0			0		G,o
//		0			1		G,T
//		1			0		N,0
//		1			1		N,T
//extern unsigned int status_pos;
//extern unsigned int status_len;

extern volatile sig_atomic_t shutdown_flag;
extern void ser_settings(struct rs232_port_t *ser);

extern scale_client_t cli_array[];

int readnbytes(int fd,int n,char *buf)
{
int len, bytes;
char *p;
int trials=0;
  len=0;
  do{
	p=buf+len;
	bytes=read(fd,p,n-len);
#ifdef DEBUG
	printf("read %d bytes\n",bytes);
#endif
	if( bytes<0 ){
	  perror("read:");
	  break;
	}
	else if( bytes==0 ){
	  trials++;
	  if( trials>=5 ){
#ifdef DEBUG
		printf("device is not responding\n");
#endif
		break;
	  }
	}
	else{
	  len+=bytes;
	  trials=0;
	}
  } while(len<n);
  return len;
}

// returns -1 if comm error, 1 if cmd not executed, 0 if OK
int do_cdl(int fd)
{
int len;
char buf[8];
char cdl[4]="CDL\r";		// set zero = clear dead load
  write(fd,cdl,sizeof cdl);
  len=readnbytes(fd,3,buf);
  if( len<3 )
	return -1;
  else if( buf[0]!=30 )
	return 1;
  return 0;
}

// mode=?/0/1=ask_current_mode/net/gross
int oper_mode(int fd,char mode,unsigned int *fields)
{
char buf[8];
char tas[5]="TAS?\r";	// ask/set mode net/gross
int len;
int net_on=0;		// current weight is net/gross
  tas[3]=mode;
  write(fd,tas,sizeof tas);
  len=readnbytes(fd,3,buf);
  if( len<3 || buf[0]!=30 || buf[0]!=31 )
	return -1;
  net_on= (buf[0]==30) ? 1:0;
  if( net_on ) SET_BIT(*fields,NETGROSS);
  else CLEAR_BIT(*fields,NETGROSS);
  return 0;
}

int get_tare_val(int fd,int *val)
{
char buf[16];
char tav[5]="TAV?\r";	// ask current tare value
int len;
  write(fd,tav,sizeof tav);
  len=readnbytes(fd,10,buf);
  if( len<10 )
	return -1;
  buf[8]=0;
  *val=atoi(buf);
  return 0;
}

// mode=?/0/1=ask_current_mode/net/gross
int set_tare_val(int fd)
{
char buf[8];
char tar[4]="TAR\r";		// write current weight to memory as tare value
int len;
char cdl[4]="CDL\r";		// set zero = clear dead load
  write(fd,cdl,sizeof tar);
  len=readnbytes(fd,3,buf);
  if( len<3 )
	return -1;
  else if( buf[0]!=30 )
	return 1;
  return 0;
}

void exec_cmd(int fd,struct cmd_pending_str *c,int *tare)
{
  switch (c->cmdbuf[0]){
	case 'Z':	// set zero
	  c->rc=do_cdl(fd);
	  break;
	case 'T':	// set tare
	  c->rc=set_tare_val(fd);
	  if( c->rc==0 && tare!=NULL )
		get_tare_val(fd,tare);
	  break;
	default: // if incorrect opcode
	  c->rc=-1;
	  break;
  }
}

void *dis2_loop(void *context)
{
scale_sys_t *s=(scale_sys_t *)context;
int len;//, bytes;
int serfd;
#define BUFSIZE 20
char buf[BUFSIZE];
int msg_len=16;
char msv[5]="MSV?\r";	// ask weight

// in this version this var is the same for all scales
int send_tare=0;		// send or not tare value to matrix
unsigned int status=0;
int tare_val=0;		// current tare value
int *ptare_val=NULL;	// pointer to current tare value
int weight=0;		// current weight
unsigned int stst;	// standstill state

//struct timespec tp0, tp1;
//clockid_t clock_type=CLOCK_REALTIME;
//double msec;

struct weight_struct w_s;
char *sweight=(char *)malloc((weight_len+1)*sizeof(char));

  memset(sweight,'\0',(weight_len+1));
  stst=0x20202020;
  // preset fields which will not be modified
  w_s.tare=0;
  w_s.channel=s->id; // this scale system id from cfg file

  if( rs232_open(s->ser) ){
	syslog(LOG_CRIT, "Failed to open port: %s\n",s->ser->dev);
	printf("Failed to open port: %s\n",s->ser->dev);
	ser_settings(s->ser);
	rs232_end(s->ser);
	return NULL;
  }
//  syslog(LOG_INFO,"Open port with port settings: %s\n",rs232_to_string(s->ser));
  rs232_flush(s->ser);
  serfd=rs232_fd(s->ser);

  if( gross[GROSS] && gross[GROSS][0]=='1' ){
	send_tare=1;
	ptare_val=&tare_val;
//	SET_BIT(status,TARE);
  }
/*
  if( get_tare_val(&prev_tare_val)<0 ){
#ifdef DEBUG
	printf("Failed to get tare value\n");
#endif
	return NULL;
  }
*/
  do_cdl(serfd);
  // after this command terminal sends gross weight

  if( send_tare ){
	// now tare value=0, but it is not written in memory
	set_tare_val(serfd);
	if( get_tare_val(serfd,&tare_val)<0 ){
#ifdef DEBUG
	  printf("Failed to get tare value\n");
#endif
	  return NULL;
	}
	// if in cfg net weight is required set this mode
	if( gross[NET] && gross[NET][0]=='1' )
	  oper_mode(serfd,'0',&status);
  }
  packet_length=16;


  // start pselect eternal loop
  while( shutdown_flag!=1 )
  {
	// check for pending command
	// !!!
//	clock_gettime(clock_type,&tp0);
	write(serfd,msv,5);
	len=readnbytes(serfd,msg_len,buf);
	if( len==msg_len ){
//	  clock_gettime(clock_type,&tp1);
//	  msec=(double)(tp1.tv_nsec-tp0.tv_nsec)*1e-6+(tp1.tv_sec-tp0.tv_sec)*1000;

	  if( gettimeofday(&w_s.ts,NULL)!=0 ){
#ifdef DEBUG
		printf("gettimeofday failed!\n");
#endif
	  }
//	  buf[16]=0;
//	  printf("%s\n",buf);
	  if( buf[1]=='-' ){ // "-...-" indicates that the weight is out of range
		SET_BIT(status,OVERLOAD);
	  }
	  else{
		buf[9]=0;
		weight=atoi(buf);
		CLEAR_BIT(status,OVERLOAD);
	  }
	  if( stst==*(unsigned *)&buf[10] )
		CLEAR_BIT(status,STABLE);
	  else
		SET_BIT(status,STABLE);
	  w_s.weight=(float)weight;
	  w_s.status=status;
	  if( send_tare )
		w_s.tare=(float)tare_val;
	  printf("%d kg\n",weight);

	  // distribute data
	  char *pkt=0;
	  size_t pkt_len=0;
	  pkt_len=pack_weight(&pkt,&w_s);

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
		if( write(wrfd,pkt,pkt_len)<0 ){
		  close(wrfd);
		  c->wrfd[s->idx]=-1;
//printf("Close wrdf: %d %d\n",s->idx,c->wrfd[s->idx]);
		}
	  }
	  free(pkt);
	  pkt_len=0;
	}// if message received
	else{
#ifdef DEBUG
	  buf[len]=0;
	  printf("Read from device, discarded bad packet buffer[%d]: %s\n",len,buf);
#endif
	}
	if( s->cmd ){
	  exec_cmd(serfd,s->cmd,ptare_val);
	  sem_post(&s->sem);
	  s->cmd=NULL;
	}
  }
#ifdef DEBUG
  printf("scale[%d] finished\n",s->id);
#endif
  return NULL;
}

