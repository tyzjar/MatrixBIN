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
char buf[MAX_MSG_SIZE]={0x01,0x02,0x00,0x00,0x00,0x06,
						0x01,0x04,0x00,0x00,0x00,0x78};

void prt_msg(unsigned short *);

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
  }
  tmv.tv_sec=3;
  tmv.tv_usec=0;
//	setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,&tmv,sizeof(struct timeval));
  setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tmv,sizeof(struct timeval));
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

  /* Connect to server */
  errno=0;
  while( connect(sock,(struct sockaddr *)&server,sizeof(server))<0 )
  {
	if( errno == EISCONN ) return sock; //Socket is already connected
	printf("Connect failed %d\n",errno);
	perror("Connect");
	sleep(5);
  }
  return sock;
}

void pipe_handler(int signo)
{
  printf("sig=%d\n",signo);
  go=0;
}


/////////////////////////////

int main()
{
int len;
char data[1024];
char b[250];
int i, j;
char *p, *q;

//  signal(SIGPIPE,pipe_handler);
  signal(SIGINT,pipe_handler);
//  signal(SIGKILL,pipe_handler);
//  signal(SIGTERM,pipe_handler);
//  signal(SIGINT,pipe_handler);

  sock=-1;

//  ConnectToServer("192.168.0.99",502);
  ConnectToServer("192.168.0.218",502);
  if( sock<0 )
	return -1;

  for(i=0;i<10;i++){
printf("\nrequest #%d\n",i+1);
	len=write(sock,buf,12);
//	  printf("Data is available now %d bytes.\n",len);
//  sleep(1);
	len=read(sock,&data,1024);
	if( len>0 ){
//	printf("%d\n",len);
//	prt_buf(data,len);
	  len=(unsigned char)data[8]/2;
//printf("%d\n",len);
	  p=&data[9];
	  q=b;
	  for(j=0;j<len-1;j++){
		q[1]=p[0]; q[0]=p[1];
		p+=2; q+=2;
	  }
	  prt_msg((unsigned short *)b);
	}
	else
	  printf("read returned %d\n",len);
	sleep(1);
  }
  close(sock);
  return 0;
}
