#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>	// pause
#include <signal.h>
#include <syslog.h>
#include "scaled.h"
#include <librs232/rs232.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>

#define DAEMON_NAME "scaled"
#define PROGRAMM_VERSION "1.0.0"


extern int loadConfig(const char* configFile, int* iError);
extern void *ldu_loop(void *);
extern void *dis2_loop(void *);
extern void *dis4_loop(void *);
extern void *runpacket(void *);
extern void *runcommand(void *);
extern void *mqtt_thr(void *);
extern int init_socket(const char *,int);
extern int udp_socket(const char *,int);

volatile sig_atomic_t shutdown_flag=0;
pthread_t scale_thread[MAX_SCALES];
pthread_t data_thread;
pthread_t ctrl_thread;
pthread_t mqtt_thread;

char ip_address[24];
int ip_size=sizeof(ip_address);
int data_portno = 0;
int cmds_portno = 0;
int scale_cnt=0;
scale_sys_t *scale[MAX_SCALES];
int models_cnt=0;
model_cmd_t *mod_cmd[MAX_MODELS];
scale_client_t cli_array[MAX_CLIENTS];

char *configFile=NULL;
int daemon_flag=1;	//daemonizing by default

static struct option long_options[] =
{
{"version",     no_argument,        0,                  'v'},
{"n",           no_argument,        &daemon_flag,       'n'},
{"config",      required_argument,  0,                  'c'},
{"help",        no_argument,        0,                  'h'},
{0, 0, 0, 0}
};

void PrintUsage(int argc)
{
  if( argc>=1 ){
	printf(	"-v, --version       prints version and exits\n" \
			"-c, --config=FILE   specify configuration file\n" \
			"-n                  don\'t fork off as daemon\n" \
			"-h, --help          prints this message\n");
  }
}

void signal_handler(int sig)
{
  switch(sig) {
	case SIGINT:
	case SIGTERM:
	  syslog(LOG_WARNING, "Received SIGTERM signal.");
	  syslog(LOG_INFO, "Cleaning and shutting down.");
	  shutdown_flag=1;
	  pthread_cancel(mqtt_thread);
	  pthread_cancel(ctrl_thread);
	  pthread_cancel(data_thread);
	  break;
	case SIGHUP:
	  syslog(LOG_INFO, "Received SIGHUP signal.");
	  syslog(LOG_INFO, "Reloading config file.");
//	  reload_flag = 1;
	  break;
	case SIGUSR1:
	  syslog(LOG_INFO, "Received SIGUSR1 signal.");
	  break;
	case SIGUSR2:
	  syslog(LOG_INFO, "Received SIGUSR2 signal.");
	  break;
	default:
	  syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sig));
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


extern void ser_settings(struct rs232_port_t *ser);

void prt_term(scale_term_t *s)
{
model_cmd_t *m=s->model;
  printf("addr=%d\n",s->addr);
  printf("terminal model=%s\n",m->model);
  printf("comands supported:\n");
  for(enum cmd_e i=set_zero;i<max_cmd;i++){
	if( m->cmds[i] )
	  printf("\t%s\n",m->cmds[i]);
	else
	  printf("\t-\n");
  }
}

void prt_scale(scale_sys_t *s)
{
  printf("id=%d\n",s->id);
  printf("seral device=%s\n",s->ser->dev);
//  printf("Port settings: %s\n",rs232_to_string(s->ser));
  ser_settings(s->ser);
  printf("number of terminals=%d\n",s->dev_cnt);
  for(int i=0;i<s->dev_cnt;i++){
	printf("\tterminal #%d:\n",i);
	prt_term(s->st[i]);
  }
}

void init_scales()
{
int i;
scale_sys_t *s;
  for(i=0;i<scale_cnt;i++){
	s=scale[i];
	s->idx=i;
//	s->ctlfd[0]=s->ctlfd[1]=-1;
	s->calibr_mode=0;
	s->cmd=NULL;
	if( sem_init(&s->sem,0,0)<0 ){
	  printf("runcommand: failed to create semaphore\n");
	  exit(EXIT_FAILURE);
	}
  }
}

void init_clients()
{
int i;
scale_client_t *c;
  for(i=0;i<MAX_CLIENTS;i++){
	c=&cli_array[i];
	c->thr=-1;
	c->sock=-1;
	c->go=0;
	c->fin=0;
	for(int j=0;j<MAX_SCALES;j++){
	  c->wrfd[j]=c->rdfd[j]=-1;
	}
  }
}

void stop_clients()
{
int i;
  for(i=0;i<MAX_CLIENTS;i++) cli_array[i].go=0;
  for(i=0;i<MAX_CLIENTS;i++)
	if( cli_array[i].sock!=-1 ) pthread_join(cli_array[i].thr,NULL);
//	if( cli_array[i].thr!=-1 ) pthread_join(cli_array[i].thr,NULL);
}


int main(int argc, char *argv[])
{
int option_index=0;
int c=-1;

int iError;
int packet_sockfd=-1;
int cmds_sockfd=-1;

scale_term_t *s;
model_cmd_t *m;

  while( (c=getopt_long(argc,argv,"vnc:h",long_options,&option_index))!=-1 ){
	switch(c){
	  case 'v' :
		printf("%s\n", PROGRAMM_VERSION);
		exit(EXIT_SUCCESS);
		break;
	  case 'c':
		configFile=optarg;
		if( access(configFile,R_OK)!=0 ){
		  printf("Error accessing the given config file: %s\n", configFile);
		  perror(configFile);
		  exit(EXIT_FAILURE);
		}
		printf("Using config file: %s\n", configFile);
		break;
	  case 'n':
		daemon_flag = 0;
		break;
	  case 'h':
		PrintUsage(argc);
		exit(EXIT_SUCCESS);
		break;
	  default:
		break;
	}
  }//while getopt

  if( loadConfig(configFile, &iError)==-1 ){
	printf( "main[%d]: loadConfig() error 0x%X\n",__LINE__,iError);
	return -1;
  }
/*
  for(int i=0;i<scale_cnt;i++){
	printf("scale[%d]:\n",i);
	prt_scale(scale[i]);
  }
  for(int i=0;i<scale_cnt;++i){
	s=scale[i]->st[0];
	m=s->model;
	if( strncmp(m->model,"ldu",3)==0 )
	  printf("%s\n",m->model);
	else if( strncmp(m->model,"dis",3)==0 )
	  printf("%s\n",m->model);
  }
return 0;
*/
//return 0;

#if defined(DEBUG)
  daemon_flag = 0;
  setlogmask(LOG_UPTO(LOG_DEBUG));
  openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
  setlogmask(LOG_UPTO(LOG_INFO));
  openlog(DAEMON_NAME, LOG_CONS, LOG_USER);
#endif
  syslog(LOG_INFO,"%s %s daemon starting up, daemon_flag=%d",DAEMON_NAME,PROGRAMM_VERSION,daemon_flag);

  /** @todo more daemonizing
  */
  /* Our process ID and Session ID */
  pid_t pid, sid;
  if( daemon_flag ){
	syslog(LOG_INFO,"%s: Starting the daemonizing process", DAEMON_NAME);

	/* Fork off the parent process */
	pid=fork();
	if( pid<0 ) exit(EXIT_FAILURE);
	/* If we got a good PID, then
	   we can exit the parent process. */
	if( pid>0) exit(EXIT_SUCCESS);

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if( sid<0 ) // Log the failure
	  exit(EXIT_FAILURE);

	/* Change the current working directory */
	if( (chdir("/"))<0 )  // Log the failure
	  exit(EXIT_FAILURE);

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
  }

  set_signal_handler();
  init_scales();
  init_clients();

//printf("%d %d\n",packet_sockfd,data_portno);
  packet_sockfd=init_socket(ip_address,data_portno);
  syslog(LOG_INFO,"Data socket: %d\n",packet_sockfd);
//printf("%d %d\n",packet_sockfd,data_portno);
  cmds_sockfd=udp_socket(ip_address,cmds_portno);
  syslog(LOG_INFO,"Control socket: %d\n",cmds_sockfd);
//cmds_sockfd=cmds_sockfd;

//  printf("terminal model=%s\n",m->model);
  for(int i=0;i<scale_cnt;++i){
	s=scale[i]->st[0];
	m=s->model;
	if( strncmp(m->model,"ldu",3)==0 )
	  pthread_create(&scale_thread[i],NULL,ldu_loop,(void *)scale[i]);
	else if( strncmp(m->model,"dis2",4)==0 )
	  pthread_create(&scale_thread[i],NULL,dis2_loop,(void *)scale[i]);
	else if( strncmp(m->model,"dis4",4)==0 )
	  pthread_create(&scale_thread[i],NULL,dis4_loop,(void *)scale[i]);
  }

// Now start listening for the clients, here process will go in sleep mode
// and will wait for the incoming connections
  if(pthread_create(&data_thread,NULL,runpacket,(void*)&packet_sockfd)!=0 ){
	perror("Could not create server thread");
	syslog(LOG_CRIT,"Could not create server thread");
	exit(EXIT_FAILURE);
  }
  if(pthread_create(&ctrl_thread,NULL,runcommand,(void*)&cmds_sockfd)!=0 ){
	perror("Could not create control thread");
	syslog(LOG_CRIT,"Could not create control thread");
	exit(EXIT_FAILURE);
  }
  if(pthread_create(&mqtt_thread,NULL,mqtt_thr,NULL)!=0 ){
	perror("Could not create mqtt thread");
	syslog(LOG_CRIT,"Could not create mqtt thread");
	exit(EXIT_FAILURE);
  }

  pause();

//printf("FIN %d\n",shutdown_flag);
  close(cmds_sockfd);
  close(packet_sockfd);
  stop_clients();
  for(int i=0;i<scale_cnt;++i)
	pthread_join(scale_thread[i],NULL);
//  close(control_sockfd);
  syslog(LOG_INFO,"%s daemon exiting",DAEMON_NAME);
  return 0;
}

