#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int tareSave( VDB *pVDB)
 */
int tareSave( VDB *pVDB)
{
	fprintf( stdout, "tareSave: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
