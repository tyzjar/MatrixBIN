#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int resetToFactory( VDB *pVDB)
 */
int resetToFactory( VDB *pVDB)
{
	fprintf( stdout, "resetToFactory: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
