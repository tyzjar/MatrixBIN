#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int wimSolver( VDB *pVDB)
 */
int wimSolver( VDB *pVDB)
{
	fprintf( stdout, "wimSolver: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
