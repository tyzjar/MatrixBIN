#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tarePreset( VDB *pVDB)
 */
int tarePreset( VDB *pVDB)
{
	fprintf( stdout, "tarePreset: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
