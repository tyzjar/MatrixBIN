#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setIncrementSize( VDB *pVDB)
 */
int setIncrementSize( VDB *pVDB)
{
	fprintf( stdout, "setIncrementSize: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
