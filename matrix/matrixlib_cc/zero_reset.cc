#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int zeroReset( VDB *pVDB)
 */
int zeroReset( VDB *pVDB)
{
	fprintf( stdout, "zeroReset: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
