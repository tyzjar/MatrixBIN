#include <sys/select.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>	// atof etc
#include <unistd.h>	// write
#include <string.h>
#include <ctype.h> // isprint
#include <errno.h>
#include "locbuf.h"

#define UNUSED(A) (void)(A)

int catch_cmd_reply(int fd)
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

int get_cmd_reply2(int fd)
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

/*
int write_calibr(char *num,int id)
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
*/

char *get_resp(int fd,locbuf_t *l,int *size)
{
struct timespec tv;
fd_set rfd;
int ready, nfds, bytes, n, m=0;
char *p;

  FD_ZERO(&rfd);
  nfds=fd;
  nfds++;
  for(;;){
	tv.tv_sec=2;
	tv.tv_nsec=0;	// nanoseconds
	FD_SET(fd,&rfd);
	ready=pselect(nfds,&rfd,0,0,&tv,0);
//printf("%d\n",ready);
	if( ready==-1 ){
#ifdef DEBUG
	  perror("get_resp, pselect()");
#endif
	}
	else if( ready==0 ){
	  // timeout
//printf("Select timeout\n");
	  *size=-1;
	  return NULL;
	}
	else if( ready>0 ){
	  ioctl(fd,FIONREAD,&bytes);
//printf("bytes=%d\n",bytes);
	  if( bytes<=0 ){
#ifdef DEBUG
		printf("Invalid descriptor %d\n",fd);
#endif
		*size=-2;
		return NULL;
	  }
	  while( (n=get_fromfd(l,fd,bytes))>0 ){
//printf("n=%d\n",n);
		p=last_by_sym(l,'\r',&m);
//printf("m=%d p=%p\n",m,p);
		if( p!=NULL ) break;
		bytes-=n;
	  }
	  if( p!=NULL ) break;
	}// if ready to read
  }//for
  *size=m;
  return p;
}

void mk_prt_str(int n, char *in, char *out)
{
int i, j;
  for(i=0,j=0;i<n;i++,j++){
	if( isprint(in[i]) ){
	  out[j]=in[i];
	}
	else{
	  out[j]='['; j++;
	  sprintf(&out[j],"%2.2x",in[i]); j+=2;
	  out[j]=']';
	}
  }
  out[j]=0;
}

#define PRT_RESP() if(resp){ resp[n-1]=0; printf("LDU: %s\n",resp); }

int init_ldu(int fd,int id,locbuf_t *l)
{
char *resp, *p;
int i, n;
//char bb[32];
  // Just in case stop flow
  UNUSED(id);
  write(fd,"ID\r",3);
  write(fd,"ID\r",3);
  for(i=0;i<5;i++){
	resp=NULL;
	resp=get_resp(fd,l,&n);
	if( resp==NULL && n==-1 ) // timeout
	  break;
	if( resp && resp[0]=='D' )
	  break;
  }
  if( resp==NULL && n==-1 ){ // timeout occured
	// In this situation the flow is already stopped,
	// so one make more attempt to establish communication
	init_locbuf(l);
	write(fd,"ID\r",3);
	resp=NULL;
	resp=get_resp(fd,l,&n);
  }

  printf("Device ID request\n");
  PRT_RESP();
  if( resp==NULL ){
#ifdef DEBUG
	printf("LDU not responding\n");
#endif
	return -1;
  }
  // Ask calibration number
  write(fd,"CE\r",3);
  resp=NULL;
  resp=get_resp(fd,l,&n);
  p=NULL;
  i=0;

  printf("Calibration number request\n");
  PRT_RESP();

  if( n>4 ){ // not "ERR\r"
	resp[7]=0;
	p=&resp[2];
	while( *p=='0' ) p++;
	if( *p==0 ) p=NULL;
	i=atoi(p); // str is not a number
  }
#ifdef DEBUG
  if( p!=NULL ) printf("%s\n",p);
#endif
  // set zero
  write(fd,"SZ\r",3);
//  if( i>0 ) write_calibr(p,id);
  // get response to clean system buffer
  resp=get_resp(fd,l,&n);
  // publish in mqtt either real number or error

  printf("Set zero request\n");
  PRT_RESP();

  return i;
}

