#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * int setZeroInitialRange( VDB *pVDB)
 */
int setZeroInitialRange( VDB *pVDB)
{
	fprintf( stdout, "setZeroInitialRange: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;

	ZeroSet_t* set = (ZeroSet_t*)pVDB->pParam;
	int idx = getIdxScale( set->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}
	Configuration.scaleConfiguration[idx].zero.type_zero = set->type_zero;
	Configuration.scaleConfiguration[idx].zero.percent_max = set->percent_max;
	Configuration.scaleConfiguration[idx].zero.add_coef = set->add_coef;

	return iRc;
}
