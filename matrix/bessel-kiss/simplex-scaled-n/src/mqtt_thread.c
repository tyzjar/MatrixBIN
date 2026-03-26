#include <stdio.h>
#include <stdlib.h> // atoi
#include <string.h>
#include <time.h>
//#include <errno.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/select.h>
//#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <mosquitto.h>
#include "scaled.h"
#include "cJSON.h"
#include "commands.h"
#include "json_stuff.h"

#define UNUSED(A) (void)(A)

//#define mqtt_host "test.mosquitto.org";
#define mqtt_host "localhost"
#define mqtt_port 1883

//extern volatile sig_atomic_t shutdown_flag;
extern volatile int shutdown_flag;
extern int scale_cnt;
extern scale_sys_t *scale[MAX_SCALES];
extern char *json_conf;

int started_cnt=0;
static scale_sys_t *cur_scale=NULL;
static char ibuf[512];
static char obuf[512];
//<MJK 08.07.2025
static cmd_pending_t cur_cmd;
//>

int find_scale(int id)
{
int i;
  for(i=0;i<scale_cnt;i++)
	if( scale[i]->id==id ){
	  cur_scale=scale[i];
	  return id;
	}
  return -1;
}

int exec_ldu_cmd(char *cmd,int stop_flow)
{
int len, total=0;
char *p=obuf;

struct timespec tv;
fd_set rfd;
int ready, nfds, bytes;

  if( stop_flow ){
	cur_cmd.cmdbuf=obuf;
	cur_cmd.rc=-2;
	clock_gettime(CLOCK_REALTIME,&tv);
	tv.tv_sec+=5; // set timeout =2 seconds
	write(cur_scale->fd,"ID\r",3);
	len=write(cur_scale->fd,cmd,strlen(cmd));
	cur_scale->cmd=&cur_cmd;
//printf("%d %s\n",len,cmd);
	sem_timedwait(&cur_scale->sem,&tv);
	if( !(cur_cmd.rc==0 && obuf[0]=='D' && obuf[1]==':') )
	  return -1;
	cur_scale->calibr_mode=1;
  }

  len=write(cur_scale->fd,cmd,strlen(cmd));

// timeout settings
  tv.tv_sec=1;
  tv.tv_nsec=0;	// nanoseconds
  FD_ZERO(&rfd);
  nfds=cur_scale->fd;
  nfds++;

  memset(obuf,'\0',sizeof(obuf));
  for(;;){
	FD_SET(cur_scale->fd,&rfd);
	ready=pselect(nfds,&rfd,0,0,&tv,0);
//printf("%d\n",ready);
	if( ready==-1 ){
	  perror("pselect()");
	}
	else if( ready==0 ){
	  // timeout
//printf("Select timeout\n");
	  return total;
	}
	else if( ready>0 ){
	  if( FD_ISSET(cur_scale->fd,&rfd) ){
		ioctl(cur_scale->fd,FIONREAD,&bytes);
	printf("read %d\n",bytes);
		len=read(cur_scale->fd,p,bytes);
		total+=len;
		p+=len;
		if( *(p-1)=='\r' || len==0 ) break;
	  }
	}// if ready to read
  }
printf("%s\n",obuf);
  return total;
}

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
int rc;
  UNUSED(obj);	// user data provided in mosquitto_new
#ifdef VERBOSE
  printf("on_connect: %s\n",mosquitto_connack_string(reason_code));
#endif
  if( reason_code!=0 ){
	mosquitto_disconnect(mosq);
	return;
  }
// retain must be 1!
  rc=mosquitto_publish(mosq,NULL,"calibration/configuration",
						strlen(json_conf),json_conf,0,1);
  if( rc!=MOSQ_ERR_SUCCESS )
	fprintf(stderr,"Error publishing: %s\n",mosquitto_strerror(rc));
  mosquitto_subscribe(mosq,NULL,"calibration/cmd",0); // qos=0
}

//#define VERBOSE

#ifdef VERBOSE
#define _ERR(str) fprintf(stderr,(str))
#define _ERR1(fmt,par) fprintf(stderr,(fmt),(par))
#else
#define _ERR(str)
#define _ERR1(fmt,par)
#endif


#define JSON_ITEM(k,j) \
  (j)=cJSON_GetObjectItem(jmsg,(k)); \
  if( (j)==NULL ){ _ERR1("Error: Payload missing data %s.\n",(k)); return; }

#define JSON_ADDSTR(k,v) \
  if( !cJSON_AddStringToObject(jresp,(k),(v)) ) \
	{ _ERR("Error: cJSON_AddStringToObject(response)\n"); return; }


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
//scale_sys_t *s=(scale_sys_t *)obj;
int id=-1;
int switch_mode=0;
int len, rc=0;
cJSON *jmsg, *jresp;
cJSON *jcmd, *jchan;
char *json_resp;

  UNUSED(mosq);
  UNUSED(obj);
//  printf("%d\n",s->id);

  if( strcmp(msg->topic,"calibration/cmd") ){
	_ERR("Invalid msg received\n");
	return;
  }
  memcpy(ibuf,msg->payload,msg->payloadlen);
  ibuf[msg->payloadlen]=0;
#if CJSON_VERSION_FULL < 1007013
  jmsg=cJSON_Parse(ibuf);
#else
  jmsg=cJSON_ParseWithLength(ibuf,(size_t)msg->payloadlen);
#endif

  if( jmsg==NULL ){
	_ERR("Payload not JSON.\n");
	return;
  }

  JSON_ITEM("chan",jchan);
  JSON_ITEM("cmd",jcmd);
  id=atoi(jchan->valuestring);
#ifdef VERBOSE
  fprintf(stderr,"id=%d cmd=%s\n",id,jcmd->valuestring);
#endif

  if( cur_scale ){
	if( cur_scale->id==id ) switch_mode=0; // continue transparent mode
	else{
	// finish with this scale whether id=-1 or not
	  cur_scale->calibr_mode=0;
	  cur_scale=NULL;
	  if( id!=-1 ){
		if( find_scale(id)==-1 ){ // invalid channel(=id)
		  _ERR1("Error: invalid channel %s.\n",jchan->valuestring);
		  return;
		}
		else switch_mode=1;  // switch to scale with scale->id=id
	  }
	// just finish with current scale when new id=-1
	}
  }
  else{ // scale is not selected yet
	if( id==-1 ) return;
	if( find_scale(id)==-1 ){ // invalid channel(=id)
	  _ERR1("Error: invalid channel %s.\n",jchan->valuestring);
	  return;
	}
	else switch_mode=1;
  }
  if( cur_scale ){
#ifdef VERBOSE
	fprintf(stderr,"cmd=%s\n stop flow=%d\n",jcmd->valuestring,switch_mode);
#endif
	len=exec_ldu_cmd(jcmd->valuestring,switch_mode);
	if( len<=0 ){ // error ?
	  rc=1;
	}
	// replace '\r' by end of string
	obuf[len-1]=0;
  }
  else return;
  jresp=cJSON_CreateObject();
  JSON_ADDSTR("chan",jchan->valuestring);
  JSON_ADDSTR("cmd",jcmd->valuestring);
  if( rc>0 ){
	JSON_ADDSTR("response","NO");
	JSON_ADDSTR("ret_code","COMM_ERR");
  }
  else{
	JSON_ADDSTR("response",obuf);
	JSON_ADDSTR("ret_code","OK");
  }
  json_resp= cJSON_Print(jresp);
  rc=mosquitto_publish(mosq,NULL,"calibration/cmd_response",
			strlen(json_resp),json_resp,0,0);
  free(json_resp);
  cJSON_Delete(jmsg);
  cJSON_Delete(jresp);
}

void *mqtt_thr(void *context)
{
int rc=0;
struct mosquitto *mosq;
char clientid[]="scaled";

  UNUSED(context);
//  if( json_conf==NULL ) return NULL;
  mosquitto_lib_init();
  mosq=mosquitto_new(clientid,true,NULL);
  if( mosq==NULL ){
#ifdef VERBOSE
	fprintf(stderr, "Error: Out of memory.\n");
#endif
	return NULL;
  }

// Configure callbacks. This should be done before connecting ideally.
  mosquitto_connect_callback_set(mosq,on_connect);
  mosquitto_message_callback_set(mosq,on_message);
//  mosquitto_publish_callback_set(mosq,on_publish);

//printf("s=%d t=%d\n",started_cnt,scale_cnt);
  while( started_cnt<scale_cnt )
	usleep(1000);
//printf("s=%d t=%d\n",started_cnt,scale_cnt);
  jarray_convert();

  rc=mosquitto_connect(mosq,mqtt_host,mqtt_port,60);
  if( rc!=MOSQ_ERR_SUCCESS ){
	mosquitto_destroy(mosq);
#ifdef VERBOSE
	fprintf(stderr,"Error: %s\n",mosquitto_strerror(rc));
#endif
	return NULL;
  }
#ifdef VERBOSE
  fprintf(stderr,"%s\n",json_conf);
#endif

// Run the network loop in a background thread, this call returns quickly.
  while( shutdown_flag!=1 ){
	rc=mosquitto_loop(mosq,-1,1); // -1\equiv timeout=1000msec
	if( (shutdown_flag!=1) && rc ){ // MOSQ_ERR_SUCCESS=0
	  sleep(20);
	  mosquitto_reconnect(mosq);
	}
  }
  mosquitto_destroy(mosq);

  mosquitto_lib_cleanup();
  return NULL;
}

