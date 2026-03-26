#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int modeX10( VDB *pVDB)
 */
int modeX10( VDB *pVDB)
{
	fprintf( stdout, "modeX10: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	
	int iRc = MATRIXLIB_OK;

	return iRc;
}
