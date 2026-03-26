#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);
extern int initLinerarSections(Channel_t*);

/*
 * int setCalibrateZero( VDB *pVDB)
 */
int setCalibrateZero( VDB *pVDB)
{
	fprintf( stdout, "setCalibrateZero: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;
	
	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;

	int iRc = MATRIXLIB_OK;
	
	ZeroCalibration_t* zeroCalibration = (ZeroCalibration_t*)pVDB->pParam;

	int idx = getIdxChannel( zeroCalibration->channel);
	if( idx == -1) {
		return MATRIXLIB_PARAM_ERROR;
	}
	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
	
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
		pChannel->zero_calibration_value = zeroCalibration->zero_calibration_value;
		pChannel->diff_zero_value = 0;
		initLinerarSections(pChannel);
	}

	return iRc;
}
