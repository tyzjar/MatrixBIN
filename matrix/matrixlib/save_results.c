#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int saveResults( VDB *pVDB)
 */
int saveResults( VDB *pVDB)
{
	fprintf( stdout, "saveResults: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
