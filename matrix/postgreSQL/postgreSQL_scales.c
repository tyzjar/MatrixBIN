// gcc -o lib_version lib_version.c -I/usr/include/postgresql -lpq -std=c99

#include "interface_scales.h"
#include <libpq-fe.h>
#include <string.h>
#include <stdlib.h>

#define	MAX_STR_SQL	1024

int libVersion( VDB *pVDB);
int connectToDB( VDB *pVDB);
int connectClose( VDB *pVDB);
int serverVersion( VDB *pVDB);
int writeTrain( VDB *pVDB);

int (*VpiFunc[])(VDB *)	= 
{
		libVersion,
		connectToDB, 
        connectClose,
        serverVersion,
        writeTrain,
         0,				               // Read block of gates.
         0,					           // Write single gate.
	     0,
         0, //StationInfo,		// VPI_STATION_INFO (9) 14.12.2016. гЮОПНЯ МЮ БШБНД Б
							// КНЦ дкк ХМТН. Н ЯРЮМЖХЪУ (ЙНМРПНККЕПЮУ)
         0,					    // Write block of gates.
         0, //EndVpi,                 // VPI_SHUTDOWN (11) End vpi.
         0,						// Close communication & free device.
         0,						// Restart communication.
         0, //GetVpiInfo,             // VPI_GET_INFO (14) Get VPI information.
         0,						// Change VPI parameters.
		 0, //OpcBlockRead,			// VPI_READ_OPC (101) ВРЕМХЕ ОЕПЕЛЕММШУ Б ТНПЛЮРЕ OPC
		 0, //OpcWrite,				// VPI_WRITE_OPC (102) ГЮОХЯЭ ОЕПЕЛЕММШУ Б ТНПЛЮРЕ OPC
		 0, //OpcBrowse,				// VPI_BROWSE_OPC (103) БГЪРЭ ХЛЕМЮ ОЕПЕЛЕММШУ Б ТНПЛЮРЕ OPC
		 0, //GetOpcInfo,				// VPI_INFO_OPC (104) БГЪРЭ ХМТНПЛЮЖХЧ НА OPC
         0
};


/*********************************/
/* VPI main entry point          */
/*********************************/
int VpiMain(VDB *pVDB)
{
   int iRc,
   	   iFunc;

   iFunc = pVDB->function;

   /* Check if the function number received is legal */
   if ( iFunc > (int)(sizeof(VpiFunc) / sizeof(VpiFunc[0])) )
      return SCALES_RECOVERY_ERROR;

   if ( VpiFunc[iFunc-1] == 0 )
      return SCALES_RECOVERY_ERROR;

   iRc = ( *VpiFunc[iFunc-1] )(pVDB); /* call the required function */

   return iRc;

} //VpiMain()

/*
 * int libVersion( VDB *pVDB)
 */
int libVersion( VDB *pVDB)
{
	int iRc = SCALES_OK;
	LibVersion *p = (LibVersion *)pVDB->pParam;
	if( p)
    	p->libVersion = PQlibVersion();
    else
    	iRc = SCALES_PARAM_ERROR;
    	
    return iRc;
}

/*
 * int connectToDB( VDB *pVDB)
 */
int connectToDB( VDB *pVDB)
{
	int iRc = SCALES_OK;
	
	InstanceData* inst_data = (InstanceData*)pVDB->pInstanceData;
	Connect* connect = 	(Connect* )pVDB->pParam;
	
	if( inst_data && connect && connect->strConnect) {	

		inst_data->conn = (void*)PQconnectdb(connect->strConnect);
		
		if (PQstatus((PGconn *)inst_data->conn) == CONNECTION_BAD) {
			inst_data->str_error = strdup (PQerrorMessage((PGconn *)inst_data->conn));
			iRc = SCALES_CONNECT_ERROR;
			PQfinish((PGconn *)inst_data->conn);
			inst_data->conn = NULL;
		}
	}
    else {
    	iRc = SCALES_PARAM_ERROR;
    }
    	
	return iRc;	
}

/*
 * int connectClose( VDB *pVDB)
 */
int connectClose( VDB *pVDB)
{
	int iRc = SCALES_OK;
	InstanceData* inst_data = (InstanceData*)pVDB->pInstanceData;
	
	if( inst_data && inst_data->conn) {
		PQfinish((PGconn *)inst_data->conn);
		inst_data->conn = NULL;
	}
    else {
    	iRc = SCALES_PARAM_ERROR;
    }
	
	return iRc;	
}

/*
 * int serverVersion( VDB *pVDB)
 */
int serverVersion( VDB *pVDB)
{
	int iRc = SCALES_OK;
	ServerVersion *p = (ServerVersion *)pVDB->pParam;
	InstanceData* inst_data = (InstanceData*)pVDB->pInstanceData;
	
	if( p && inst_data && inst_data->conn)
    	p->serverVersion = PQserverVersion((PGconn *)inst_data->conn);
    else
    	iRc = SCALES_PARAM_ERROR;
    	
    return iRc;
}

/*
 * int writeTrain( VDB *pVDB)
 */
int writeTrain( VDB *pVDB)
{
	int iRc = SCALES_OK;
	Train *p = (Train *)pVDB->pParam;
	InstanceData* inst_data = (InstanceData*)pVDB->pInstanceData;
	PGconn *conn = (PGconn *)inst_data->conn;
	
	if( p && inst_data && inst_data->conn) {
		if (PQstatus( conn) == CONNECTION_BAD) {
			inst_data->str_error = strdup (PQerrorMessage(conn));
			iRc = SCALES_CONNECT_ERROR;
			return iRc;
		}
		PGresult *res = PQexec(conn, "BEGIN");    
    
   		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	        PQclear(res);
			inst_data->str_error = strdup (PQerrorMessage(conn));
			iRc = SCALES_TRANSACTION_BEGIN_ERROR;
			return iRc;
		}
		
    	PQclear(res); 

		char str_sql[MAX_STR_SQL];
		snprintf( str_sql, sizeof(str_sql), "INSERT INTO train (num,mode,mode_auto,direct,start_weighing,finish_weighing,type_weighing,cnt_car,weight,result,error) VALUES (%u,'%c','%c','%c%c',"
				"'%04u-%02u-%02u %02u:%02u:%02u','%04u-%02u-%02u %02u:%02u:%02u','%c',%u,%d,%d,'%s') RETURNING id",
			p->num,
			p->mode,
			p->mode_auto,
			p->direct[0],p->direct[1],
			p->start_weighing->tm_year + 1900,p->start_weighing->tm_mon + 1,p->start_weighing->tm_mday,p->start_weighing->tm_hour,p->start_weighing->tm_min,p->start_weighing->tm_sec,
			p->finish_weighing->tm_year + 1900,p->finish_weighing->tm_mon + 1,p->finish_weighing->tm_mday,p->finish_weighing->tm_hour,p->finish_weighing->tm_min,p->finish_weighing->tm_sec,
			p->type_weighing,
			p->cnt_car,
			p->weight,
			p->result,
			p->error ? p->error : ""
			);
			
    	res = PQexec(conn, str_sql);    
    	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        	PQclear(res);
			inst_data->str_error = strdup (PQerrorMessage(conn));
			iRc = SCALES_EXECUTE_ERROR;
        	return iRc;
    	}       
    	unsigned id_car = (unsigned)strtoul(PQgetvalue(res, 0, 0), NULL, 0);
    	printf( "id_car=%u\n", id_car);
    	
    	for( int i=0; i < p->cnt_car; ++i) {
			snprintf( str_sql, sizeof(str_sql), "INSERT INTO car (train_id,num,car_type,weight,speed,result,error) "
				"VALUES (%u,%u,'%c',%d,%3.1f,%d,'%s')",
				id_car,
				p->cars[i].num,
				p->cars[i].car_type,
				p->cars[i].weight,
				p->cars[i].speed,
				p->cars[i].result,
				p->cars[i].error ? p->cars[i].error : ""
				);
        	PQclear(res);
    		res = PQexec(conn, str_sql);    
   			if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	        	PQclear(res);
				inst_data->str_error = strdup (PQerrorMessage(conn));
				iRc = SCALES_EXECUTE_ERROR;
				return iRc;
			}
    	}
    	
   		res = PQexec(conn, "COMMIT"); 
    
    	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        	PQclear(res);
			inst_data->str_error = strdup (PQerrorMessage(conn));
			iRc = SCALES_TRANSACTION_COMMIT_ERROR;
			return iRc;
    	}       
    
    }

    else
    	iRc = SCALES_PARAM_ERROR;
    	
    return iRc;
}

