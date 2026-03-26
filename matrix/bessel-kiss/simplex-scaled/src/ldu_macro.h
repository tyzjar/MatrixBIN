extern unsigned int weight_pos;
extern unsigned int weight_len;
extern unsigned int gross_pos;
extern unsigned int gross_len;

extern char *status[4];
extern unsigned int status_pos;
extern unsigned int status_len;

#define DISTRIBUTE_WEIGHT(_wtmsg) { \
struct weight_struct w_s; \
char sweight[8]; \
enum estatus istatus=0; \
char *clmsg;	\
int clmsg_len;	\
/*  printf("%s\n",wtmsg); */ \
  (void)gettimeofday(&w_s.ts,NULL); \
  strncpy(sweight,&(_wtmsg)[weight_pos],weight_len); \
  w_s.weight=atof(sweight);	\
  w_s.status=0;				\
/* BIT STATUS */ \
  for(unsigned int i=0;i<status_len;i++) \
	if( status[i][0]==(_wtmsg)[status_pos] ) istatus=i; \
  switch( istatus ){	\
	case STATUS_STABLE:	\
		  SET_BIT(w_s.status,STABLE); break; \
	case STATUS_UNSTABLE:	\
	case STATUS_OVERLOAD:	\
	case STATUS_UNDERLOAD:		  break; \
  } \
  w_s.channel=s->id; \
/* <MJK 27.05.2025  */ \
  if( gross_pos ){ \
	strncpy(sweight,&(_wtmsg)[gross_pos],gross_len); \
	w_s.tare=atof(sweight)-w_s.weight; \
  } \
  else w_s.tare=0; \
/* > */ \
  clmsg_len=0; clmsg=NULL; \
  clmsg_len=pack_weight(&clmsg,&w_s); \
/* distribute message */ \
  for(int i=0;i<MAX_CLIENTS;++i){ \
	scale_client_t *c=&cli_array[i]; \
	int wrfd=c->wrfd[s->idx]; \
/* printf("%d : %d\n",i,wrfd); */ \
	if( wrfd==-1 ) continue; \
	if( c->fin ){ \
	  close(wrfd); \
	  c->wrfd[s->idx]=-1; \
/* printf("Close wrdf fin: %d %d\n",s->idx,c->wrfd[s->idx]); */ \
	  continue; \
	} \
	if( write(wrfd,clmsg,clmsg_len)<0 ){ \
	  close(wrfd); \
	  c->wrfd[s->idx]=-1; \
/* printf("Close wrdf: %d %d\n",s->idx,c->wrfd[s->idx]); */ \
	} \
  } \
  free(clmsg); \
}

