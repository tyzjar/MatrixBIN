#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int print( VDB *pVDB)
 */
int print( VDB *pVDB)
{
	fprintf( stdout, "print: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
