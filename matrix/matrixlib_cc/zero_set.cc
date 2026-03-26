#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxScale( int);


/*
 * int zeroSet( VDB *pVDB)
 */
int zeroSet( VDB *pVDB)
{
	fprintf( stdout, "zeroSet: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	int iRc = MATRIXLIB_OK;	
	
	return iRc;
}
