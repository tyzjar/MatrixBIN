#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tareMode( VDB *pVDB)
 */
int tareMode( VDB *pVDB)
{
	fprintf( stdout, "tareMode: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
