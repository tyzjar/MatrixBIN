#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int loadConfiguration( VDB *pVDB)
 */
int loadConfiguration( VDB *pVDB)
{
	fprintf( stdout, "loadConfiguration: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
