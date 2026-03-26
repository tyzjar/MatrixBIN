#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);

/*
 * int setMaximumChannel( VDB *pVDB)
 */
int setMaximumChannel( VDB *pVDB)
{
	fprintf( stdout, "setMaximumChannel: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	
	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	int iRc = MATRIXLIB_OK;

	SetMaximumChannel_t *setMaxChannel = (SetMaximumChannel_t *)pVDB->pParam;
	int idx = getIdxChannel( setMaxChannel->channel);
	if( idx == -1) {
		return MATRIXLIB_CHANNEL_ERROR;
	}

	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
	pChannel->max_weight_value = setMaxChannel->max_weight_value;
	
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
// Удалим линейные участки со значеними выше максимального значения
		for( int i = (int)pChannel->linearSections.size()-1; i>=0; i--) {
			if( pChannel->max_weight_value < pChannel->linearSections[i].raw_value) {
				pChannel->linearSections.erase( pChannel->linearSections.begin() + i);
			}
		}
	}

	return iRc;
}
