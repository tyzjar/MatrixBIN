#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>		// close

int init_socket(const char* ipaddr, int portno)
{
struct sockaddr_in addr;
int on;
  // First call to socket() function
  int sockfd=socket(PF_INET,SOCK_STREAM,0);
  if( sockfd==-1 ){
	switch(errno){
	  case EPROTONOSUPPORT:
		break;
	  case EMFILE:
		break;
	  case ENFILE:
		break;
	  case EACCES:
		break;
	  case ENOBUFS:
		break;
	  case EINVAL:
		break;
	  default:
		break;
	}
	perror("TCP socket open failure.\n");
	exit(EXIT_FAILURE);
  }
  on=1;
  if( setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0 )
	perror("setsockopt");

  // Initialize socket structure
  bzero((char *)&addr,sizeof(addr));
  addr.sin_family = PF_INET;
  if( ipaddr==0 )
	addr.sin_addr.s_addr = INADDR_ANY;
  else
	addr.sin_addr.s_addr = inet_addr(ipaddr);
  addr.sin_port = htons(portno);

  // Now bind the host address using bind() call.
  if( bind(sockfd,(struct sockaddr *) &addr, sizeof(addr))<0 ){
	perror("TCP bind failed");
	exit(EXIT_FAILURE);
  }
  return sockfd;
}


int udp_socket(char *ipaddr,int port)
{
int sockfd;
struct sockaddr_in addr;
  // Creating socket file descriptor
  if ( (sockfd=socket(PF_INET,SOCK_DGRAM,0))<0 ){
	perror("UDP socket creation failed");
	exit(EXIT_FAILURE);
  } 
  memset(&addr,'\0',sizeof(addr));

  // Filling server information
  addr.sin_family=AF_INET; // IPv4
  if( ipaddr )
	addr.sin_addr.s_addr=inet_addr(ipaddr);
  else
	addr.sin_addr.s_addr=INADDR_ANY;
  addr.sin_port=htons(port);
  memset(addr.sin_zero,'\0',sizeof addr.sin_zero);  

  // Bind the socket with the server address 
  if( bind(sockfd,(const struct sockaddr *)&addr,sizeof(addr))<0 ){
	perror("UDP bind failed");
	exit(EXIT_FAILURE);
  }
  return sockfd;
}

