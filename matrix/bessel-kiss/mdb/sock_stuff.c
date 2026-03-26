#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// close

char *ip_address=NULL;

int init_socket(int portno)
{
int soc_fd;
struct sockaddr_in sin;
int stat, opt=1;
  soc_fd=socket(AF_INET,SOCK_STREAM,0);
  memset((void*)&sin,0,sizeof(sin));	// reset sin structure
  sin.sin_family=AF_INET;				// address family
  if( ip_address )
	sin.sin_addr.s_addr=inet_addr(ip_address);
  else
	sin.sin_addr.s_addr=htonl(INADDR_ANY);	// bind to any present IP address
  sin.sin_port=htons(portno);				// Port Number to bind to
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
  return soc_fd;
}

