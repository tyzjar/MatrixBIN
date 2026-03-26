#ifndef INTERFACE_SCALES_H
#define INTERFACE_SCALES_H

#include <time.h>

#pragma pack(1)

// return
#define	SCALES_OK							0
#define	SCALES_RECOVERY_ERROR				-1
#define	SCALES_PARAM_ERROR					-2
#define	SCALES_CONNECT_ERROR				-3
#define	SCALES_TRANSACTION_BEGIN_ERROR		-4
#define	SCALES_TRANSACTION_COMMIT_ERROR		-5
#define	SCALES_EXECUTE_ERROR				-6




// VPI functions
#define VPI_LIB_VERSION         1
#define VPI_CONNECT_TO_DB      	2
#define VPI_CONNECT_CLOSE       3
#define VPI_SERVER_VERSION      4
#define VPI_WRITE_TRAIN         5
#define VPI_READ_BLOCK          6
#define VPI_WRITE_GATE          7
#define VPI_CONNECT_NODE        8
#define VPI_RCS_QUERY           9
#define VPI_WRITE_BLOCK         10
#define VPI_SHUTDOWN            11
#define VPI_CLOSE_COM           12
#define VPI_REOPEN_COM          13
#define VPI_GET_INFO            14
#define VPI_CHANGE_PARAMS       15
#define VPI_BROWSE_ADDRESS      16              //OPC


#define	MODE_STATIC	'S'
#define	MODE_MOTION	'M'

#define	MODE_MOTION_AUTO		'A'
#define	MODE_MOTION_SEMIAUTO	'S'

// VDB - Vpi Data Block
typedef struct
{
  void     *pInstanceData;         // Pointer to per instance vpi data
  int      function;               // Function number called
  int      vpiNumber;              // Vpi number
  int      parmSize;               // Size of additional function data
  void     *pParam;                // Pointer to additional function data
} VDB;

typedef struct {
	void *conn;
	char* str_error;
} InstanceData;

typedef struct  {
	int	libVersion;
} LibVersion;

typedef struct  {
	char* strConnect;
} Connect;

typedef struct  {
	int	serverVersion;
} ServerVersion;

/*
	num INTEGER,				-- номер поезда
	mode VARCHAR(1),			-- в статике/в движении S/M
	mode_auto VARCHAR(1),		-- в движении, автомат/полуавтомат A/S
	direct VARCHAR(2),			-- в движении, направление движения LR/RL
	start_weighing	TIMESTAMP,	-- начало взвешивания
	finish_weighing TIMESTAMP,	-- окончание взвешивания
	type_weighing VARCHAR(1),	-- тип взвешивания, тара/брутто Т/Б
	cnt_car	SMALLINT,			-- кол-во вагонов
	weight INTEGER,				-- общий вес состава
	result SMALLINT,			-- результат взвешивани: ок или номер ошибки
	error TEXT					-- текст ошибки
);

CREATE TABLE IF NOT EXISTS car (
	train_id integer REFERENCES train(id),	-- id поезда (вторичный ключ)
	num	SMALLINT,							-- номер вагона
	axis SMALLINT,							-- кол-во осей
	weight INTEGER,							-- вес вагона
	speed	REAL,							-- скорость при взвешивании в движении
	result	SMALLINT,						-- результат взвешивания
	error TEXT,								-- текст ошибки

*/

typedef struct {
	int	num;
	char car_type;
	int	weight;
	float	speed;
	int	result;
	char* error;
} Car;

typedef struct  {
	int	num;
	char mode;					// S/M статика/движение
	char mode_auto;
	char direct[2];
	struct tm	*start_weighing;
	struct tm	*finish_weighing;
	char type_weighing;
	int	cnt_car	;
	int	weight;
	int result;
	char *error;
	Car	 *cars;
} Train;

int VpiMain(VDB *pVDB);

#pragma pack()

#endif


