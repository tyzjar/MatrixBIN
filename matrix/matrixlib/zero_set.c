#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int zeroSet( VDB *pVDB)
 */
int zeroSet( VDB *pVDB)
{
	fprintf( stdout, "zeroSet: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
