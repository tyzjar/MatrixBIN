#include "interface_matrixlib.h"
#include <dlfcn.h>  
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

int fun_lib( void* data)
{
	VDB* pvdb = (VDB*) data;
	printf( "FUN_LIB\n");
	
	return 0;
}
int main( int argc, char* argv[])
{
	void* handle;
	handle = dlopen( "./matrixlib.so", RTLD_NOW );
	if( handle != NULL) {
		printf( "dlopen() OK!\n");
		
		void (*vpi_initialization)();
		vpi_initialization = dlsym( handle, "initialization" );
		if( vpi_initialization)
			(*vpi_initialization)();
		else
			return -1;
		
		int (*vpiFun)(VDB*) ;
		vpiFun = dlsym( handle, "VpiMain" );	
		if( vpiFun) {
			int inst_data;
			
			VDB vdb;
			LibVersion lv;

			printf( "vpiFun OK!\n");
			
			vdb.pInstanceData = (void*)&inst_data;
			
			vdb.function = VPI_LIB_VERSION;
			vdb.pParam = &lv;
			vdb.fun = fun_lib;
			int irc = (*vpiFun)(&vdb);
			printf( "irc=%d, libVersion=%s\n", irc, lv.szLibVersion);
			
		}	
		else {
			printf( "Error: dlsym()\n");
		}
		dlclose( handle);
	}
	else
		printf( "Error: dlopen(), %p, %s\n", handle, dlerror());
	return 0;
}

