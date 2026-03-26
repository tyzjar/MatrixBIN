#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * int setMaximum( VDB *pVDB)
 */
int setMaximum( VDB *pVDB)
{
	fprintf( stdout, "setMaximum: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;
	
	MaximumWeight_t* maxWeight =  (MaximumWeight_t*)pVDB;

	int idx = getIdxScale( maxWeight->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}

	Configuration.scaleConfiguration[idx].maximum_weight = maxWeight->maximum_weight;
	
	return iRc;
}
