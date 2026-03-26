#ifndef _locbuf_h
#define _locbuf_h

#include <sys/types.h>
#include <malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOCBUF_SIZE 1024

struct tag_locbuf {
  int  count;	// current number of bytes
  char *head;
  char *tail;
  char spc[LOCBUF_SIZE];
};

typedef struct tag_locbuf locbuf_t;

extern locbuf_t *new_locbuf();
//extern void *head(ptr_queue_t *q);
//extern void *dequeue(ptr_queue_t *q);
extern int get_fromfd(locbuf_t *b,int fd,int bytes);
extern char *next_by_size(locbuf_t *b,int size);
extern char *next_by_sym(locbuf_t *b,char sym,int *size);
extern char *last_by_sym(locbuf_t *b,char sym,int *size);
extern void purge_sysbuf(locbuf_t *b,char sym,int fd,int bytes);
extern void init_locbuf(locbuf_t *);
extern void delete_locbuf(locbuf_t *);


#ifdef __cplusplus
};
#endif

#endif
