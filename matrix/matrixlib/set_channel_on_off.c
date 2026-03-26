#include <stdio.h>

#include "interface_matrixlib.h"

/*
 * int setChannelOnOff( VDB *pVDB)
 */
int setChannelOnOff( VDB *pVDB)
{
	fprintf( stdout, "setChannelOnOff: %p\n", (void*)pVDB);

	int iRc = MATRIXLIB_OK;

	return iRc;
}
