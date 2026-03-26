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

#define UNUSED(A) (void)(A)

//#define mqtt_host "test.mosquitto.org";
#define mqtt_host "localhost"
#define mqtt_port 1883

extern volatile int shutdown_flag;
extern char publish_var[];

static struct mosquitto *mosq;

void to_mqtt(char *val)
{
int rc;
  if( (rc=mosquitto_publish(mosq,NULL,publish_var,
		strlen(val),val,2,0)!=MOSQ_ERR_SUCCESS) ){
#ifdef VERBOSE
	fprintf(stderr,"Error publishing: %s\n",mosquitto_strerror(rc));
#endif
  }
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
}

void *mqtt_thr(void *context)
{
int rc=0;
char clientid[]="barcode-scan";

  UNUSED(context);
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
	  sleep(2);
	  mosquitto_reconnect(mosq);
	}
  }
  mosquitto_destroy(mosq);

  mosquitto_lib_cleanup();
  return NULL;
}

