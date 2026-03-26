#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);


/*
 * int addScale( VDB *pVDB)
 */
int addScale( VDB *pVDB)
{
	fprintf( stdout, "addScale: %p\n", (void*)pVDB);
	
	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	if( !(Configuration.flag_system & FL_SYSTEM_SERVICE_MODE))
		return MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE;
	
	int iRc = MATRIXLIB_OK;
	
	AddScale_t* addScale =  (AddScale_t*)pVDB;

	int idx = getIdxScale( addScale->id_scale);
	if( idx != -1) {
		return MATRIXLIB_SCALE_ERROR;
	}

	Configuration.scaleConfiguration.resize( Configuration.scaleConfiguration.size()+1);
	Configuration.scaleConfiguration[Configuration.scaleConfiguration.size()-1].id_scale = addScale->id_scale;
		
	return iRc;
}
