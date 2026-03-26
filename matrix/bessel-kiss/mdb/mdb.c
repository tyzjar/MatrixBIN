#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <errno.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <unistd.h> // read, write
#include "mdb.h"

extern int to_mqtt(int,unsigned short);
extern int serialize_db(unsigned short *);

extern int verb;
/*
int pack_str(char *dst, char *src)
{
int len, regs;
unsigned short *r=(unsigned short *)dst;
  len=strlen(src);
  if( len==0 ){
	*r=0;
	return 0;
  }
  regs=len/2;
  if( 2*regs<len ) regs++;
  *r=regs;
  r[regs]=0;
  memcpy((char *)&dst[2],src,len);
  return regs;
}

void fill_rtdb()
{
char *p;
  rt_db[0]=wgt[0]; // netto
  rt_db[1]=wgt[1]; // brutto
  rt_db[2]=wgt[2]; // tare
  p=(char *)&rt_db[3];
  p[0]=w.stable;
  p[1]=w.status;
  p[2]=w.err;
  p[3]=0;
  if( carnum_bytes>0 ){ // pack number
	carnum_regs=carnum_bytes/2;
	if( 2*carnum_regs<carnum_bytes ) carnum_regs++;
	p=(char *)&rt_db[4];
	rt_db[4]=carnum_regs;
	memcpy(p,carnumber,carnum_bytes);
  }
}
*/
int slave=97;

int proc_mdb(char *mdb_buf)
{
//struct timespec ts;
unsigned short rt_db[120];
int size, num_regs, reg0, i;
unsigned short val;
unsigned char *q=(unsigned char *)&reg0;
unsigned char *p, *b;

// parse message
// the same for 0x04 and 0x10
  reg0=0;
  q[0]=REG_0Lo(mdb_buf);
  q[1]=REG_0Hi(mdb_buf);
  num_regs=REGS_Lo(mdb_buf);

  if( FUNC(mdb_buf)==0x4 ){ // read holing regs
//printf("0x04\n");
	serialize_db(rt_db);
	b=(unsigned char *)&mdb_buf[REP04_REG0];
	for(i=0;i<num_regs;i++,b+=2){
	  p=(unsigned char *)&rt_db[reg0+i];
	  b[0]=p[1]; b[1]=p[0];
	}
	size=2*num_regs;
	mdb_buf[REP04_BYTES]=size;
	size+=3;
	BYTES(mdb_buf)=size;
	size+=6;
  }
  else if( FUNC(mdb_buf)==0x10 ){ // preset multiple regs
	b=(unsigned char *)&mdb_buf[REQ10_REG0];
	p=(unsigned char *)&val;
	for(i=0;i<num_regs;i++,b+=2){
	  p[1]=b[0]; p[0]=b[1];
	  if( to_mqtt(reg0+1,val)==-1 ){
		  // invalid register
	  }
	}
	size=12; // OK
  }
  return size;
}

void prtmsg(int count,char *buf)
{
int i;
  for(i=0;i<count;i++)
	printf("%2.2x ",buf[i]&0xff);
  printf("\n");
}

void proc_mdb_msg(int fd,int n)
{
char mdb_buf[256];
int rep_size, len;
  len=read(fd,mdb_buf,n);
  if( verb ) prtmsg(len,mdb_buf);
  if( len<7 || BYTES(mdb_buf)<6 ){
	fprintf(stderr,"Invalid message %d bytes\n",len);
	return;
  }
  // parse message
  rep_size=proc_mdb(mdb_buf);
  write(fd,mdb_buf,rep_size);
}


