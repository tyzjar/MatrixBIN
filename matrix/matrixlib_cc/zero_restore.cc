#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int zeroRestore( VDB *pVDB)
 */
int zeroRestore( VDB *pVDB)
{
	fprintf( stdout, "zeroRestore: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
