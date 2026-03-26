#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

/*
 * int normalizeWeight( VDB *pVDB)
 */
int normalizeWeight( VDB *pVDB)
{
	fprintf( stdout, "normalizeWeight: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	int iRc = MATRIXLIB_OK;

	return iRc;
}
