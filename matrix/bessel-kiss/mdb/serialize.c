// gcc -o aa aa.c
#include <stdio.h>
#include <string.h>
#include "prot.h"


///// database (mqtt_thread) //////
extern wgt_t wgt;
extern car_t car;
extern int seals_count;
extern seal_t seal[];
///////////////

// *dst=number of subsequent regs
// returns the number of occupied registers
int pack_str(char *dst, char *src)
{
int len, regs;
unsigned short *r=(unsigned short *)dst;
  len=strlen(src);
  if( len==0 ){
	*r=0;
	return 1;
  }
  regs=len/2;
  if( 2*regs<len ) regs++;
  *r=regs;
  r[regs]=0;
  memcpy((char *)&dst[2],src,len);
  return regs+1;
}

// returns the number of registers
int serialize_db(unsigned short *msg)
{
int i;
int pos;
seal_t *se;
// weights
  for(i=0;i<5;i++) msg[i]=wgt.d[i];
  pos=5;
// car
  msg[pos]=car.qu.d; pos++;
  pos+=pack_str((char *)&msg[pos],car.id);
// seals
  msg[pos]=seals_count; pos++;
  for(i=0;i<seals_count;i++){
	se=&seal[i];
	msg[pos]=se->num; pos++;	// seal number
	msg[pos]=se->qu.d; pos++;	// seal quality
	pos+=pack_str((char *)&msg[pos],se->id); // seal itself
  }
  return pos;
}

