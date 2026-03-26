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
#include "cJSON.h"
#include "mdb.h"

#define UNUSED(A) (void)(A)

//#define mqtt_host "test.mosquitto.org";
#define mqtt_host "localhost"
#define mqtt_port 1883

extern int verb;
extern unsigned short rt_db[];
extern volatile int shutdown_flag;

static char ibuf[512];
//static char obuf[512];
#define MAPSIZE 4 
REG_MAP rmap[MAPSIZE]={{0x100,"r0"},{0x101,"r1"},{0x102,"r2"},{0x103,"r3"}};
unsigned short rt_db[HOLD_REGS];

static struct mosquitto *mosq;

char *regname(int reg)
{
char *str=NULL;
int i;
  UNUSED(reg);
  for(i=0;i<MAPSIZE;i++){
	if( reg==rmap[i].r ){
	  str=rmap[i].name;
	  break;
	}
  }
  return str;
}

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

int to_mqtt(int reg,unsigned char v)
{
cJSON *jresp;
char *json_resp;
char val[33];
char *key;
  if( (key=regname(reg))==NULL ){
	fprintf(stderr,"Error: register %x not found\n",reg);
	return -1;
  }
  jresp=cJSON_CreateObject();
  sprintf(val,"%d",v);
  if( !cJSON_AddStringToObject(jresp,key,val) ){
	fprintf(stderr,"Error: cJSON_AddStringToObject(response)\n");
	cJSON_Delete(jresp);
	return -2;
  }
  json_resp=cJSON_Print(jresp);
  mosquitto_publish(mosq,NULL,"modbus/set_register",
			strlen(json_resp),json_resp,0,0);
  free(json_resp);
  cJSON_Delete(jresp);
  return 0;
}

/* Callback called when the client receives
   a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
  UNUSED(obj);	// user data provided in mosquitto_new
#ifdef VERBOSE
  printf("on_connect: %s\n",mosquitto_connack_string(reason_code));
#endif
  if( reason_code!=0 ){
	mosquitto_disconnect(mosq);
	return;
  }
  mosquitto_subscribe(mosq,NULL,"matrix/wgt",0); // qos=0
}

void on_message(struct mosquitto *mosq, void *obj,
				const struct mosquitto_message *msg)
{
cJSON *jmsg;
cJSON *jwgt;
  UNUSED(mosq);
  UNUSED(obj);

  if( strcmp(msg->topic,"matrix/wgt") ){
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
  JSON_ITEM("SumWgtR",jwgt);
  rt_db[0]=atoi(jwgt->valuestring);
  if( verb )
	printf("on_msg: wgt=%d\n",rt_db[0]);
}

void *mqtt_thr(void *context)
{
int rc=0;
char clientid[]="modbus";

  UNUSED(context);
  mosquitto_lib_init();
  mosq=mosquitto_new(clientid,true,NULL);
  if( mosq==NULL ){
#ifdef VERBOSE
	fprintf(stderr, "Error: Out of memory.\n");
#endif
	return NULL;
  }

  rt_db[0]=678;
// Configure callbacks. This should be done before connecting ideally.
  mosquitto_connect_callback_set(mosq,on_connect);
  mosquitto_message_callback_set(mosq,on_message);
//  mosquitto_publish_callback_set(mosq,on_publish);

  rc=mosquitto_connect(mosq,mqtt_host,mqtt_port,60);
  if( rc!=MOSQ_ERR_SUCCESS ){
	mosquitto_destroy(mosq);
#ifdef VERBOSE
	fprintf(stderr,"Error: %s\n",mosquitto_strerror(rc));
#endif
	return NULL;
  }

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

