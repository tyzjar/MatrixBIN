#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int saveConfiguration( VDB *pVDB)
 */
int saveConfiguration( VDB *pVDB)
{
	fprintf( stdout, "saveConfiguration: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
