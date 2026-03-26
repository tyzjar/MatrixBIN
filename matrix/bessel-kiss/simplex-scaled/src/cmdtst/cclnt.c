//gcc -o cclnt cclnt.c
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 5001
#define SRVADDR "0.0.0.0"

char *cmds[3]={"SZ","ST","RT"};

char cmdbuf[16]={ 0x02,'S','C',';','0','1',';','S','T',0x0d,0x0a, 0 };
char repbuf[16]={0};

int main()
{
int sock, nBytes;//, portNum;
struct sockaddr_in serverAddr;
socklen_t addr_size;
char *srvrAddr=SRVADDR;
int scale_id=1;

  cmdbuf[5]='0'+scale_id;
  printf("scale_id=%c\n",cmdbuf[5]);

  //Create UDP socket
  sock = socket(PF_INET, SOCK_DGRAM, 0);

  //Configure settings in address struct
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = srvrAddr ? inet_addr(srvrAddr) : INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  //Initialize size variable to be used later on
  addr_size = sizeof serverAddr;

  while(1){
	int cmd;
	int num;
	while(1){
	  printf("Choose a command: type 1/2/3 for resp. SZ/ST/RT\n");
	  cmd=getchar();
//	  if( cmd==10 ) break;
//printf("%x\n",cmd);
	  switch(cmd){
		case '1': num=0; break;
		case '2': num=1; break;
		case '3': num=2; break;
		default:
		  num=-1;
		  if( cmd!=10 ) printf("Invalid command number %d\n",cmd);
		  break;
	  }
	  if( num!=-1 ){
		printf("Command: %c:%s\n",cmd,cmds[num]);
		break;
	  }
	}
	// clean stdin
	cmd=getchar();

	cmdbuf[7]=cmds[num][0];
	cmdbuf[8]=cmds[num][1];
	nBytes = strlen(cmdbuf);

	//Send message to server
	sendto(sock,cmdbuf,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);

	//Receive message from server
	nBytes = recvfrom(sock,repbuf,sizeof(repbuf),0,NULL, NULL);
	repbuf[9]=0;
	printf("Received from server: %s\n",&repbuf[1]);
  }
  return 0;
}

