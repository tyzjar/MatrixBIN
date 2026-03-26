#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int zeroSave( VDB *pVDB)
 */
int zeroSave( VDB *pVDB)
{
	fprintf( stdout, "zeroSave: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
