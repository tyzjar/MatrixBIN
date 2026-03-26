#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int processingCurMeasurement( VDB *pVDB)
 */
int processingCurMeasurement( VDB *pVDB)
{
	fprintf( stdout, "processingCurMeasurement: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	
	int iRc = MATRIXLIB_OK;

	return iRc;
}
