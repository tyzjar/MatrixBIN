#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setRangeMode( VDB *pVDB)
 */
int setRangeMode( VDB *pVDB)
{
	fprintf( stdout, "setRangeMode: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
