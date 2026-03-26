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
#include <syslog.h>
#include <signal.h>

#include <errno.h>
#include "scaled.h"

extern volatile sig_atomic_t shutdown_flag;
extern scale_client_t cli_array[MAX_CLIENTS];
extern int scale_cnt;

// Регистрация клиентского соединения и запуск нити 
int register_client(int socket_fd)
{
int i=0;
int pipefd[2];
scale_client_t *c;
  for(i=0;i<MAX_CLIENTS;++i)
	if(cli_array[i].sock==-1 ) break;
  if( i==MAX_CLIENTS ){
	syslog(LOG_CRIT,"No resource to register client");
//printf("No resource to register client\n");
	return -1;
  }
//printf("register %d\n",i);
  c=&cli_array[i];
  // Register new client
  c->sock=socket_fd;
  for(int j=0;j<scale_cnt;j++){
	if( pipe2(pipefd,O_NONBLOCK)==-1 ){
	  perror("pipe creation");
	  syslog(LOG_CRIT, "Error on initializing pipes");
	  exit(EXIT_FAILURE);
	}
	c->rdfd[j]=pipefd[0];
	c->wrfd[j]=pipefd[1];
  }
  return i;
}

void clean_client(scale_client_t *c)
{
int j;
  c->sock=-1;
  for(j=0;j<scale_cnt;j++){
	if( c->rdfd[j]!=-1 ) close(c->rdfd[j]);
	c->rdfd[j]=-1;
	if( c->wrfd[j]!=-1 ) close(c->wrfd[j]);
	c->wrfd[j]=-1;
  }
}

void *packet_feed(void *context)
{
scale_client_t *c=(scale_client_t *)context;
int status = EXIT_SUCCESS;
sigset_t mask, omask;	// This is the client process
fd_set rfds;
fd_set errfs;
int retval, max_fd;

  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_BLOCK,&mask,&omask);

  while( c->go ){
	FD_ZERO(&rfds);
	FD_ZERO(&errfs);
	FD_SET(c->sock,&errfs);
	max_fd=c->sock;
	for(int i=0;i<scale_cnt;i++){
	  if( c->rdfd[i]==-1 ) continue;
	  FD_SET(c->rdfd[i],&rfds);
	  if( max_fd<c->rdfd[i] ) max_fd=c->rdfd[i];
	}

	struct timespec tv;
	tv.tv_sec=3;
	tv.tv_nsec=0;
	retval=pselect(max_fd+1,&rfds,0,&errfs,&tv,&omask);
	if( retval==0 ) continue; //syslog(LOG_CRIT, "device_daemon: Timeout reached reading from device.");
	if( FD_ISSET(c->sock,&errfs) )
	  printf("Exception caught on socket");

	//sigprocmask(SIG_SETMASK, &omask, 0);
	if( errno==EINTR ){
	  syslog(LOG_INFO, "Interrupted by SIGTERM");
	  continue;
	}
	if( retval==-1 ){
	  perror("select()");
	  syslog(LOG_CRIT, "Error on select()");
	}
	else{
	  for(int i=0;i<scale_cnt;i++){
		if( FD_ISSET(c->rdfd[i],&rfds) ){ // data from server (данные из ser_loop())
		  size_t len = 0;
		  ioctl(c->rdfd[i],FIONREAD,&len);
		  if( len==0 ){
			syslog(LOG_INFO,"Client disconnected. Empty read on socket.");
//printf("Client disconnected. Empty read on socket.\n");
//continue;
			status = EXIT_FAILURE;
			goto quit;
		  }

		  size_t read_len;
		  // while not all readed
		  while( len>0 ){
			char data[1024] = {0};
			if( len>sizeof(data) )
			  read_len=read(c->rdfd[i],&data,sizeof(data));
			else
			  read_len=read(c->rdfd[i],&data,len);
//#ifdef DEBUG
//			struct timeval tv;
//			gettimeofday(&tv,NULL);
//			printf("<%ld.%06ld> Data from server length: %d\n", (long int)(tv.tv_sec), (long int)(tv.tv_usec), (unsigned int)len);
//			printf("<%ld.%06ld> Data from server: %s\n", (long int)(tv.tv_sec), (long int)(tv.tv_usec), data);
//#endif
			int n=write(c->sock,data,read_len);
			if( n<0 ){
			  syslog(LOG_CRIT,"ERROR writing to socket, closing socket");
//printf("ERROR writing to socket, closing socket\n");
			  status = EXIT_SUCCESS;
			  goto quit;
			}
			len-=read_len;
		  }
		}
	  }//for
	}
  }
quit:
  syslog(LOG_INFO, "Client disconnected");
//  printf("Client disconnected\n");
  for(int i=0;i<scale_cnt;i++){
	if( c->rdfd[i]!=-1 ) close(c->rdfd[i]);
	c->rdfd[i]=-1;
  }
  // ask all scales to close their pipes
  c->fin=1;
  close(c->sock);
  c->sock=-1;
//  printf("client finished\n");
  c->thr=-1;
  pthread_exit((void*)&status);
}

// Нить, ожидающая соединения клиентских программ (например, devdrvd)
// This thread is tightly hanging on accept and normally can only be terminated
// by calling pthread_cancel on it (see signal handler in scaled.c)
void *runpacket(void *context)
{
int sockfd=*((int*)context);
int newsockfd;
socklen_t clilen;
struct sockaddr_in cli_addr;
int cli_idx;
scale_client_t *c;

  listen(sockfd,5);
  clilen=sizeof(cli_addr);
  while( !shutdown_flag ){
	newsockfd=accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);
	if( errno==EINVAL || errno==EINTR ){
	  continue;
	}
	else if( newsockfd<0 ){
	  perror("accept");
	  syslog(LOG_CRIT, "ERROR on accept: %d", errno);
	  exit(EXIT_FAILURE);
	}
	if( (cli_idx=register_client(newsockfd))<0 ) continue;
	c=&cli_array[cli_idx];
	c->fin=0;
	c->go=1;
	if( pthread_create(&(c->thr),NULL,packet_feed,c)!=0 ){
	  perror("Could not create client thread");
	  syslog(LOG_CRIT,"Could not create client thread");
	  c->go=0;
	  clean_client(c);  
	  break;
	}
//printf("Thread created\n");
  }
  return 0;
}

