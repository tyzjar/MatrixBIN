#include <stdio.h>
#include <string.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

/*
 * int libVersion( VDB *pVDB)
 */

int libVersion( VDB *pVDB)
{
	fprintf( stdout, "libVersion: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}
	int iRc = MATRIXLIB_OK;
	
	LibVersion_t *p = (LibVersion_t *)pVDB->pParam;
	
	if( p != NULL) {
		char szVersion[20];
		
		snprintf( szVersion, sizeof( szVersion), MATRIXLIB_VERSION_STR, MATRIXLIB_MAJOR_VERSION, 
				 	MATRIXLIB_MINOR_VERSION, MATRIXLIB_MODIFICATION_VERSION);
		szVersion[sizeof(szVersion)-1] = 0;
    	p->szLibVersion = strdup(szVersion);
	}
    else {
    	iRc = MATRIXLIB_PARAM_ERROR;
	}
	return iRc;
}

