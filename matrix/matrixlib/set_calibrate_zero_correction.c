#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern ScaleConfiguration_t scaleConfiguration;

extern int getIdxChannel( int channel);

/*
 * Функции калибровки
 */

/*
 * int setCalibrateZeroCorrection( VDB *pVDB)
 * 
 */
int setCalibrateZeroCorrection( VDB *pVDB)
{
	fprintf( stdout, "setCalibrateZeroCorrection: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	int iRc = MATRIXLIB_OK;

	if( !(scaleConfiguration.flag_scale & FL_SCALE_CONFIGURATION_READY))
		return MATRIXLIB_SCALE_NOT_CONFIGURATION;
	
	if( !(scaleConfiguration.flag_scale & FL_SCALE_METROLOGICAL_VERIFICATION))
		return MATRIXLIB_SCALE_NO_IN_METROLOGICAL_VERIFICATION;
	 
	ZeroCorrection_t* zeroCorrection = (ZeroCorrection_t*)pVDB->pParam;

	int idx = getIdxChannel( zeroCorrection->channel);
	if( idx == -1) {
		return MATRIXLIB_PARAM_ERROR;
	}
	Channel_t* pChannel = &scaleConfiguration.channels[idx];
	
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
		
	// ущё не установлен калибровочный нуль ?	
		if( pChannel->zero_calibration_value == -1)
			return MATRIXLIB_EXECUTION_SEQUENCE_ERROR;
		
		pChannel->zero_correction_value = zeroCorrection->zero_correction_value;
		pChannel->diff_zero_value = pChannel->zero_calibration_value - pChannel->zero_correction_value;
	}
	return iRc;
}
