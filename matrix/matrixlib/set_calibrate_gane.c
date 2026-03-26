#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern ScaleConfiguration_t scaleConfiguration;

extern int getIdxChannel( int channel);


/*
 * int setCalibrateGane( VDB *pVDB)
 */
int setCalibrateGane( VDB *pVDB)
{
	fprintf( stdout, "setCalibrateGane: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(scaleConfiguration.flag_scale & FL_SCALE_CONFIGURATION_READY))
		return MATRIXLIB_SCALE_NOT_CONFIGURATION;
	
	if( !(scaleConfiguration.flag_scale & FL_SCALE_METROLOGICAL_VERIFICATION))
		return MATRIXLIB_SCALE_NO_IN_METROLOGICAL_VERIFICATION;
	
	GaneCalibration_t* ganeCalibration = (GaneCalibration_t*)pVDB->pParam;
	int idx = getIdxChannel( ganeCalibration->channel);
	if( idx == -1) {
		return MATRIXLIB_PARAM_ERROR;
	}
	
	Channel_t* pChannel = &scaleConfiguration.channels[idx];
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
		
	// ущё не установлен калибровочный нуль ?	
		if( pChannel->zero_calibration_value == -1)
			return MATRIXLIB_EXECUTION_SEQUENCE_ERROR;
		
		
	}	

	int iRc = MATRIXLIB_OK;

	return iRc;
}
