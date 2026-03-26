// gcc -o bc_scan bc_scan.c

//#undef DEF_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>     //close()
#include <errno.h>
#include <ctype.h> // isprint

#include <signal.h>
#include <sys/select.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


char scan_name[300]="usb-Tomato_Scanner_0123456789ABCDEF-if00";
char publish_var[256]="matrix/raw-barcode";
int shutdown_flag;

pthread_t mqtt_thread;

extern int find_device(char *,char *);
extern int config_serial(char *);

extern void *mqtt_thr(void *);
extern void to_mqtt(char *);
/*
void to_mqtt(char *str)
{
  (void)str;
}
void *mqtt_thr(void *context)
{
  (void)context;
  return NULL;
}
*/
void sighand(int signo)
{
//#ifdef DEF_DEBUG
  shutdown_flag=1;
  printf("signal=%d\n",signo);
//#endif
  exit(0);
}

void mk_prt_str(int n, char *in, char *out)
{
int i, j;
  for(i=0,j=0;i<n;i++,j++){
	if( isprint(in[i]) ){
	  out[j]=in[i];
	}
	else{
	  out[j]='['; j++;
	  sprintf(&out[j],"%2.2x",in[i]); j+=2;
	  out[j]=']';
	}
  }
  out[j]=0;
}

void mk_prt_str1(int n, char *in, char *out)
{
int i, j;
  for(i=0,j=0;i<n;i++){
	if( isprint(in[i]) ){
	  out[j]=in[i];
	  j++;
	}
  }
  out[j]=0;
}

char buf[1024];
char prt_buf[2048];
char serdevname[16];

int scan_loop(int fd)
{
fd_set rfd;
int ready, nfds;
int len;
char *p;
  FD_ZERO(&rfd);
  nfds=fd;
  FD_SET(fd,&rfd);
  nfds++;
  p = buf;	
  for(;shutdown_flag==0;){
	ready=pselect(nfds,&rfd,0,0,NULL,0);
	if( ready==-1 ){
	  perror("pselect()");
	}
	else{
	  if( FD_ISSET(fd,&rfd) ){
		len=read(fd,p,1024);
		  mk_prt_str(len,p,prt_buf);
		  printf("received:  %s\n",prt_buf);
		if( len<=0 ){
		  close(fd);
		  return -1;
		}
		if( p[len-1]==0x0a ){
		  p[len-1]=0;
		  printf("pubished:  %s\n",buf);
		  to_mqtt(buf);
		  p=buf;
		}
		else p+=len;
	  }
	}// if ready to read
  }
  return 0;
}

void PrintUsage()
{
  printf(	"-f <str> scanner name\n" \
			"\tdefault: usb-Tomato_Scanner_0123456789ABCDEF-if00\n" \
			"-p <str> string to publish\n" \
			"\tdefault: matrix/raw-barcode\n" \
			"-h       prints this message\n");
}


int main(int argc, char *argv[])
{
int dev_fd=-1;
int c=-1, rc;

  while( (c=getopt(argc,argv,"hf:p:"))!=-1 ){
	switch(c){
	  case 'f':
		strncpy(scan_name,optarg,255);
		printf("Search ACM device by string: %s\n",scan_name);
		break;
	  case 'p':
		strncpy(publish_var,optarg,255);
		printf("Publish value as variable: %s\n",publish_var);
		break;
	  case 'h':
		PrintUsage();
		exit(EXIT_SUCCESS);
		break;
	  default:
		PrintUsage();
		exit(EXIT_SUCCESS);
		break;
	}
  }//while getopt

  signal(SIGKILL,sighand);
  signal(SIGINT,sighand);
  signal(SIGTERM,sighand);
  signal(SIGSTOP,sighand);
  signal(SIGPIPE,sighand);

  shutdown_flag=0;
  if(pthread_create(&mqtt_thread,NULL,mqtt_thr,NULL)!=0 ){
	perror("Could not create mqtt thread");
	return -1;
  }

  while( shutdown_flag==0 ){
	sleep(3);
	if( find_device(scan_name,serdevname)<0 ){
	  printf("Serial device for %s not found\n",scan_name);
	  continue;
	}
	dev_fd=open(serdevname,O_RDONLY);
	if( dev_fd==-1 ){
	  printf("Failed to open %s\n",serdevname);
	  continue;
	}
	if( (rc=scan_loop(dev_fd)==-1) )
	  continue;
	else if( rc==0 )
	  break;
  }

  return 0;
}

//#include <fcntl.h>
//#include <linux/input.h>
//  ioctl(fd,EVIOCGRAB,1);
/*
  for(;;){
	len=read(fd,buf,1024);
	if( len>0 ){
	  mk_prt_str1(len,buf,prt_buf);
	  printf("received %d:  %s\n",len,prt_buf);
	}
	usleep(50000);
  }
*/

