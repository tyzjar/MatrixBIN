#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setLinearization( VDB *pVDB)
 */
int setLinearization( VDB *pVDB)
{
	fprintf( stdout, "setLinearization: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
