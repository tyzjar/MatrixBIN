#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cjson/cJSON.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

static int parsePlatformSection( int idx_platform, cJSON *json_platform)
{
	int rc = 1;

	cJSON* json_platform_sections = cJSON_GetObjectItem( json_platform, "sections");
	if( cJSON_IsArray( json_platform_sections)) {
		int cnt_sections = cJSON_GetArraySize(json_platform_sections);
		Platform_t *platform = &Configuration.loadReceptor.platforms[idx_platform];
		platform->sections.resize(cnt_sections);
		for( int i=0; rc && (i < cnt_sections); ++i) {
			cJSON* json_platform_section = cJSON_GetArrayItem( json_platform_sections, i);
			if( json_platform_section) {
				cJSON* json_id_section = cJSON_GetObjectItem( json_platform_section, "id");
				if( json_id_section)
					platform->sections[i].id_section = cJSON_GetNumberValue(json_id_section);
				else {
					rc = 0;
					break;
				}
				cJSON* json_live_rail = cJSON_GetObjectItem( json_platform_section, "live_rail");
				if( json_live_rail)
					platform->sections[i].live_rail = cJSON_GetNumberValue(json_live_rail);
				else {
					rc = 0;
					break;
				}
				cJSON* json_dead_rail = cJSON_GetObjectItem( json_platform_section, "dead_rail");
				if( json_dead_rail)
					platform->sections[i].dead_rail = cJSON_GetNumberValue(json_dead_rail);
				else {
					rc = 0;
					break;
				}
			}
			else {
				rc = 0;
				break;
			}
		} // for
	}
	else
		rc = 0;
	return rc;
}


static int parserGPU( cJSON	*tree)
{
	int  rc = 1;
	char *gpu_description;

	cJSON* json_gpu = cJSON_GetObjectItem( tree, "gpu");
	if( json_gpu && cJSON_IsObject(json_gpu)) {
		cJSON* json_description = cJSON_GetObjectItem( json_gpu, "description");
		gpu_description = cJSON_GetStringValue( json_description);
		if( gpu_description)  {
			Configuration.loadReceptor.description = strdup( gpu_description);
		}
		else
			rc = 0;
		if( rc) {
			cJSON* json_platforms = cJSON_GetObjectItem( json_gpu, "platforms");
			if( cJSON_IsArray( json_platforms)) {
				int cnt_platforms = cJSON_GetArraySize(json_platforms); 
				Configuration.loadReceptor.platforms.resize(cnt_platforms);
				for( int i=0; i < cnt_platforms; ++i) {
					cJSON* json_platform = cJSON_GetArrayItem( json_platforms, i);
					if( json_platform) {
						cJSON* json_id_platform = cJSON_GetObjectItem( json_platform, "id");
						if( json_id_platform)
							Configuration.loadReceptor.platforms[i].id_platform = cJSON_GetNumberValue(json_id_platform); 
						else {
							rc = 0;
							break;
						}
						cJSON* json_description_platform = cJSON_GetObjectItem( json_platform, "description");
						if( json_description_platform) {
							char *description = cJSON_GetStringValue(json_description_platform);
							if( description) {
								Configuration.loadReceptor.platforms[i].description = strdup( description);
							}
							else {
								rc = 0;
								break;
							}							
						}
						else {
							rc = 0;
							break;
						}
						rc = parsePlatformSection( i, json_platform);
						if( !rc)
							break;
					}
					else {
						rc = 0;
						break;
					}
				}
			}
			else
				rc = 0;
		}
	}
	else
		rc = 0;
	
	return rc;
}

static int parseMetrologyChannel( int idx, cJSON *json_channel)
{
	int rc = 1;

	Channel_t* channel = &Configuration.channelConfiguration[idx];
	
	cJSON* json_metrology = cJSON_GetObjectItem( json_channel, "metrology");
	if( json_metrology && cJSON_IsObject( json_metrology)) {
		cJSON* json_max_weight_value = cJSON_GetObjectItem( json_metrology, "max_weight_value");
		if( json_max_weight_value)
			channel->max_weight_value = cJSON_GetNumberValue(json_max_weight_value); 
		else
			rc = 0;
		if( rc) {
			cJSON* json_zero_calibration_value = cJSON_GetObjectItem( json_metrology, "zero_calibration_value");
			if( json_zero_calibration_value)
				channel->zero_calibration_value = cJSON_GetNumberValue(json_zero_calibration_value); 
			else
				rc = 0;
		}
		if( rc) {
			cJSON* json_linear_sections = cJSON_GetObjectItem( json_metrology, "linear_sections");
			if( json_linear_sections && cJSON_IsArray( json_linear_sections)) {
				int cnt_sections = cJSON_GetArraySize(json_linear_sections);
				channel->linearSections.resize(cnt_sections);
				for( int i=0; i<cnt_sections; i++) {
					cJSON* json_section = cJSON_GetArrayItem( json_linear_sections, i);
					if( json_section && cJSON_IsObject(json_section)) {
						cJSON* json_weight = cJSON_GetObjectItem( json_section, "weight");
						if( json_weight) {
							channel->linearSections[i].weight_value = cJSON_GetNumberValue(json_weight);
						}
						else {
							rc = 0;
							break;
						}
						
						cJSON* json_counter = cJSON_GetObjectItem( json_section, "counter");
						if( json_counter) {
							channel->linearSections[i].raw_value = cJSON_GetNumberValue(json_counter);
						}
						else {
							rc = 0;
							break;
						}
						
					}
					else {
						rc = 0;
						break;
					}
				}
			}
		}
	}
	else
		rc = 0;
	return rc;
}

static int parseChannels( cJSON *json_hardware)
{
	int rc = 1;

	cJSON* json_channels = cJSON_GetObjectItem( json_hardware, "channels");
	if( cJSON_IsArray( json_channels)) {
		int cnt_channels = cJSON_GetArraySize(json_channels);
		Configuration.channelConfiguration.resize(cnt_channels);
		for( int i=0; i < cnt_channels; i++) {
			Channel_t *channel = &Configuration.channelConfiguration[i];
			cJSON* json_channel = cJSON_GetArrayItem( json_channels, i);
			if( cJSON_IsObject(json_channel)) {
				cJSON* json_id_channel = cJSON_GetObjectItem( json_channel, "id");
				if( json_id_channel)
					channel->channel = cJSON_GetNumberValue(json_id_channel); 
				else {
					rc = 0;
					break;
				}

				cJSON* json_description_channel = cJSON_GetObjectItem( json_channel, "description");
				if( json_description_channel) {
					char *description = cJSON_GetStringValue(json_description_channel);
					if( description) {
						channel->description = strdup( description);
					}
					else {
						rc = 0;
						break;
					}							
				}
				else {
					rc = 0;
					break;
				}

				cJSON* json_com_port = cJSON_GetObjectItem( json_channel, "comPort");
				if( json_com_port)
					channel->com_port = cJSON_GetNumberValue(json_com_port); 
				else {
					rc = 0;
					break;
				}

				cJSON* json_dev_type = cJSON_GetObjectItem( json_channel, "deviceType");
				if( json_dev_type) {
					channel->device_type = cJSON_GetNumberValue(json_dev_type); 
				}
				else {
					rc = 0;
					break;
				}
				
				cJSON* json_sensors = cJSON_GetObjectItem( json_channel, "sensors");
				if( json_sensors && cJSON_IsObject(json_sensors)) {

					cJSON* json_name = cJSON_GetObjectItem( json_sensors, "name");
					char* name;
					if( json_name && (name = cJSON_GetStringValue(json_name))) {
						channel->sensors.name = strdup( name);
					}
					else {
						rc = 0;
						break;
					}

					cJSON* json_type = cJSON_GetObjectItem( json_sensors, "type");
					char* type;
					if( json_type && (type = cJSON_GetStringValue(json_type))) {
						channel->sensors.type = strdup( type);
					}
					else {
						rc = 0;
						break;
					}

					cJSON* json_count = cJSON_GetObjectItem( json_sensors, "count");
					if( json_count)
						channel->sensors.count = cJSON_GetNumberValue(json_count); 
					else {
						rc = 0;
						break;
					}
					
					cJSON* json_connected = cJSON_GetObjectItem( json_channel, "connectedSections");
					if( json_connected && cJSON_IsArray( json_connected)) {
						int cnt_connected = cJSON_GetArraySize(json_connected);
						
						channel->pluginSections.resize(cnt_connected);
						
						for( int j=0; j<cnt_connected; j++) {
							cJSON* json_pl_sec = cJSON_GetArrayItem( json_connected, j);
							if( json_pl_sec && cJSON_IsObject(json_pl_sec)) {
								cJSON* json_id_platform = cJSON_GetObjectItem( json_pl_sec, "id_plaform");
								if( json_id_platform) 		
									channel->pluginSections[j].id_platform = cJSON_GetNumberValue(json_id_platform);
								else {
									rc = 0;
									break;
								}
								cJSON* json_id_section = cJSON_GetObjectItem( json_pl_sec, "id_section");
								if( json_id_section) 		
									channel->pluginSections[j].id_section = cJSON_GetNumberValue(json_id_section);
								else {
									rc = 0;
									break;
								}
								
							}
							else {
								rc = 0;
								break;
							}
						}
					}
					else {
						rc = 0;
						break;
					}

					cJSON* json_average_time = cJSON_GetObjectItem( json_channel, "average_time");
					if( json_average_time)
						channel->averag_value.averag_time = cJSON_GetNumberValue(json_average_time); 
					else {
						rc = 0;
						break;
					}

					cJSON* json_wim = cJSON_GetObjectItem( json_channel, "wim");
					if( json_wim) {
						if( cJSON_GetNumberValue(json_wim))
							channel->flag_channel |= FL_CHANNEL_IN_MOTION;
						else
							channel->flag_channel &= ~FL_CHANNEL_IN_MOTION;
							
					}
					else {
						rc = 0;
						break;
					}
					
					rc = parseMetrologyChannel( i, json_channel);
					
				}
				else {
					rc = 0;
					break;
				}
			}
			
			else {
				rc = 0;
				break;
			}
		}
	}
	else
		rc = 0;
	
	return rc;
}	

static int parserHardware( cJSON *tree)
{
	int  rc = 1;

	cJSON* json_hardware = cJSON_GetObjectItem( tree, "hardware");
	if( json_hardware && cJSON_IsObject(json_hardware)) {
		cJSON* json_dev_types = cJSON_GetObjectItem( json_hardware, "deviceTypes");
		if( cJSON_IsArray( json_dev_types)) {
			int cnt_dev_types = cJSON_GetArraySize(json_dev_types);
			Configuration.dev_types.resize( cnt_dev_types);
			for( int i=0; i < cnt_dev_types; i++) {
				cJSON* json_dev_type = cJSON_GetArrayItem( json_dev_types, i);
				if( json_dev_type) {
					char *dev = cJSON_GetStringValue(json_dev_type);
					if( dev) {
						Configuration.dev_types[i] = strdup( dev);
					}
					else {
						rc = 0;
						break;
					}
				}
				else {
					rc = 0;
					break;
				}
			}
		}
		else
			rc = 0;
		
		if( rc) {
			rc = parseChannels( json_hardware);
		}
	}
	else
		rc = 0;
	return rc;
}

static int parserScales( cJSON *tree)
{
	int  rc = 1;

	cJSON* json_scales = cJSON_GetObjectItem( tree, "scales");
	if( json_scales && cJSON_IsArray( json_scales)) {
		int cnt_scales = cJSON_GetArraySize(json_scales);
		Configuration.scaleConfiguration.resize( cnt_scales);
		for( int i=0; i<cnt_scales; i++) {
			cJSON* json_scale = cJSON_GetArrayItem( json_scales, i);
			if( json_scale && cJSON_IsObject( json_scale)) {
				cJSON* json_id = cJSON_GetObjectItem( json_scale, "id");
				if( json_id) 
					Configuration.scaleConfiguration[i].id_scale = cJSON_GetNumberValue(json_id);
				else {
					rc=0;
					break;
				}
				cJSON* json_units = cJSON_GetObjectItem( json_scale, "units");
				if( json_units) 
					Configuration.scaleConfiguration[i].idx_unit = cJSON_GetNumberValue(json_units);
				else {
					rc=0;
					break;
				}
				cJSON* json_modes_work = cJSON_GetObjectItem( json_scale, "modesWork");
				if( json_modes_work && cJSON_IsArray( json_modes_work)) {
					int cnt_modes = cJSON_GetArraySize(json_modes_work);
					for( int j=0; rc && (j<cnt_modes);j++) {
						cJSON* json_mode = cJSON_GetArrayItem( json_modes_work, j);
						if( json_mode) {
							char *mode = cJSON_GetStringValue(json_mode);
							if( mode) {
								if( !strcmp( mode, "В СТАТИКЕ")) {
									Configuration.scaleConfiguration[i].flag_mode_work |= SCALE_STATIC_MODE;
								}
								else if( !strcmp( mode, "В ДВИЖЕНИИ")) {
									Configuration.scaleConfiguration[i].flag_mode_work |= SCALE_MOTION_MODE;
								}
								else {
									rc = 0;
									break;
								}	
							}
							else {
								rc =0;
								break;
							}
						}
						else {
							rc =0;
							break;
						}
					}
				}
				else {
					rc=0;
					break;
				}
				
				if(!rc) 
					break;
				
				cJSON* json_channel_ids = cJSON_GetObjectItem( json_scale, "channelIds");
				if( json_channel_ids && cJSON_IsArray( json_channel_ids)) {
					int cnt_channel = cJSON_GetArraySize(json_channel_ids);
					Configuration.scaleConfiguration[i].id_channel.resize(cnt_channel);
					for( int j=0; rc && (j<cnt_channel);j++) {
						cJSON* json_channel = cJSON_GetArrayItem( json_channel_ids, j);
						if( json_channel)
							Configuration.scaleConfiguration[i].id_channel[j] = cJSON_GetNumberValue(json_channel);
						else {
							rc = 0;
							break;
						}
					}
				}
				else {
					rc=0;
					break;
				}
				
				cJSON* json_internal_discret = cJSON_GetObjectItem( json_scale, "internalDiscreteness");
				if( json_internal_discret && cJSON_IsObject( json_internal_discret)) {
					cJSON* json_value = cJSON_GetObjectItem( json_internal_discret, "value");
					if( json_value)
						Configuration.scaleConfiguration[i].discretChannel.value = cJSON_GetNumberValue(json_value);
					else {
						rc = 0;
						break;
					}
					cJSON* json_multiplier = cJSON_GetObjectItem( json_internal_discret, "multiplier");
					if( json_multiplier)
						Configuration.scaleConfiguration[i].discretChannel.multiplier = cJSON_GetNumberValue(json_multiplier);
					else {
						rc = 0;
						break;
					}
						
				}
				else {
					rc=0;
					break;
				}
				
				
				
			}
			else {
				rc = 0;
				break;
			}
		}
	}
	else
		rc=0;

	return rc;
}

static int parserJSON(char* conf)
{
	int rc=1;
	cJSON	*tree;
	char	*version;
	
#if CJSON_VERSION_FULL < 1007013
	tree = cJSON_Parse(conf);
#else
	int payloadlen = strlen( conf);
	tree = cJSON_ParseWithLength( conf, (size_t)payloadlen);
#endif	
	if(tree == NULL){
		fprintf( stderr, "parserJSON: ERROR tree: %s\n",  cJSON_GetErrorPtr());
		return 0;
	}
	cJSON* json_version = cJSON_GetObjectItem( tree, "version");
	if( json_version) {
		version  = cJSON_GetStringValue(json_version);
		if( version) {
			Configuration.version = strdup( version);
		}	
		else
			rc = 0;
	}
	else
		rc = 0;
	
	if( rc) {
		rc = parserGPU(tree);
	}
	if( rc) {
		rc = parserHardware(tree);
	}
	if( rc) {
		rc = parserScales(tree);
	}
	cJSON_Delete(tree);
	if( rc == 1) {
//		Configuration.version = strdup( version);
//		Configuration.loadReceptor.description = strdup( gpu_description);
	}
	return rc;
}	

/*
 * int loadConfiguration( VDB *pVDB)
 */
int loadConfiguration( VDB *pVDB)
{
	fprintf( stdout, "loadConfiguration: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	LoadConfiguration_t* json_file_conf = (LoadConfiguration_t*) pVDB->pParam;
	
	if( json_file_conf->file_configuration==NULL)
    	return MATRIXLIB_PARAM_ERROR; 

	struct stat sb; 
	if( stat( json_file_conf->file_configuration, &sb) == -1) {
		fprintf( stderr, "loadConfiguration: Error stat %s (%s)\n", json_file_conf->file_configuration,
					strerror( errno));
    	return MATRIXLIB_CONFIG_OPEN_ERROR; 
	}
	if( sb.st_size == 0) {
    	return MATRIXLIB_CONFIG_ERROR; 
	}
	char *json_conf = (char*)calloc( 1, sb.st_size + 1);
	if( json_conf == NULL)
		return MATRIXLIB_ALLOC_ERROR;
	
	int fd = open( json_file_conf->file_configuration, O_RDONLY);
	if( fd == -1) {
		fprintf( stderr, "loadConfiguration: Error open %s (%s)\n", json_file_conf->file_configuration,
					strerror( errno));
    	return MATRIXLIB_CONFIG_OPEN_ERROR; 
	}
	int cnt_bytes = (int)read( fd, json_conf, sb.st_size);
	close( fd);

	if( cnt_bytes != sb.st_size) {
		fprintf( stderr, "loadConfiguration: Error read %ld bytes\n", sb.st_size);
    	return MATRIXLIB_CONFIG_ERROR; 
	}
	
	if( !parserJSON(json_conf)) {
		fprintf( stderr, "loadConfiguration: Error JSON config\n");
    	return MATRIXLIB_CONFIG_ERROR; 
	}
	
	Configuration.json_config = strdup( json_conf);
	Configuration.flag_system |= FL_SYSTEM_CONFIGURATION_READY;
	int iRc = MATRIXLIB_OK;

	fprintf( stdout, "->version: '%s'\n", Configuration.version);
	fprintf( stdout, " ->gpu description: '%s'\n", Configuration.loadReceptor.description);
	for( int i=0; i < (int)Configuration.loadReceptor.platforms.size(); i++) {
		fprintf( stdout, "  ->platform id: %d\n", Configuration.loadReceptor.platforms[i].id_platform);
		fprintf( stdout, "  ->platform descr: %s\n", Configuration.loadReceptor.platforms[i].description);
		for( int j=0; j < (int)Configuration.loadReceptor.platforms[i].sections.size(); j++) {
			Section_t *section = &Configuration.loadReceptor.platforms[i].sections[j];
			fprintf( stdout, "   ->section id: %d\n", section->id_section);
			fprintf( stdout, "   ->section live_rail: %d\n", section->live_rail);
			fprintf( stdout, "   ->section dead_rail: %d\n", section->dead_rail);
		}
	}
	for( int i=0; i < (int)Configuration.dev_types.size(); i++) {
		fprintf( stdout, " ->dev type: '%s'\n", Configuration.dev_types[i]);
	}
	for( int j=0; j < (int)Configuration.channelConfiguration.size(); j++) {
		Channel_t *channel = &Configuration.channelConfiguration[j];
		fprintf( stdout, "  ->channel: %d\n", channel->channel);
		fprintf( stdout, "  ->descr: '%s'\n", channel->description);
		fprintf( stdout, "  ->comPort: %d\n", channel->com_port);
		fprintf( stdout, "  ->devType: %d\n", channel->device_type);
		fprintf( stdout, "   ->sensor name: '%s'\n", channel->sensors.name);
		fprintf( stdout, "   ->sensor type: '%s'\n", channel->sensors.type);
		fprintf( stdout, "   ->sensor count: %d\n", channel->sensors.count);
		for( int k=0; k < (int)channel->pluginSections.size(); k++) {
			fprintf( stdout, "   ->connect id_platform: %d\n", channel->pluginSections[k].id_platform);
			fprintf( stdout, "   ->connect id_section: %d\n", channel->pluginSections[k].id_section);
		}
		fprintf( stdout, "  ->average_time: %d\n", channel->averag_value.averag_time);
		fprintf( stdout, "  ->max_weight_value: %d\n", channel->max_weight_value);
		fprintf( stdout, "  ->zero_calibration_value: %d\n", channel->zero_calibration_value);
		for( int k=0; k < (int)channel->linearSections.size(); k++) {
			fprintf( stdout, "   ->linear weight: %d\n", channel->linearSections[k].weight_value);
			fprintf( stdout, "   ->linear raw: %d\n", channel->linearSections[k].raw_value);
		}
		fprintf( stdout, "  ->flag_channel: %X\n", channel->flag_channel);
	}	
	for( int j=0; j < (int)Configuration.scaleConfiguration.size(); j++) {
		ScaleConfiguration_t *scale = &Configuration.scaleConfiguration[j];
		fprintf( stdout, "  ->id_scale: %d\n", scale->id_scale);
		fprintf( stdout, "  ->idx_unit: %d\n", scale->idx_unit);
		fprintf( stdout, "  ->flag_mode_work: %X\n", scale->flag_mode_work);
		for( int j=0; j < (int)scale->id_channel.size(); j++)
			fprintf( stdout, "    ->channel: %d\n", scale->id_channel[j]);
		fprintf( stdout, "    ->discret value: %d\n",  scale->discretChannel.value);			
		fprintf( stdout, "    ->discret multiplier: %f\n",  scale->discretChannel.multiplier);			
	}	
	return iRc;
}
