#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * int setDiscret( VDB *pVDB)
 */
int setDiscret( VDB *pVDB)
{
	fprintf( stdout, "setDiscret1: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	
	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;

	Discret_t* discret = (Discret_t*)pVDB->pParam;
	int idx = getIdxScale( discret->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}

	   
	Configuration.scaleConfiguration[idx].discretChannel.value 		= discret->value;
	Configuration.scaleConfiguration[idx].discretChannel.multiplier = discret->multiplier;
		
	return iRc;
}

