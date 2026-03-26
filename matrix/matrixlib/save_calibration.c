#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int saveCalibration( VDB *pVDB)
 */
int saveCalibration( VDB *pVDB)
{
	fprintf( stdout, "saveCalibration: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
