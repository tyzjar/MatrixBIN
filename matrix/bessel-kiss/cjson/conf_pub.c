
// Пример конфигурации
Dev_t devices[4] = {
  {1,(char*)"LDU781",(char*)"/dev/ttyS1",115200,8,1,(char*)"off",(char*)"none"},
  {2,(char*)"LDU781",(char*)"/dev/ttyS2",115200,8,1,(char*)"off",(char*)"none"},
  {3,(char*)"LDU781",(char*)"/dev/ttyS3",115200,8,1,(char*)"off",(char*)"none"},
  {4,(char*)"LDU781",(char*)"/dev/ttyS4",115200,8,1,(char*)"off",(char*)"none"}
};

char *json_str_configuration=NULL;

/*
  В функции обработки соединения создаем json конфигурации
  (согласно конфигурационному файлу) и пубикуем ее в "calibration/configuration"
  здесь же подписываемся на "calibration/cmd"

calibration/configuration
{
  "id": "0",
  "dev": "LDU781",
  "tty": "/dev/ttyS1",
  "baud": "115200",
  "data": "8",
  "stop": "1",
  "flow": "off",
  "parity": "none"
},
  {},
  ...
*/
    
int createConfiguration(char** szConfiguration)
{
int rc=1;
cJSON *array, *config;
char *szConfig, buf[100];
int i;
char buf[50];

  array=cJSON_CreateArray();
  if( array==NULL ){
	fprintf( stderr, "Error: cJSON_CreateArray()\n");
	return 0;
  }
  for(i=0;i<4;++i){
	cJSON* json_device=cJSON_CreateObject();		
	if( json_device==NULL ){
	  fprintf(stderr,"Error: cJSON_CreateObject(%d)\n",i);
	  rc=0;
	  break;
	}
	snprintf(buf,sizeof(buf),"%d",devices[i].id);
				
	if( !cJSON_AddStringToObject(json_device,"id",buf)){
	  fprintf( stderr, "Error: cJSON_AddStringToObject(%d, id)\n", i);
	  rc=0;
	  break;
	}
	if( !cJSON_AddStringToObject(json_device,"dev",devices[i].szDev)){
	  fprintf(stderr,"Error: cJSON_AddStringToObject(%d,dev)\n",i);
	  rc=0;
	  break;
	}
	if( !cJSON_AddStringToObject(json_device,"tty",devices[i].szTTY)){
	  fprintf(stderr,"Error: cJSON_AddStringToObject(%d,tty)\n",i);
	  rc=0;
	  break;
	}
		snprintf( buf, 50, "%d", devices[i].baud);
		if( !cJSON_AddStringToObject( json_device, "baud", buf)) {
			fprintf( stderr, "Error: cJSON_AddStringToObject(%d, baud)\n", i);
			rc = 0;
			break;
		}
		snprintf( buf, 50, "%d", devices[i].data);
		if( !cJSON_AddStringToObject( json_device, "data", buf)) {
			fprintf( stderr, "Error: cJSON_AddStringToObject(%d, data)\n", i);
			rc = 0;
			break;
		}
		snprintf( buf, 50, "%d", devices[i].stop);
		if( !cJSON_AddStringToObject( json_device, "stop", buf)) {
			fprintf( stderr, "Error: cJSON_AddStringToObject(%d, stop)\n", i);
			rc = 0;
			break;
		}
		if( !cJSON_AddStringToObject( json_device, "flow", devices[i].flow)) {
			fprintf( stderr, "Error: cJSON_AddStringToObject(%d, flow)\n", i);
			rc = 0;
			break;
		}
		if( !cJSON_AddStringToObject( json_device, "parity", devices[i].parity)) {
			fprintf( stderr, "Error: cJSON_AddStringToObject(%d, parity)\n", i);
			rc = 0;
			break;
		}
		if( !cJSON_AddItemToArray( array, json_device)) {
			fprintf( stderr, "Error: AddItemToArray(%d)\n", i);
			rc = 0;
			break;
		}
	}

	if( rc) {
		szConfig = cJSON_Print(config);
		if( szConfig) {
			*szConfiguration = szConfig;
		}
		else {
			fprintf( stderr, "mqtt createConfiguration: Error cJSON_Print()\n");
			rc = 0;
			*szConfiguration = NULL;
		}
	}

	cJSON_Delete(config);

	return rc;
}

void mqtt_matrix::on_connect(int rc)
{
	printf("mqtt_matrix:  Connected with code %d.\n", rc);
	if(rc == 0){

		if( json_str_configuration == NULL) {
			if( createConfiguration( &json_str_configuration)) {
				publish(NULL, "calibration/configuration", strlen(json_str_configuration), json_str_configuration, 0, 1);
			}
			else
				return;
		}
		subscribe( NULL, "calibration/cmd");
			
	}
}


int	duplicateCmd( cJSON **j_response, cJSON *j_cmd, cJSON *j_parameter)
{
	int rc=0;

	cJSON *j_cmd_resp = cJSON_CreateObject();

	if( j_cmd_resp == NULL) {
		fprintf(stderr, "Error: Error cJSON_CreateObject().\n");
		return rc;
	}
	if( !cJSON_AddStringToObject( j_cmd_resp, "cmd", j_cmd->valuestring)) {
		fprintf( stderr, "Error: cJSON_AddStringToObject(cmd)\n");
		return rc;
	}
	if( !cJSON_AddStringToObject( j_cmd_resp, "parameter", j_parameter->valuestring)) {
		fprintf( stderr, "Error: cJSON_AddStringToObject(parameter)\n");
		return rc;
	}
	if( !cJSON_AddItemToObject(*j_response, "cmd", j_cmd_resp)) {
		fprintf( stderr, "Error: cJSON_AddItemToObject(cmd)\n");
		return rc;
	}

	rc = 1;
	return rc;
}

int processingCmd( cJSON **tree, cJSON **tree_response)
{
	int	rc=0;
	cJSON *j_cmd,
		  *j_parameter,
		  *j_response;
	char buf[200],
		 *str_response=(char*)"";
	int error=0;	
	
	j_cmd = cJSON_GetObjectItem(*tree, "cmd");
	if( j_cmd == NULL) {
		fprintf(stderr, "Error: Payload missing data 'cmd'.\n");
		return rc;
	}
	j_parameter = cJSON_GetObjectItem(*tree, "parameter");
	if( j_parameter == NULL) {
		fprintf(stderr, "Error: Payload missing data 'parameter'.\n");
		return rc;
	}
	
	printf( "cmd: '%s', parameter: '%s'\n", j_cmd->valuestring, j_parameter->valuestring);
	
// object for response	
	j_response = cJSON_CreateObject();		
	if( j_response == NULL) {
		fprintf(stderr, "Error: Error cJSON_CreateObject().\n");
		return rc;
	}

// duplicate object 'cmd' 	
	if( !duplicateCmd( &j_response, j_cmd, j_parameter)) {
		fprintf(stderr, "Error: Error duplicateCmd().\n");
		return rc;
	}


	if( !strcmp(j_cmd->valuestring, "channel")){
		select_channel = atoi( j_parameter->valuestring);
		error = 0;
		str_response = (char*)"OK";		
	} 

	if( !cJSON_AddStringToObject( j_response, "response", str_response)) {
		fprintf( stderr, "Error: cJSON_AddStringToObject(response)\n");
		return rc;
	}

	snprintf( buf, sizeof(buf),"%d", error);
	if( !cJSON_AddStringToObject( j_response, "error", buf)) {
		fprintf( stderr, "Error: cJSON_AddStringToObject(error)\n");
		return rc;
	}
	
	rc = 1;
	*tree_response = j_response;
	return rc;
}

// При получении подписки
// "calibration/cmd"

void mqtt_matrix::on_message(const struct mosquitto_message *message)
{
	double temp_celsius, temp_fahrenheit;
	char buf[200+1];

// "calibration/cmd"
	if(!strcmp(message->topic, "calibration/cmd")){
		printf( "-> %s\n", message->topic);

		cJSON *tree,
		      *tree_response = NULL;
		char* json_str;
		
		memset(buf, 0, sizeof(buf));
	/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, sizeof(buf) - 1);

#if CJSON_VERSION_FULL < 1007013
		tree = cJSON_Parse(buf);
#else
		int payloadlen = strlen( buf);
		tree = cJSON_ParseWithLength(buf, (size_t)payloadlen);
#endif	
		if(tree == NULL){
			fprintf(stderr, "Error: (%s) Payload not JSON.\n", message->topic);
			return;
		}
		
		if( processingCmd( &tree, &tree_response)) {
			json_str = cJSON_Print(tree_response);
			publish(NULL, "calibration/cmd_response", strlen(json_str), json_str);
			free(json_str);
			cJSON_Delete(tree_response);
			
		}
		cJSON_Delete(tree);
		return;		
		
	}

