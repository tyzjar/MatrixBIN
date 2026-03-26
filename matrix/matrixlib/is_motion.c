#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int isMotion( VDB *pVDB)
 */
int isMotion( VDB *pVDB)
{
	fprintf( stdout, "isMotion: %p\n", (void*)pVDB);
	
	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	

	int iRc = MATRIXLIB_OK;

	return iRc;
}
