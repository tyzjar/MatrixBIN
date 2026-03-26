#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

extern int verb;
extern volatile sig_atomic_t shutdown_flag;
extern void proc_mdb_msg(int,int);
extern int init_socket(int);

// Allow only one connection!!!
//int sock=-1;

//char ip_address[24];
#define MAX_CNT 4
#define MAX_CONNECTIONS 4
pthread_t connection_thr[MAX_CONNECTIONS];
//int tid[MAX_CONNECTIONS];
int fds[MAX_CONNECTIONS];

void *proc_connect(void *context)
{
int tid=*(int *)context;
int csfd=fds[tid];
struct timespec ts;
fd_set rfds;
int ready, len, tm_cnt=0;
  while( !shutdown_flag ){
	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(csfd,&rfds);
	/* Wait up to second. */
	ts.tv_sec=1;
	ts.tv_nsec = 0;
	errno=0;
	ready=pselect(csfd+1,&rfds,NULL,NULL,&ts,0);
	if( ready==-1 ){
	  if( verb ) perror("select()");
	  if( errno==EBADF || errno==ENOMEM )
		break;
	}
	else if(ready){
	  if( FD_ISSET(csfd,&rfds) ){
		ioctl(csfd,FIONREAD,&len);
		if( len==0 )
		  break;
		proc_mdb_msg(csfd,len);
		tm_cnt=0;
	  }
	}// there is data available
	else{
	  if( verb ) printf("Timeout.\n");
	  tm_cnt++;
	  if( tm_cnt>=MAX_CNT )
		break;
	}
  }
  close(csfd);
  fds[tid]=-1;
  return NULL;
}

void tcp_loop()
{
int msfd=-1, fd;
socklen_t size;
struct sockaddr_in from;
int i;

  if( (msfd=init_socket(502))==-1 )
	return;

  for(i=0;i<MAX_CONNECTIONS;i++)
	fds[i]=-1;

  while( !shutdown_flag ){
	listen(msfd,4);
	fd=accept(msfd,(struct sockaddr *)&from,&size);
	if( fd==-1 ){
	  perror("accept()");
	  continue;
	}
	for(i=0;i<MAX_CONNECTIONS;i++){
	  if( fds[i]==-1 )
		break;
	}
	if( i==MAX_CONNECTIONS ){
	  if( verb ) printf("Connection refused\n");
	  continue;
	}
	fds[i]=fd;
	if( pthread_create(&connection_thr[i],NULL,proc_connect,(void *)&i)==0 ){
	  if( verb ) printf("New connection accepted\n");
	}
  }
  return;
}

