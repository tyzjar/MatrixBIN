#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);

/*
 * int switchUnit( VDB *pVDB)
 */
int switchUnit( VDB *pVDB)
{
	fprintf( stdout, "switchUnit: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	
	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;

	int iRc = MATRIXLIB_OK;

	SwitchUnit_t* unit = (SwitchUnit_t*)pVDB->pParam;
	int idx = getIdxScale( unit->id_scale);
	if( idx == -1) {
		return MATRIXLIB_SCALE_ERROR;
	}

	// unit в массиве ? 
	if( (int)(Configuration.units.size()-1) < unit->idx_unit)
		return MATRIXLIB_PARAM_ERROR;

	Configuration.scaleConfiguration[idx].idx_unit = unit->idx_unit;
		
	return iRc;
}
