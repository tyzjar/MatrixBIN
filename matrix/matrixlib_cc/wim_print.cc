#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int wimPrint( VDB *pVDB)
 */
int wimPrint( VDB *pVDB)
{
	fprintf( stdout, "wimPrint: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
