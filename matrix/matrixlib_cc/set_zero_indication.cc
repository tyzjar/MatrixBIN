#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * int setZeroIndication( VDB *pVDB)
 */
int setZeroIndication( VDB *pVDB)
{
	fprintf( stdout, "setZeroIndication: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;

	ZeroIndicationSet_t* set = (ZeroIndicationSet_t*)pVDB->pParam;
	int idx = getIdxScale( set->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}
	Configuration.scaleConfiguration[idx].zero.zero_indication.diff_center_d = set->diff_center_d;
	Configuration.scaleConfiguration[idx].zero.zero_indication.underload_d = set->underload_d;
	
	return iRc;
}
