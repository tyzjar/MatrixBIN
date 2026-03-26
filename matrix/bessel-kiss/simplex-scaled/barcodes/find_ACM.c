#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int find_device(char *name,char *serdev)
{
char devdir[]="/dev/serial/by-id/";
char namebuf[256];
char buffer[256];
struct stat ss;
ssize_t len;
char *p;

  strcpy(namebuf,devdir);
  strcat(namebuf,name);
//  printf("%s\n",namebuf);
  if( lstat(namebuf,&ss)==-1 ){
	perror("lstat");
	return -1;
  }
  if( !S_ISLNK(ss.st_mode) ){
	printf("Failed to find path to device\n");
	return -1;
  }

  len=readlink(namebuf,buffer,sizeof(buffer)-1);
  if( len==-1 ){
	perror("readlink");
	return -1;
  }
  buffer[len] = '\0';
  p=strrchr(buffer,'/');
  strcpy(serdev,"/dev/");
  strcat(serdev,p);
  return 0;
}

#ifdef LOCAL_TEST
char name[]="usb-Tomato_Scanner_0123456789ABCDEF-if00";

int main (void)
{
char serdevname[12];

  if( find_device(name,serdevname)<0 ){
	printf("Serial device for %s not found\n",name);
	return -1;
  }
  printf("Serial device for %s is %s\n",name,serdevname);

  return 0;
}
#endif
