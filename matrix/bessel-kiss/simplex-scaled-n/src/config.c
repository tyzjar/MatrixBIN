#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

#include <confuse.h>
#include <librs232/rs232.h>

#include "config.h"
#include "scaled.h"
//<Jun 12, 2025 MJK
#include "json_stuff.h"
//>

extern int scale_cnt;
extern scale_sys_t *scale[];
extern int models_cnt;
extern model_cmd_t *mod_cmd[];

/** @begin package parse variables */
extern int data_portno;
extern int cmds_portno;
extern char ip_address[];
extern int ip_size;

unsigned int packet_length = 0;

unsigned int weight_pos = 0;
unsigned int weight_len = 0;

char *status[4];
unsigned int status_pos = 0;
unsigned int status_len = 0;

char *gross[2];
unsigned int gross_pos = 0;
unsigned int gross_len = 0;

char *unit[2];
unsigned int unit_pos = 0;
unsigned int unit_len = 0;

/** @end package parse variables */
//<MJK
cfg_opt_t cmd_opts[] = {
	CFG_STR("set_zero", 0, CFGF_NODEFAULT),
	CFG_STR("set_tare", 0, CFGF_NODEFAULT),
	CFG_STR("reset_tare", 0, CFGF_NODEFAULT),
	CFG_END()
};

cfg_opt_t command_opts[] = {
	CFG_SEC("cmd",cmd_opts,CFGF_TITLE | CFGF_MULTI),
	CFG_END()
};

//>

cfg_opt_t device_opts[] = {
	CFG_INT("addr", 0, CFGF_NONE),
	CFG_STR("model", 0, CFGF_NODEFAULT),
	CFG_STR("datatype", 0, CFGF_NODEFAULT),
	CFG_END()
};

cfg_opt_t serial_interface_opts[] = {
	CFG_STR("dev", 0, CFGF_NODEFAULT),
	CFG_STR("baud", 0, CFGF_NODEFAULT),
	CFG_STR("data", 0, CFGF_NODEFAULT),
	CFG_STR("stop", 0, CFGF_NODEFAULT),
	CFG_STR("flow", 0, CFGF_NODEFAULT),
	CFG_STR("parity", 0, CFGF_NODEFAULT),
	CFG_INT("id", 0, CFGF_NODEFAULT),
	CFG_SEC("device", device_opts, CFGF_MULTI),
	CFG_END()
};

cfg_opt_t weight_opts[] = {
	CFG_INT("Position", 0, CFGF_NONE),
    CFG_INT("Length", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t status_opts[] = {
    CFG_INT("Position", 0, CFGF_NONE),
    CFG_INT("Length", 0, CFGF_NONE),
    CFG_STR("Stable", 0, CFGF_NONE),
    CFG_STR("Unstable", 0, CFGF_NONE),
    CFG_STR("OverLoad", 0, CFGF_NONE),
    CFG_STR("UnderLoad", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t gross_opts[] = {
    CFG_INT("Position", 0, CFGF_NONE),
    CFG_INT("Length", 0, CFGF_NONE),
    CFG_STR("Net", 0, CFGF_NONE),
    CFG_STR("Gross", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t unit_opts[] = {
    CFG_INT("Position", 0, CFGF_NONE),
    CFG_INT("Length", 0, CFGF_NONE),
    CFG_STR("Kg", 0, CFGF_NONE),
    CFG_STR("g", 0, CFGF_NONE),
    CFG_END()
};

cfg_opt_t opts[] = {
    CFG_INT("Listen_port", 5000, CFGF_NONE),
    CFG_INT("Command_port", 5001, CFGF_NONE),
    CFG_INT("DDTime", 0, CFGF_NONE),
    CFG_STR("ip","0.0.0.0", CFGF_NONE),
    CFG_SEC("serial", serial_interface_opts, CFGF_MULTI),
	CFG_SEC("command",command_opts, CFGF_NONE),
    CFG_INT("Length", 0, CFGF_NONE),
    CFG_SEC("Weight", weight_opts, CFGF_NONE),
    CFG_SEC("Status", status_opts, CFGF_NONE),
    CFG_SEC("Gross", gross_opts, CFGF_NONE),
    CFG_SEC("Unit", unit_opts, CFGF_NONE),
    CFG_END()
};


static const char *
rs232_baud[] = {
		"300",
		"2400",
		"4800",
		"9600",
		"19200",
		"38400",
		"57600",
		"115200",
		"460800",
};

enum rs232_baud_e rs232_to_baud(const char* baud)
{
	unsigned int i;
	for(i = 0; i < sizeof(rs232_baud); i++) {
		if(strncmp(rs232_baud[i], baud, strlen(rs232_baud[i])) == 0)
			break;
	}

	return (enum rs232_baud_e)i;
}

static const char *
rs232_data[] = {
		"5",
		"6",
		"7",
		"8",
};

enum rs232_data_e rs232_to_data(const char *data)
{
	unsigned int i;
	for(i = 0; i < sizeof(rs232_data); i++) {
		if(strncmp(rs232_data[i], data, strlen(rs232_data[i])) == 0)
			break;
	}

	return (enum rs232_data_e)i;
}


static const char *
rs232_parity[] = {
		"none",
		"odd",
		"even",
};

enum rs232_parity_e rs232_to_parity(const char* parity)
{
	unsigned int i;
	for(i = 0; i < sizeof(rs232_parity); i++) {
		if(strncmp(rs232_parity[i], parity, strlen(rs232_parity[i])) == 0)
			break;
	}

	return (enum rs232_parity_e)i;
}

static const char *
rs232_stop[] = {
		"1",
		"2",
};

enum rs232_stop_e rs232_to_stopbits(const char* stop)
{
	unsigned int i;
	for(i = 0; i < sizeof(rs232_stop); i++) {
		if(strncmp(rs232_stop[i], stop, strlen(rs232_stop[i])) == 0)
			break;
	}

	return (enum rs232_stop_e)i;
}

static const char *
rs232_flow[] = {
		"off",
		"hardware",
		"xon/xoff",
};

enum rs232_flow_e rs232_to_flow(const char* flow)
{
	unsigned int i;

	for(i = 0; i < sizeof(rs232_flow); i++) {
		if(strncmp(rs232_flow[i], flow, strlen(rs232_flow[i])) == 0)
			break;
	}

	return (enum rs232_flow_e)i;
}

static int returnError(int* errorPtr, int errorValue) {
	if(errorPtr != NULL) {
		*errorPtr = errorValue;
	}
	return CONFIG_FUNCTION_RETURN_ERR;
}

void ser_settings(struct rs232_port_t *ser)
{
  printf("%s settings: baud=%d data=%d stop=%d flow=%d parity=%d\n",
	ser->dev,ser->baud,ser->data,ser->stop,ser->flow,ser->parity);
}

struct rs232_port_t *config_serial(cfg_t *cfg_serial)
{
struct rs232_port_t *rs232;
  rs232=rs232_init();	// из библиотеки rs232_posix
  rs232_set_device(rs232,cfg_getstr(cfg_serial,"dev"));
//  strcpy(rs232->dev,cfg_getstr(cfg_serial,"dev"));
  if( access(rs232->dev,W_OK|R_OK)!=0 ){
	printf("loadConfig[%d]: Error accessing the given device: %s", __LINE__,rs232->dev);
	return NULL;
  }
  rs232->baud = rs232_to_baud(cfg_getstr(cfg_serial, "baud"));
  rs232->data = rs232_to_data(cfg_getstr(cfg_serial, "data"));
  rs232->stop = rs232_to_stopbits(cfg_getstr(cfg_serial, "stop"));
  rs232->flow = rs232_to_flow(cfg_getstr(cfg_serial, "flow"));
  rs232->parity = rs232_to_parity(cfg_getstr(cfg_serial, "parity"));
//  printf("loadConfig[%d]: Section \"serial\" : baud=%d data=%d stop=%d flow=%d parity=%d\n", __LINE__,
//	rs232->baud,rs232->data,rs232->stop,rs232->flow,rs232->parity);
  return rs232;
}

#define CONF_VAL(k) { \
  val_str=cfg_getstr(cfg_serial,(k)); jobj_add_str(num,(k),val_str); }

struct rs232_port_t *conf_serial(cfg_t *cfg_serial,int num)
{
struct rs232_port_t *rs232;
char *val_str;
  rs232=rs232_init();	// из библиотеки rs232_posix
  CONF_VAL("dev");
  rs232_set_device(rs232,val_str);
  if( access(rs232->dev,W_OK|R_OK)!=0 ){
	printf("loadConfig[%d]: Error accessing the given device: %s", __LINE__,rs232->dev);
	clean_json();
	return NULL;
  }
//  val_str=cfg_getstr(cfg_serial,"baud");
  CONF_VAL("baud");
  rs232->baud = rs232_to_baud(val_str);
  CONF_VAL("data");
  rs232->data = rs232_to_data(val_str);
  CONF_VAL("stop");
  rs232->stop = rs232_to_stopbits(val_str);
  CONF_VAL("flow");
  rs232->flow = rs232_to_flow(val_str);
  CONF_VAL("parity");
  rs232->parity = rs232_to_parity(val_str);
//  printf("loadConfig[%d]: Section \"serial\" : baud=%d data=%d stop=%d flow=%d parity=%d\n", __LINE__,
//	rs232->baud,rs232->data,rs232->stop,rs232->flow,rs232->parity);
  return rs232;
}

// should be called after config_serial
void parse_commands(cfg_t *cfg_command)
{
char *name[]={ "set_zero", "set_tare", "reset_tare" };
int i,j;
enum cmd_e k;
cfg_t *opt;
char model[12];
char *p;
int len;

  for(i=0;i<(int)cfg_size(cfg_command,"cmd");i++){
	opt=cfg_getnsec(cfg_command,"cmd",i);
	strcpy(model,cfg_title(opt));
	// is this model in use?
	for(j=0;j<models_cnt;j++)
	  if( strcmp(model,mod_cmd[j]->model)==0 ) break;
	// if the model is not used
	if( j==models_cnt ) continue;
	// build command set for this model
	for(k=set_zero;k<max_cmd;k++){
//printf("parse: %s\n",name[k]);
	  if( (p=cfg_getstr(opt,name[k]))==NULL ){
		mod_cmd[j]->cmds[k]=NULL;
		continue;
	  }
	  len=strlen(p);
	  mod_cmd[j]->cmds[k]=(char *)malloc((len+1)*sizeof(char));
	  strcpy(mod_cmd[j]->cmds[k],p);
	}
  }
}

int loadConfig(const char* configFile, int* iError)
{
int	ret;
cfg_t *cfg, *sec, *opt;
scale_sys_t *s=NULL;
model_cmd_t *m=NULL;
int i,j,k;
char *p;

  if( configFile==NULL)
	return returnError(iError,CONFIG_ERROR_FILE_NOT_SPECIFIED);

  cfg=cfg_init(opts,CFGF_NOCASE);
  ret=cfg_parse(cfg,configFile);
  if( ret==CFG_FILE_ERROR ){
	perror(configFile);
	return returnError( iError, CONFIG_ERROR_FILE_ERROR);
  }
  else if( ret==CFG_PARSE_ERROR ){
	printf("loadConfig[%d]: cfg_parse() parse error\n", __LINE__);
	return returnError(iError,CONFIG_ERROR_PARSE);
  }
  else if( ret!=CFG_SUCCESS ){
	printf("loadConfig[%d]: cfg_parse() return code = %d\n", __LINE__, ret);
	cfg_free(cfg);
	return returnError(iError, CONFIG_ERROR_RETURN_CODE);
  }
  data_portno=cfg_getint(cfg,"Listen_port");
  cmds_portno=cfg_getint(cfg,"Command_port");
  if( (p=cfg_getstr(cfg,"ip"))!=NULL ){
	strncpy(ip_address,p,ip_size);
	ip_address[ip_size-1]=0;
  }

  scale_cnt=cfg_size(cfg,"serial");
  if( scale_cnt==0 ){
	printf("loadConfig[%d]: Section \"serial\" cannot be empty\n", __LINE__);
	cfg_free(cfg);
	return returnError(iError,CONFIG_ERROR_PARSE_SECTION);
  }
//<Modified 14.06.2025  to create JSON config
  init_jarray(scale_cnt);
  for(i=0;i<scale_cnt;i++){
	s=(scale_sys_t *)malloc(sizeof(scale_sys_t));
	if( s==NULL ){
	  cfg_free(cfg);
	  clean_json();
	  block_json();
	  return returnError(iError,CONFIG_ERROR_MEMORY_LACK);
	}
	sec=cfg_getnsec(cfg,"serial",i);
	init_jobj(i);
//	s->ser=config_serial(sec);
	s->ser=conf_serial(sec,i);
//  printf("Port settings: %s\n",rs232_to_string(s->ser));
//printf("%s settings: baud=%d data=%d stop=%d flow=%d parity=%d\n",s->ser->dev,
//	s->ser->baud,s->ser->data,s->ser->stop,s->ser->flow,s->ser->parity);

	if( s->ser==NULL ){
	  clean_json();
	  block_json();
	  cfg_free(cfg);
	  return returnError(iError,CONFIG_ERROR_PARSE_SECTION);
	}
	s->id=cfg_getint(sec,"id");
	jobj_add_int(i,"id",s->id);
	s->dev_cnt=cfg_size(sec,"device");
	if( s->dev_cnt==0 ){
	  printf("loadConfig[%d]: Section \"device\" cannot be empty\n", __LINE__);
	  cfg_free(cfg);
	  clean_json();
	  block_json();
	  return returnError(iError,CONFIG_ERROR_PARSE_SECTION);
	}
	for(j=0;j<s->dev_cnt;j++){
	  opt=cfg_getnsec(sec,"device",j);
	  s->st[j]=(scale_term_t *)malloc(sizeof(scale_term_t));
	  s->st[j]->addr=cfg_getint(opt,"addr");
	  s->st[j]->model=NULL;
//< MJK 12.01.2026
	  s->st[j]->datatype_raw=0;
//>
	  if( (p=cfg_getstr(opt,"model")) ){
		jobj_add_str(i,"model",p);
		for(k=0;k<models_cnt;k++)
		  if( strcmp(p,mod_cmd[k]->model)==0 ) break;
		if( k==models_cnt ){ // add new model in the list
		  mod_cmd[models_cnt]=(model_cmd_t *)malloc(sizeof(model_cmd_t));
		  strcpy(mod_cmd[models_cnt]->model,p);
		  models_cnt++;
		}
		m=s->st[j]->model=mod_cmd[k];
		for(k=0;k<MAX_TERM_CMDS;k++) m->cmds[k]=NULL;
	  }
//< MJK 12.01.2026
	  if( (p=cfg_getstr(opt,"datatype")) ){
		jobj_add_str(i,"datatype",p);
		if( strcmp(p,"raw")==0 )
		  s->st[j]->datatype_raw=1;
	  }
//>
	}// alls devices in a scale system
	add_jitem(i);
//14.06.2025 MJK >
	scale[i]=s;
  }// all scale systems
//  jarray_convert();
  sec=cfg_getnsec(cfg,"command",0);
  if( sec!=0 )
	parse_commands(sec);

  //process other sections
  packet_length = (unsigned int)cfg_getint(cfg,"Length");

  sec=cfg_getnsec(cfg,"Weight",0);
  if( sec==0 ){
	printf("Section \"Weight\" cannot be empty: %s\n",configFile);
	return returnError(iError,CONFIG_ERROR_PARSE_SECTION);
  }
  weight_pos=(unsigned int)cfg_getint(sec,"Position");
  weight_len=(unsigned int)cfg_getint(sec,"Length");

  sec=cfg_getnsec(cfg,"Status",0);
  if( sec!=0 ){
	status_pos=(unsigned int)cfg_getint(sec,"Position");
	status_len=(unsigned int)cfg_getint(sec,"Length");
    for(i=0;i<4;i++){
	  status[i] = (char*)malloc((status_len+1)*sizeof(char));
	  memset(status[i],'\0',status_len+1);
	}
	if( cfg_getstr(sec,"Stable")!=0 )
	  strncpy(status[STATUS_STABLE],cfg_getstr(sec,"Stable"),status_len);
	if( cfg_getstr(sec,"Unstable")!=0 )
	  strncpy(status[STATUS_UNSTABLE],cfg_getstr(sec,"Unstable"),status_len);
	if( cfg_getstr(sec,"OverLoad")!=0 )
	  strncpy(status[STATUS_OVERLOAD],cfg_getstr(sec,"OverLoad"),status_len);
	if( cfg_getstr(sec,"UnderLoad")!=0 )
	  strncpy(status[STATUS_OVERLOAD],cfg_getstr(sec,"UnderLoad"),status_len);
  }

  sec=cfg_getnsec(cfg,"Gross",0);
  if( sec!=0 ){
	gross_pos=(unsigned int)cfg_getint(sec,"Position");
	gross_pos=gross_pos;
	gross_len=(unsigned int)cfg_getint(sec,"Length");
	for(i=0;i<2;i++){
	  gross[i]=(char*)malloc((gross_len+1)*sizeof(char));
	  memset(gross[i],'\0',gross_len+1);
	}
	if( cfg_getstr(sec,"Net")!=0 )
	  strncpy(gross[NET],cfg_getstr(sec,"Net"),gross_len);
	if( cfg_getstr(sec,"Gross")!=0 )
	  strncpy(gross[GROSS],cfg_getstr(sec,"Gross"),gross_len);
  }

  sec=cfg_getnsec(cfg,"Unit",0);
  if( sec!=0 ){
	unit_pos=(unsigned int)cfg_getint(sec,"Position");
	unit_pos=unit_pos;
	unit_len=(unsigned int)cfg_getint(sec,"Length");
	for(i=0;i<2;i++){
	  unit[i]=(char*)malloc((unit_len+1)*sizeof(char));
	  memset(gross[i],'\0',unit_len + 1);
	}
	if( cfg_getstr(sec,"Kg")!=0 )
	  strncpy(unit[Kg],cfg_getstr(sec,"Kg"),unit_len);
	if( cfg_getstr(sec,"g")!=0 )
	  strncpy(unit[g],cfg_getstr(sec,"g"),unit_len);
  }
  cfg_free(cfg);
  if( iError )
	*iError=CONFIG_ERROR_NOT_ERROR;
  return CONFIG_FUNCTION_RETURN_OK; 
}

