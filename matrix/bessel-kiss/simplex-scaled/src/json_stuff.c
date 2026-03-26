#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <confuse.h>
#include <cJSON.h>

char *json_conf=NULL;

static cJSON *jarray=NULL;
static cJSON **jobj=NULL;
static int json_fail=0;

void clean_json()
{
  if( jarray ){
	cJSON_Delete(jarray);
	jarray=NULL;
  }
  if( jobj ){
	free(jobj);
	jobj=NULL;
  }
  if( json_conf ){
	free(json_conf);
	json_conf=NULL;
  }
}

void block_json()
{
  json_fail=1;
}


void init_jarray(int cnt)
{
  jobj=(cJSON **)malloc(sizeof(cJSON *)*cnt);
  if( jobj==NULL ){
	json_fail=1;
	return;
  }
  jarray=cJSON_CreateArray();
  if( jarray==NULL ){
	fprintf(stderr,"Error: cJSON_CreateArray()\n");
	json_fail=1;
  }
}

void init_jobj(int num)
{
  if( json_fail ) return;
  jobj[num]=cJSON_CreateObject();
  if( jobj[num]==NULL ){
	fprintf(stderr,"Error: cJSON_CreateObject(%d)\n",num);
	json_fail=1;
	clean_json();
  }
}

void add_jitem(int num)
{
  if( json_fail ) return;
  if( !cJSON_AddItemToArray(jarray,jobj[num]) ){
	fprintf(stderr,"Error: AddItemToArray(%d)\n",num);
	json_fail=1;
	clean_json();
  }
}

void jobj_add_str(int num,char *key_str,char *val_str)
{
  if( json_fail ) return;
  if( !cJSON_AddStringToObject(jobj[num],key_str,val_str) ){
	fprintf(stderr,"Error: cJSON_AddStringToObject(%s,%s)\n",key_str,val_str);
	json_fail=1;
	clean_json();
  }
}

void jobj_add_int(int num,char *key_str,int val)
{
char buf[8];
  if( json_fail ) return;
  sprintf(buf,"%d",val);
  if( !cJSON_AddStringToObject(jobj[num],key_str,buf) ){
	fprintf(stderr,"Error: cJSON_AddStringToObject(%s,%s)\n",key_str,buf);
	json_fail=1;
	clean_json();
  }
}

void jarray_convert()
{
  if( json_fail ) return;
  json_conf=cJSON_Print(jarray);
  cJSON_Delete(jarray);
  jarray=NULL;
}


