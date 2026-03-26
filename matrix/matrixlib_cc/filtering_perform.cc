#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int filteringPerform( VDB *pVDB)
 */
int filteringPerform( VDB *pVDB)
{
	fprintf( stdout, "filteringPerform: %p\n", (void*)pVDB);
	
	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	

	int iRc = MATRIXLIB_OK;

	return iRc;
}
