#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setMaximum( VDB *pVDB)
 */
int setMaximum( VDB *pVDB)
{
	fprintf( stdout, "setMaximum: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
