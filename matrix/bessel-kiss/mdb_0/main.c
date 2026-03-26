#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>	// pause
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
//#include "mdb.h"

extern void *tcp_thr(void *);
extern void *mqtt_thr(void *);

extern int slave;
//extern char *ip_address;

int verb=0;
volatile sig_atomic_t shutdown_flag=0;
pthread_t tcp_thread;
pthread_t mqtt_thread;

void signal_handler(int sig)
{
  switch(sig) {
	case SIGINT:
	case SIGTERM:
	  printf("Received SIGTERM signal.");
	  printf("Cleaning and shutting down.");
	  shutdown_flag=1;
	  pthread_cancel(mqtt_thread);
	  pthread_cancel(tcp_thread);
	  break;
	case SIGHUP:
	  printf("Received SIGHUP signal.");
	  printf("Reloading config file.");
	  break;
	case SIGUSR1:
	  printf("Received SIGUSR1 signal.");
	  break;
	case SIGUSR2:
	  printf("Received SIGUSR2 signal.");
	  break;
	default:
	  printf("Unhandled signal %s", strsignal(sig));
	  break;
  }
}

void set_signal_handler()
{
struct sigaction sa;
sigset_t emptyset, blockset;

  sigemptyset(&blockset);		// Block SIGINT
  //sigaddset(&blockset, SIGINT);
  sigaddset(&blockset,SIGHUP);
  sigaddset(&blockset,SIGTERM);
  sigaddset(&blockset,SIGQUIT);
  sigaddset(&blockset,SIGCHLD);
  sigaddset(&blockset,SIGPIPE);

  sigprocmask(SIG_BLOCK,&blockset,NULL);

  sa.sa_handler=signal_handler;	// Attach signal handler
  sa.sa_flags=0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT,&sa,NULL);
  sigaction(SIGCHLD,&sa,NULL);
  sigaction(SIGUSR1,&sa,NULL);
  sigaction(SIGUSR2,&sa,NULL);

  sigemptyset(&emptyset);
}


int main(int argc, char *argv[])
{
int c=-1;

  while( (c=getopt(argc,argv,"s:v"))!=-1 ){
	switch(c){
/*
	  case 'a':
		ip_address=optarg;
		break;
*/
	  case 's':
		slave=atoi(optarg);
		break;
	  case 'v':
		verb=1;
		break;
	  default:
		break;
	}
  }//while getopt


  set_signal_handler();

  if(pthread_create(&tcp_thread,NULL,tcp_thr,NULL)!=0 ){
	perror("Could not modbus thread");
	exit(EXIT_FAILURE);
  }
  if(pthread_create(&mqtt_thread,NULL,mqtt_thr,NULL)!=0 ){
	perror("Could not create mqtt thread");
	exit(EXIT_FAILURE);
  }

  pause();

//printf("FIN %d\n",shutdown_flag);
//  close(control_sockfd);
  return 0;
}

