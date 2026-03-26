#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tareReset( VDB *pVDB)
 */
int tareReset( VDB *pVDB)
{
	fprintf( stdout, "tareReset: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
