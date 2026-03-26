#include <dlfcn.h>  
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <math.h>


#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

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
		vpi_initialization = (void (*)())dlsym( handle, "initialization" );
		printf( "vpi_initialization: %p\n", vpi_initialization);
		if( vpi_initialization)
			(*vpi_initialization)();
		else {
			printf( "ERROR: not function 'initialization'\n");
			return -1;
		}
		int (*vpiFun)(VDB*) ;
		vpiFun = (int (*)(VDB*))dlsym( handle, "VpiMain" );	
		if( vpiFun) {
			int inst_data;
			
			VDB vdb;
			LibVersion_t lv;

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

	float discret = 5 * 0.001;
	printf( "discret=%.3f\n", discret);
	srand((unsigned int)time(NULL));
    float a = 100.0;

	while( 1) {

//        printf("%f\n", ((float)rand()/(float)(RAND_MAX)) * a);
//		int weight = rand() % 100;
		float weight = ((float)rand()/(float)(RAND_MAX)) * a;
		float d = fmod(weight, discret);

		float weight1 = weight - d;

    	if ( (d > 0) && (d >= (discret / 2)))
		{
        	weight1 += discret;
		}
		
		printf( "%.3f d=%.3f %.3f %.3f\n", weight, d, weight1, round(weight1));
		char c = getchar();
		
		if( c == '\n')
			break;
	}
	
	int min=0, max = 20000;
	int cnt_min=314, cnt_max=56700;
	float k = float(max-min)/float(cnt_max-cnt_min);
	printf( "%d, %d, k=%f\n", cnt_max, max,k);
	while( 1) {
		int cnt = rand()/cnt_max + cnt_min;
		printf( "%d     %3f\n", cnt, cnt*k);
		getchar();
	}
	return 0;
}

