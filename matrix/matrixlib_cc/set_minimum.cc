#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);


/*
 * int setMinimum( VDB *pVDB)
 */
int setMinimum( VDB *pVDB)
{
	fprintf( stdout, "setMinimum: %p\n", (void*)pVDB);
	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;
	
	MinimumWeight_t* minWeight =  (MinimumWeight_t*)pVDB;

	int idx = getIdxScale( minWeight->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}

	Configuration.scaleConfiguration[idx].minimum_weight = minWeight->minimum_weight;
	
	return iRc;
}
