#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int zeroTracking( VDB *pVDB)
 */
int zeroTracking( VDB *pVDB)
{
	fprintf( stdout, "zeroTracking: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
