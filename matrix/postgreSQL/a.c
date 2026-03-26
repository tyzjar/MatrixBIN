#include "interface_scales.h"
#include <dlfcn.h>
#include <stdio.h>
#include <malloc.h>


int main( int argc, char* argv[])
{
	void* handle;
	handle = dlopen( "./libpostgreSQL_scale_db.so", RTLD_NOW );
	if( handle) {
		printf( "dlopen() OK!\n");
		
		int (*vpiFun)(VDB*) ;
		vpiFun = dlsym( handle, "VpiMain" );	
		if( vpiFun) {
			InstanceData inst_data;
			inst_data.conn = NULL;
			inst_data.str_error = NULL;
			
			VDB vdb;
			LibVersion lv;

			printf( "vpiFun OK!\n");
			
			vdb.pInstanceData = (void*)&inst_data;
			
			vdb.function = VPI_LIB_VERSION;
			vdb.pParam = &lv;
			int irc = (*vpiFun)(&vdb);
			printf( "irc=%d, libVersion=%d\n", irc, lv.libVersion);
			
			
			Connect connect;
//			connect.strConnect = "host=localhost user=janbodnar dbname=testdb password=pswd37";	//7";
			connect.strConnect = "host=localhost user=kemek dbname=scaledb password=kemek";
			vdb.function = VPI_CONNECT_TO_DB;
			vdb.pParam = &connect;
			irc = (*vpiFun)(&vdb);
			printf( "irc=%d, Connect=%p\n", irc, inst_data.conn);
			if( (irc == SCALES_CONNECT_ERROR) && inst_data.str_error) {
				printf( "Connect error: %s\n", inst_data.str_error);
				free( inst_data.str_error);
				inst_data.str_error = NULL;
			}
			

			ServerVersion sv;
			vdb.function = VPI_SERVER_VERSION;
			vdb.pParam = &sv;
			irc = (*vpiFun)(&vdb);
			printf( "irc=%d, serverVersion=%d\n", irc, sv.serverVersion);
			
			struct tm	start_time;
			struct tm	finish_time;
			time_t t_time;

	
			time(&t_time);
			localtime_r(&t_time, &finish_time);

			t_time -= 3600;
			localtime_r(&t_time, &start_time);
						
			Train train;
			Car car[2];
			
			vdb.function = VPI_WRITE_TRAIN;
			vdb.pParam = &train;
			train.num = 5;
			train.mode = MODE_MOTION;
			train.mode_auto = MODE_MOTION_AUTO;
			train.direct[0] = 'L'; train.direct[1] = 'R';
			train.start_weighing = &start_time;
			train.finish_weighing = &finish_time;
			train.type_weighing = 'B';
			train.cnt_car = 2;
			train.weight = 130000;
			train.result = 0;
			train.error = "";
/*			
CREATE TABLE IF NOT EXISTS car (
	train_id integer REFERENCES train(id),	-- id поезда (вторичный ключ)
	num	SMALLINT,							-- номер вагона
	axis SMALLINT,							-- кол-во осей
	weight INTEGER,							-- вес вагона
	speed	REAL,							-- скорость при взвешивании в движении
	result	SMALLINT,						-- результат взвешивания
	error TEXT,								-- текст ошибки

*/
			car[0].num = 1;
			car[0].car_type = '4';
			car[0].weight = 50000;
			car[0].speed = 3.1;
			car[0].result = 0;
			car[0].error = "";

			car[1].num = 2;
			car[1].car_type = '6';
			car[1].weight = 80000;
			car[1].speed = 3.0;
			car[1].result = 0;
			car[1].error = "";
			
			train.cars = &car[0];
			
			irc = (*vpiFun)(&vdb);
			if( (irc != SCALES_OK) && inst_data.str_error) {
				printf( "writeTrain: irc=%d, error: %s\n", irc, inst_data.str_error);
				free( inst_data.str_error);
			}
			
			
			vdb.function = VPI_CONNECT_CLOSE;
			irc = (*vpiFun)(&vdb);
			printf( "irc=%d, ConnectClose=%p\n", irc, inst_data.conn);
			
			
			
		}	
		else {
			printf( "Error: dlsym()\n");
		}
		dlclose( handle);
	}
	else
		printf( "Error: dlopen()\n");
	return 0;
}

