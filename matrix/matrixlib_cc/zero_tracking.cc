#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * Функция весов: установка параметров слежения за Нулем
 * int zeroTracking( VDB *pVDB)
 */
int zeroTracking( VDB *pVDB)
{
	fprintf( stdout, "zeroTracking: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;

	ZeroTracking_t* tracking = (ZeroTracking_t*)pVDB->pParam;
	int idx = getIdxScale( tracking->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}
	Configuration.scaleConfiguration[idx].zero.auto_tracking_d = tracking->auto_tracking_d;
	Configuration.scaleConfiguration[idx].zero.auto_tracking_timeout = tracking->auto_tracking_timeout;
	
	return iRc;
}
