#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);
extern int setCalibrationGaneChannel( Channel_t*, int, int);

/*
 * int setCalibrateGane( VDB *pVDB)
 */
int setCalibrateGane( VDB *pVDB)
{
	fprintf( stdout, "setCalibrateGane: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	int iRc = MATRIXLIB_OK;
	
	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;
	
	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	GaneCalibration_t* ganeCalibration = (GaneCalibration_t*)pVDB->pParam;
	int idx = getIdxChannel( ganeCalibration->channel);
	if( idx == -1) {
		return MATRIXLIB_CHANNEL_ERROR;
	}
	
	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
		
	// ущё не установлен калибровочный нуль ?	
		if( pChannel->zero_calibration_value == -1)
			return MATRIXLIB_EXECUTION_SEQUENCE_ERROR;
		
		if( ganeCalibration->weight_value > pChannel->max_weight_value)
			return MATRIXLIB_PARAM_ERROR;
			
		iRc = setCalibrationGaneChannel( pChannel, ganeCalibration->raw_value, ganeCalibration->weight_value);
	}	

	return iRc;
}
