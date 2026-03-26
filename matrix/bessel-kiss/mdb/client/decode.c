// gcc -o aa aa.c
#include <stdio.h>
#include <string.h>
#include <prot.h>


void prt_wgt(wgt_t *w)
{
  printf("%d:%d %d %d %d %d\n",
	w->wgt.qu.q.type,w->wgt.qu.q.err,w->d[1],w->d[2],w->d[3],w->d[4]);
}

int prt_car(car_t *c)
{
int n;
char b[32];
  memset(b,0,32);
  n=2*c->regs;
  memcpy(b,c->id,n);
  b[n]=0;
  printf("%d:%d %d %s\n",
	c->qu.q.type,c->qu.q.err,c->regs,b);
  return (2+c->regs);
}

int prt_seal(seal_t *s)
{
int n;
char b[32];
  memset(b,0,32);
  n=2*s->regs;
  memcpy(b,s->id,n);
  b[n]=0;
  printf("%d %d:%d %d %s\n",
	s->num,s->qu.q.type,s->qu.q.err,s->regs,b);
  return (3+s->regs);
}

void prt_msg(unsigned short *msg)
{
wgt_t *wgt;
car_t *car;
seal_t *seal;
int pos,n,i;
  wgt=(wgt_t *)msg;
  prt_wgt(wgt);
  pos=5;
  car=(car_t *)&msg[pos];
  pos+=prt_car(car);
  n=msg[pos];
  printf("%d No seals=%d\n",pos,n);
  pos++;
  for(i=0;i<n;i++){
	seal=(seal_t *)&msg[pos];
	pos+=prt_seal(seal);
  }
}

