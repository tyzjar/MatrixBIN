// gcc -o aa aa.c
#include <stdio.h>
#include <string.h>
#include "prot.h"

wgt_t wgt;
car_t car;
int seals_count;
seal_t seal[6];

char s1[]="BMOU1031733";
char s2[]="TLLU26092245";
char ss1[]="P123456";
char ss2[]="Q123456";

void init_db()
{
  seals_count=0;
  memset((char *)&wgt,0,sizeof(wgt_t));
  memset((char *)&car,0,sizeof(car_t));
  memset((char *)&seal[0],0,6*sizeof(seal_t));
}

void init_test_db()
{
data_qu_t dq;
seal_t *se;
int len;
char *s;
  // weights
  dq.q.type=0;	// normal
  dq.q.err=0;	// no error
  wgt.wgt.qu=dq;	// quality
  wgt.wgt.stb=1;
  wgt.wgt.w[0]=345;		// brutto
  wgt.wgt.w[1]=wgt.wgt.w[0];// netto
  wgt.wgt.w[2]=0;			//tare
  // car description
  car.qu.q.type=1; // manual
  car.qu.q.err=0;	// no error
  s=s2;
  len=strlen(s);
  memcpy(&car.id,s,len);
  car.id[len]=0;
//  printf("%d %s\n",car.qu.d,car.id);
  // seals
  seals_count=2;
  se=&seal[0];
  s=ss1;
  se->num=1;		// number
  se->qu.q.type=0;	// quality
  se->qu.q.err=0;
  len=strlen(s);
  memcpy(&se->id,s,len);
  se->id[len]=0;

  se=&seal[1];
  s=ss2;
  se->num=2;		// number
  se->qu.q.type=1;	// quality
  se->qu.q.err=0;
  len=strlen(s);
  memcpy(&se->id,s,len);
  se->id[len]=0;
}


