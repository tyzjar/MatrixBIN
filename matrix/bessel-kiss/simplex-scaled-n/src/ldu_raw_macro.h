#define DISTRIBUTE_DATA(_wtmsg,_w_pos,_w_len) { \
struct weight_struct w_s; \
char sweight[8]; \
char *clmsg;	\
int clmsg_len;	\
/*  printf("%s\n",wtmsg); */ \
  (void)gettimeofday(&w_s.ts,NULL); \
  strncpy(sweight,&(_wtmsg)[(_w_pos)],(_w_len)); \
  w_s.weight=atof(sweight);	\
  w_s.status=0;				\
  w_s.channel=s->id; \
  w_s.tare=0; \
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

