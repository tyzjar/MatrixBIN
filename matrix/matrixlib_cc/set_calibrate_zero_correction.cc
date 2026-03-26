#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);
extern int correctionZeroLinearSections(Channel_t*);

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

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;
	
	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	 
	ZeroCorrection_t* zeroCorrection = (ZeroCorrection_t*)pVDB->pParam;

	int idx = getIdxChannel( zeroCorrection->channel);
	if( idx == -1) {
		return MATRIXLIB_PARAM_ERROR;
	}
	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
	
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
		
	// ущё не установлен калибровочный нуль ?	
		if( pChannel->zero_calibration_value == -1)
			return MATRIXLIB_EXECUTION_SEQUENCE_ERROR;
		
		// raw value
		pChannel->diff_zero_value = pChannel->zero_calibration_value - zeroCorrection->zero_correction_value;
		pChannel->zero_calibration_value = zeroCorrection->zero_correction_value;
		correctionZeroLinearSections(pChannel);
	}
	return iRc;
}
