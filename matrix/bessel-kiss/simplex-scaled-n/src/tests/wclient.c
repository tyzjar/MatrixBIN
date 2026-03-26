#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		// sleep(), close()
#include <signal.h>		// SIGKILL
#include <errno.h>

#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>		// inet_addr(), htons()

#include <sys/ioctl.h>

#define MAX_MSG_SIZE	1024
char buf[MAX_MSG_SIZE];
int sock=-1;
int go=1;

unsigned ws_id;

void prt_buf(char *b,int n)
{
int i;
  for(i=0;i<n;i++)
	printf("%02x ",b[i]&0xff);
  printf("\n");
}


int ConnectToServer(char *server_addr,int port)
{
struct sockaddr_in server;
struct hostent *hp=NULL, *gethostbyname();
struct timeval	tmv;
int on=1;

  if( sock==-1 ){
	memset((char*)&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(port);

	if( server_addr[0]>='0' && server_addr[0]<='9' ) // server address is given
	  server.sin_addr.s_addr=inet_addr(server_addr);
	else{// server is identified by name
	  hp=gethostbyname(server_addr);
	  if( hp==0) return 0;
//	  memcpy(&server.sin_addr,hp->h_addr,hp->h_length);
	}

	if( (sock=socket(AF_INET,SOCK_STREAM,0))< 0){
	  printf("Failed to open socket\n");
	  exit(EXIT_FAILURE);
	}

	tmv.tv_sec=5;
	tmv.tv_usec=0;
	setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&tmv,sizeof(struct timeval));
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tmv,sizeof(struct timeval));
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  }

  /* Connect to server */
  errno=0;
  while( connect(sock,(struct sockaddr *)&server,sizeof(server))<0 )
  {
	if( errno == EISCONN ) return sock; //Socket is already connected
	printf("Connect failed %d\n",errno);
	sleep(1);
  }
  return sock;
}

void pipe_handler(int signo)
{
  printf("sig=%d\n",signo);
  go=0;
}

struct weight_struct {
    struct timeval ts;
    unsigned char channel;
    float weight;
    float tare;
    unsigned int status;
};


void unpack_weight(struct weight_struct* weight, const char *data)
{
  // "%cW ;%u;%u.%u;%g;%g;%u"
  // first char should be STX?
char *start = 0;
  start = strstr(data, ";");

  weight->ts.tv_sec = strtoul (++start, &start, 10);
  weight->ts.tv_usec = strtoul (++start, &start, 10);

  weight->channel = strtoul (++start, &start, 10);

  weight->weight = strtod(++start, &start);
  weight->tare = strtod(++start, &start);

  weight->status = strtoul(++start, &start, 10);
}


/////////////////////////////

int main()
{
//struct timeval tv;
fd_set rfds;
int retval, len;
char data[1024];
int i;

//  signal(SIGPIPE,pipe_handler);
  signal(SIGINT,pipe_handler);
//  signal(SIGKILL,pipe_handler);
//  signal(SIGTERM,pipe_handler);
//  signal(SIGINT,pipe_handler);

  sock=-1;

  ConnectToServer("0.0.0.0",5000);
//  ConnectToServer("192.168.0.049",5000);
  if( sock<0 )
	return -1;

  FD_ZERO(&rfds);
  FD_SET(sock,&rfds);

  for(i=1;go;i++){
	// Wait up to five seconds.
//	tv.tv_sec = 5;
//	tv.tv_usec = 0;
//	retval = select(sock+1,&rfds,NULL,NULL,&tv);
	// Blocking select
	retval = select(sock+1,&rfds,NULL,NULL,NULL);
	if( retval==-1 && errno==EINTR )
	  continue;
	if( retval==-1 ){
	  perror("select()");
	  return -1;
    }
	else{
	  ioctl(sock,FIONREAD,&len);
	  if( len==0 ){
		sleep(2);
		continue;
	  }
//	  printf("Data is available now %d bytes.\n",len);
	  len=read(sock,&data,len);
	  if( len>0 ){
		data[len-1]=0;
		printf("%d : %s\n",i,data);
	  }
	  else
		printf("read returned %d\n",len);
	}
  }
  close(sock);
  return 0;
}
