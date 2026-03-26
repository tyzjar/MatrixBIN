#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tareSet( VDB *pVDB)
 */
int tareSet( VDB *pVDB)
{
	fprintf( stdout, "tareSet: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
