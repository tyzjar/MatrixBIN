#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setWarmUpTime( VDB *pVDB)
 */
int setWarmUpTime( VDB *pVDB)
{
	fprintf( stdout, "setWarmUpTime: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
