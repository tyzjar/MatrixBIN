#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int wimSaveResults( VDB *pVDB)
 */
int wimSaveResults( VDB *pVDB)
{
	fprintf( stdout, "wimSaveResults: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
