#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tareAutoClear( VDB *pVDB)
 */
int tareAutoClear( VDB *pVDB)
{
	fprintf( stdout, "tareAutoClear: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
