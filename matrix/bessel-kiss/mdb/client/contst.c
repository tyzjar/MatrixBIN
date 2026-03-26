//gcc -o contst contst.c

#undef DEF_DEBUG /* enable trace to display TCPIP data */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define LINGER
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  //htonl()
#include <unistd.h>     //close()
#include <errno.h>
#include <ctype.h> // isprint

#include<signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#define ETH_PCT		1024

char buf[ETH_PCT];
int buf_size=ETH_PCT;
char resp[256]={0x01,0x02,0x00,0x00,0x00,0x05,
				0x01,0x04,0x02,0x00,0x0a};

typedef unsigned char	uchar;


// Imported variables

// Local variables
int  _port=502;
char _my_ip[24]="192.168.0.99";

int sig_pipe=0;
static int logfile=0;

static char prt_buf[ETH_PCT];

#ifdef LINGER
static void linger_opt(int soc_fd)
{
struct linger ling;
  ling.l_onoff=1;
  ling.l_linger=0;
  setsockopt(soc_fd,SOL_SOCKET,SO_LINGER,(char*)&ling,sizeof(ling));
}
#endif

static int sock_bind(int port,int soc_fd)
{
struct sockaddr_in sin;
int stat, opt=1;

  memset((void*)&sin,0,sizeof(sin));	// reset sin structure
  sin.sin_family=AF_INET;				// address family
  sin.sin_addr.s_addr=htonl(INADDR_ANY);// bind to any present IP address
  sin.sin_port=htons(port);				// Port Number to bind to
#ifdef DEF_DEBUG
printf("BIND to ADDR:%lx\n",sin.sin_addr.s_addr);
printf("PORT        :%d\n",sin.sin_port);
#endif

  // necessary, otherwise bind generates EADDRINUSE=248 error
  // when called repeatedly
  stat=setsockopt(soc_fd,SOL_SOCKET,SO_REUSEADDR,(int*)&opt,sizeof(opt));
  if( stat>=0 )
	stat=bind(soc_fd,(struct sockaddr *)&sin,sizeof(sin));
  if( stat<0 ){
	close(soc_fd);
#ifdef DEF_DEBUG
	printf("err=%d setting option for socket %d\n",errno,soc_fd);
#endif
	exit(-1);
  }
  return 0;
}

void sighand(int signo)
{
#ifdef DEF_DEBUG
  printf("signal=%d\n",signo);
#endif
  // skip sigpipe
  if( signo==SIGPIPE ) sig_pipe=1;
  else exit(0);
}

void prt_msg(int n, char *b)
{
int i;
  for(i=0;i<n;i++)
	printf("%2.2x ",b[i]&0xff);
  printf("\n");
}

void PrintUsage(int argc)
{
  if( argc>=1 ){
	printf(	"-a<str> this server ip address\n" \
			"-p<num> this server ip port\n");
  }
}

int main(int argc, char *argv[])
{
int mssoc_fd=-1; // Master Server Socket
int cssoc_fd=-1; // socket to get/set protocol parameters
int fd;
socklen_t size;
struct sockaddr_in from;
int c=-1, len;

struct timespec tv;
fd_set rfd;
int ready;

  while( (c=getopt(argc,argv,"a:p:"))!=-1 ){
	switch(c){
	  case 'a':
		strncpy(_my_ip,optarg,20);
		printf("Using ip address: %s\n",_my_ip);
		break;
	  case 'p':
		_port=atoi(optarg);
		printf("Using ip port: %d\n",_port);
		break;
	  default:
		break;
	}
  }//while getopt

  if( _port ){
	if( (mssoc_fd=socket(AF_INET,SOCK_STREAM,0))<0 ) return -1;
  }
  else return -1;
  sock_bind(_port,mssoc_fd);
#ifdef LINGER
  linger_opt(mssoc_fd);
#endif

  signal(SIGKILL,sighand);
  signal(SIGINT,sighand);
  signal(SIGTERM,sighand);
  signal(SIGSTOP,sighand);
  signal(SIGPIPE,sighand);

  for(;;){
	listen(mssoc_fd,4);
	fd=accept(mssoc_fd,(struct sockaddr *)&from,&size);
	if( fd==-1 ){
	  printf("accept:err=%d\n",errno);
	  continue;
	}
	if( cssoc_fd!=-1 )
	  close(cssoc_fd);
	cssoc_fd=fd;

	for(;;){
	  /* Watch stdin (fd 0) to see when it has input. */
	  FD_ZERO(&rfd);
	  FD_SET(cssoc_fd,&rfd);

	  /* Wait up to five seconds. */
	  tv.tv_sec=1;
	  tv.tv_nsec = 0;

	  ready=pselect(cssoc_fd+1,&rfd,NULL,NULL,&tv,0);
	  /* Don't rely on the value of tv now! */
	  if( ready==-1 )
		perror("select()");
	  else if(ready){
		if( FD_ISSET(cssoc_fd,&rfd) ){
//		  printf("Data is available now.\n");
		  ioctl(cssoc_fd,FIONREAD,&len);
//		  printf("Ready %d\n",len);
		  if( len==0 ){
			close(cssoc_fd);
			cssoc_fd=-1;
			break;
		  }
		  len=read(cssoc_fd,buf,buf_size);
		  if( len ) prt_msg(len,buf);
		  write(cssoc_fd,resp,11);
		}
	  }
	  else{
	    printf("Timeout.\n");
	  }
	}// process modbus sonnection
  }// listen cycle
  return 0;
}
