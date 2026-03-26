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


void *tcp_thr(void *context)
{
int msfd=-1, csfd=-1, fd;
socklen_t size;
struct sockaddr_in from;
struct timespec tv;
fd_set rfds;
int ready, len;

  (void)context;
  if( (msfd=init_socket(502))==-1 )
	return NULL;

  while( !shutdown_flag ){
	listen(msfd,4);
	fd=accept(msfd,(struct sockaddr *)&from,&size);
	if( fd==-1 ){
	  perror("accept()");
	  continue;
	}
	if( csfd!=-1 )
	  close(csfd);
	csfd=fd;
	if( verb )
	  printf("New connection accepted\n");
	for(;;){
	  /* Watch stdin (fd 0) to see when it has input. */
	  FD_ZERO(&rfds);
	  FD_SET(csfd,&rfds);
	  /* Wait up to second. */
	  tv.tv_sec=1;
	  tv.tv_nsec = 0;
	  ready=pselect(csfd+1,&rfds,NULL,NULL,&tv,0);
	  if( ready==-1 )
		perror("select()");
	  else if(ready){
		if( FD_ISSET(csfd,&rfds) ){
		  ioctl(csfd,FIONREAD,&len);
		  if( len==0 ){
			close(csfd);
			csfd=-1;
			break;
		  }
		  proc_mdb_msg(csfd,len);
		}
	  }// there is data available
	  else{
	    printf("Timeout.\n");
	  }
	}
  }
  return NULL;
}

