#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int switchUnit( VDB *pVDB)
 */
int switchUnit( VDB *pVDB)
{
	fprintf( stdout, "switchUnit: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
