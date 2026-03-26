#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int getTAC( VDB *pVDB)
 */
int getTAC(VDB *pVDB)
{
	fprintf( stdout, "getTAC: %p\n", (void*)pVDB);
	
	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	

	int iRc = MATRIXLIB_OK;

	return iRc;
}
