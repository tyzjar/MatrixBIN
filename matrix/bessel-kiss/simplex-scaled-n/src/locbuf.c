#include <stdio.h>
#include <string.h>
#include <locbuf.h>
#include <unistd.h> // read

/*
#define LOCBUF_SIZE 1024

struct tag_locbuf {
  int  count;	// current number of bytes
  char *head;
  char *tail;
  char spc[LOCBUF_SIZE];
};
*/

locbuf_t *new_locbuf()
{
locbuf_t *b=NULL;
  b=(locbuf_t *)malloc(sizeof(struct tag_locbuf));
  if( b ){
	b->count=0;
	b->head=b->spc;
	b->tail=b->spc;
  }
  memset(b->spc,0,LOCBUF_SIZE);
  return b;
}

void init_locbuf(locbuf_t *b)
{
  if( b ){
	b->count=0;
	b->head=b->spc;
	b->tail=b->spc;
  }
  memset(b->spc,0,LOCBUF_SIZE);
}

void delete_locbuf(locbuf_t *b)
{
  if( b ){
	free(b);
	b=NULL;
  }
}

int get_fromfd(locbuf_t *b,int fd,int bytes)
{
int len;
  len=LOCBUF_SIZE-b->count;
  if( len<=0 || bytes<=0 ) return -1; // buffer is full, no attempt to read
  if( len>bytes ) len=bytes;
  len=read(fd,b->tail,len);
  b->tail+=len;
  b->count+=len;
  return len;
}

// size<LOCBUF_SIZE-size
char *next_by_size(locbuf_t *b,int size)
{
char *p;
  if( b->count<size ){
	if( b->head>b->spc ){
	  memcpy(b->spc,b->head,b->count);
	  b->head=b->spc;
	  b->tail=b->head+b->count;
	}
	return NULL;
  }
  else{
	p=b->head;
	b->head+=size;
	b->count-=size;
	return p;
  }
}

char *next_by_sym(locbuf_t *b,char sym,int *size)
{
char *p, *q=NULL;
int len;
  if( b->count==0 ) { *size=0; return NULL; }
  for(p=b->head;p<b->tail;p++){
	if( sym==*p ){
	  q=b->head;
	  len=p+1-b->head;
	  b->head=p+1;
	  b->count-=len;
	  *size=len;
	  break;
	}
  }
  if( q!=NULL ){
	if( b->count==0 ){ b->head=b->tail=b->spc; }
	return q;
  }
  if( b->head>b->spc ){
	memcpy(b->spc,b->head,b->count);
	b->head=b->spc;
	b->tail=b->head+b->count;
  }
  *size=0;
  return NULL;
}

// 1). If sym is not the last char in the buffer
// size is set to the number of remaining bytes, NULL is returned,
// all bytes preceded the last sym are discarded, so new
// incoming bytes will be appended
// 2). If sym is not the last char in the buffer
// pointer to the next byte after the pre-last occurence
// of sym, size is set to the number of symbols between sym's,
// the buffer is re-inited
char *last_by_sym(locbuf_t *b,char sym,int *size)
{
char *p, *q;
  if( b->count==0 ) { *size=0; return NULL; }
  q=&b->spc[b->count-1];
  for(p=q-1;p>=b->head;p--)
	if( *p==sym ) break;
  p++;
  *size=b->count=q-p+1;
  b->head=b->spc;
  if( *q==sym ){
	b->count=0;
	b->tail=b->spc;
	return p;
  }
  else{
	if( p>b->spc )
	  memcpy(b->spc,p,b->count);
	b->tail=b->head+b->count;
	return NULL;
  }
}

// discard all messages separated by 'sym' in the system buffer
void purge_sysbuf(locbuf_t *b,char sym,int fd,int bytes)
{
int n, m;
  while( (n=get_fromfd(b,fd,bytes))>0 ){
	bytes-=n;
	while( next_by_sym(b,sym,&m)!=NULL )
	 ;
  }
}

#ifdef LTEST
//gcc -o lb locbuf.c -I. -DLTEST
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//char bb[]={'a','a','z','b','z'};
//char bb[]={'a','a','z','z'};
char bb[]={'a','a','z','b','z'};

int main()
{
locbuf_t *lb;
int ff;
int n, m;
char *p, c;

//  ff=open("./ptr_queue.c",O_RDONLY);
/*
  ff=open("./scaled.h",O_RDONLY);
  if( ff==-1 ){
	printf("Failed to open file\n");
	return -1;
  }
*/
  lb=new_locbuf();
  memcpy(lb->spc,bb,sizeof(bb));
  lb->count=sizeof(bb);
  lb->tail+=sizeof(bb);
/*
// Test next_by_size
  while( (n=get_fromfd(lb,ff))>0 ){
	while( (p=next_by_size(lb,18))!=NULL ){
	  c=*lb->head;
	  *lb->head=0;
	  printf("%s",p);
	  *lb->head=c;
	}
  }
*/

// Test next_by_sym
/*
  while( (n=get_fromfd(lb,ff,'\n'))>0 ){
	while( (p=next_by_sym(lb,'\n',&m))!=NULL ){
	  p[m-1]=0;
	  printf("%s\n",p);
	}
  }
*/
  printf("lb: %d %s\n",lb->count,lb->head);
  p=last_by_sym(lb,'z',&m);
  if( lb->count>0 ){
	*lb->tail=0;
	printf("lb: %d %s\n",lb->count,lb->head);
  }
  printf("m,p: %d %s\n",m,p);
  return 0;
}
#endif
