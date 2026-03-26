#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);

/*
 * int setAveragTime( VDB *pVDB)
 */
int setAveragTime( VDB *pVDB)
{
	fprintf( stdout, "setAveragTime: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	int iRc = MATRIXLIB_OK;
	
	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;
	
	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	AveragTime_t* averagTime = (AveragTime_t*)pVDB->pParam;
	int idx = getIdxChannel( averagTime->channel);
	if( idx == -1) {
		return MATRIXLIB_CHANNEL_ERROR;
	}
	
	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
	pChannel->averag_value.averag_time = averagTime->averag_time;
	pChannel->averag_value.averag_weight_value = 0;
	pChannel->averag_value.averag_raw_value = 0;
	pChannel->averag_value.start_time = time(NULL);
	pChannel->flag_channel &= ~FL_CHANNEL_AVERAG_VALUE;
	return iRc;
}
