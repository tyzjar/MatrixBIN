#ifndef INTERFACE_MATRIXLIB_H
#define INTERFACE_MATRIXLIB_H

#pragma pack(1)

#define	MATRIXLIB_MAJOR_VERSION				1
#define	MATRIXLIB_MINOR_VERSION				0
#define	MATRIXLIB_MODIFICATION_VERSION		0
#define	MATRIXLIB_VERSION_STR				"%u.%u.%u"

/* return */
#define	MATRIXLIB_OK										0
#define	MATRIXLIB_ERROR										-1
#define	MATRIXLIB_PARAM_ERROR								-2
#define	MATRIXLIB_RECOVERY_FUNCTION_ERROR					-3 
#define	MATRIXLIB_EXECUTE_ERROR								-4
#define	MATRIXLIB_SYSTEM_NOT_CONFIGURATION					-5 
#define	MATRIXLIB_SYSTEM_NO_IN_SERVICE_MODE					-6
#define	MATRIXLIB_EXECUTION_SEQUENCE_ERROR					-7
#define	MATRIXLIB_LINEARITAZION_ERROR						-8		// ошибка линеаризации 
#define	MATRIXLIB_CHANNEL_ERROR								-9		// неверный номер канала
#define	MATRIXLIB_SCALE_ERROR								-10		// неверный номер весов
#define	MATRIXLIB_CONFIG_OPEN_ERROR							-11		// ошибка открытия файла конфигурации
#define	MATRIXLIB_ALLOC_ERROR								-12		// ошибка выделения памяти
#define	MATRIXLIB_CONFIG_ERROR								-13		// ошибка в json конфигурации


/* VPI functions */

#define VPI_LIB_VERSION         		101		// Сообщить номер версии библиотеки
#define VPI_GET_TAC		        		102		// CE 	(М) Сообщить значение несбрасывваемого счетчика
#define VPI_LOAD_CONFIGURATION			103		// Загрузить настройки
#define VPI_SAVE_CONFIGURATION			104		// Сохранить настройки

/* Взвешивание в СТАТИКЕ */
#define	VPI_SET_MAXIMUM					105		// CM 	(M) Максимальная нагрузка(Max) 
#define	VPI_SET_MINIMUM					106		// CI 	{M} Минимальная нагрузка (Min)
#define	VPI_SET_RANGE_MODE				107		// MR 	{M) Установить многоинтервальный или многодиапазонный режим
#define	VPI_SET_INCREMENT_SIZE			108		// DS 	(M) установка цены деления шкалы (дискретность) для каждого диапазона/интервала измерений. 
#define	VPI_SET_DEC_POINT				109		// DP 	(M) установка позиции десятичной точки для каждого диапазона/интервала измерений.
#define	VPI_SET_CALIBRATE_ZERO			110		// CZ 	(M) калибровать НУЛЬ
#define	VPI_SET_CALIBRATE_GANE			111		// CG 	(M) калибровать нагрузку
#define	VPI_SET_CALIBRATE_ZERO_CORRECTION	112	// IZ 	(M) коррекция нуля
#define	VPI_SET_LINEARIZATION			113		//    	(М) Установить точку линеаризации (точек линеаризации может быть несколько, 
												// их необходимо будет отсортировать по возрастанию значения массы, при которой делали подстройку 
#define	VPI_SAVE_CALIBRATION			114		// CS 	(М) Сохранить калибровочные параметры
#define	VPI_SET_WARM_UP_TIME			115		// 		(М) WT Установить время прогрева весов

#define	VPI_ZERO_TRACKING				120		// ZT 	(M) Слежение за нулем
#define	VPI_SET_ZERO_MANUAL_RANGE		121		// ZR 	{M} Установить диапазон обнуления по кнопке "НОЛЬ"
#define	VPI_SET_ZERO_INITIAL_RANGE		122		// ZI 	(М) Установить диапазон обнуления при включении питания (PowerUP ZERO)

#define	VPI_ZERO_SET					123		// SZ 	(М) установить текущий (системный нуль)
#define	VPI_ZERO_RESET					124		// RZ 	(М) сброс текущего нуля (возврат к калибровочному)
#define	VPI_ZERO_SAVE					125		// Сохранить текущий НУЛЬ в файл для последующего восстановления после сбоя по питанию
#define	VPI_ZERO_RESTORE				126		// Восстановить текущий ноль после перезапуска контроллера

#define VPI_SET_MAXIMUM_CHANNEL			127		// Установить наибольший предел измерений (НПИ) канала
#define VPI_SET_DISCRET					128		// Установить значение [ 1, 2, 5] [ 0.001, 0.01, 0.1, 1, 10, 100, 1000]
#define VPI_SET_AVERAG_TIME				129		// Установить число секунд для усреднения значений при калибровке
#define VPI_SET_ZERO_INDICATION			130		// Установить пороговое значение НЕДОГРУЗ

#define	VPI_TARE_MODE					140		// TM 	(М) 
#define	VPI_TARE_SET					141		// ST	(M) Установить тару (оттарировать) и переключиться в режим индикации массы НЕТТО
#define	VPI_TARE_RESET					142		// RT	(M) Очистить тару и переключиться в режим индикации массы БРУТТО
#define	VPI_TARE_PRESET					143		// 		(M) Установить массу тары, заданную с клавиатуры	или сохраненную в файле	 
#define	VPI_TARE_SAVE					144		// 		(M) Сохранить массу тару в файл (для последующего восстановления после возможного сбоя питания 		 
#define	VPI_TARE_RESTORE				145		// 		(M) Установить массу тары из файла после перезапуска контроллера
#define	VPI_TARE_AUTOSET				146		// 		(M) Автоматическое тарирование после превышения порогового веса (ожидает стабильный сигнал)
#define	VPI_TARE_AUTOCLEAR				147		// 		(M) Автоматическая очистка массы тары если сигнал опустился ниже порогового (стабильный сигнал можно не ждать)
												// 			Пороговое значение массы должно быть меньше, чем у функции автоматического тарирования

#define	VPI_PROCESSING_CUR_MEASUREMENT	160		// 		(M) Обработка приходящего измеренного значения 
#define	VPI_FILTERING_PERFORM			161		// 		(М) Фильтровать фходные данные
#define	VPI_CALCULATE_WEIGHT			162		// 		(М) Рассчитать вес с учетом линеаризации по каналу
#define	VPI_NORMALIZE_WEIGHT			163		// 		(М) Округление веса с учетом текущей цены деления шкалы (дискретности) и диапазона/интервала измерений
												//          для весов
#define	VPI_SWITCH_UNIT					164		// 		(М) Переключить единицы измерения
#define	VPI_IS_MOTION					165		// 		(М) Детектор стабильности сигнала

#define	VPI_MODE_X10					166		// 		(М) X10 Включить индикацию с повышенной точностью (добавляется еще один разряд слева)
#define	VPI_SET_CHANEL_ON_OFF			167		// 		(М) Включение отключение измерительных каналов

#define	VPI_PRINT						180		// 		(М) Печать текущего измеренного значения (ожидает стабильный сигнал)
#define	VPI_SAVE_RESULTS				181		// 		(М) Сохранить текущие данные при статическом взвешивании в файл или БД


/* Взвешивание в ДВИЖЕНИИ */
#define	VPI_SET_WIM_MAX_CAPACITY		200		// 		(М) Установить максимальную нагрузку (Maximum Capacity) - максимальная масса вагона/автомобиля при взвешивании в ДВИЖЕНИИ
#define	VPI_SET_WIM_MIN_CAPACITY		201		// 		(М) Установить минимальную нагрузку (Minimum Capacity) - 

#define	VPI_SET_WIM_MAX_PLATFORM		202		// 		(М) Установить максимальную нагрузку на платформу(Max-п)
#define	VPI_SET_WIM_MIN_PLATFORM		203		// 		(М) Установить минимальную нагрузку на платформу (Min-п) 

#define	VPI_SET_WIM_MAX_WAGON_MASS		204		// 		(М) Установить максимальную  массу вагона
#define	VPI_SET_WIM_MIN_WAGON_MASS		205		// 		(М) Установить минимальную массу вагона

#define	VPI_SET_WIM_MAX_TRAIN_MASS		206		// 		(М) Установить максимальную  массу состава
#define	VPI_SET_WIM_MIN_TRAIN_MASS		207		// 		(М) Установить минимальную  массу состава

#define	VPI_SET_WIM_MAX_SPEED			208		// 		(М) Установить максимальную (Vmax) скорость движения при взвешивании
#define	VPI_SET_WIM_MIN_SPEED			209		// 		(М) Установить минимальную (Vmin) скорость движения при взвешивании
#define	VPI_SET_WIM_MAX_TRANSIT_SPEED	210		// 		(М) Установить максимальную транзитную скорость движения состава без взвешивания без ухудшения метрологии 

#define	VPI_SET_WIM_ADJUSTMENT			220		// 		(М) Установить параметры подстройки в зависимости от направления движения и позиции локомотива (тянет/толкает)
	
#define	VPI_WIM_CALCULATE_SPEED			211		// 		(М) Расчитать скорость движения
#define	VPI_WIM_SOLVER					212		// 		(М) Расчет массы вагонов

#define	VPI_WIM_PRINT					220		// 		(М) Печать результатов взвешивания в движении
#define	VPI_WIM_SAVE_RESULTS			221		// 		(М) Сохранить результаты взвешивания в движении в файл или БД

#define	VPI_SWITCH_MODE					400		// Переключить режим работы (СТАТИКА      
#define	VPI_ADD_SCALE					401		// Добавить весы в систему      
#define	VPI_RESET_TO_FACTORY			500		// FD (M) сброс к заводским установкам

#define	VPI_MAX_FUNCTION				1000

/* LDU
 * CE - возвращает счетчик TAC
 * CM 1, CM 2, CM 3 - возвращают максимумы диапазонов
 * CI -  возвращает минимальное значение (недогруз)
 * MR -  возвращает режим работы
 * DS -  возвращает цену деления шкалы
 * DP -  возвращает положение десятичной точки
 * CG -  возвращает значение калибровочного веса
 * ZT -	 возвращает признак слежения за нулем
 * ZR -  возвращает диапазон полуавтоматического обнуления (в ручную)
 * ZI -	 возвращает диапазон автоматического обнуления
 * WT -  возвращает кол-во секунд, в течение котрых будет показан недогруз, после включения
 * TM -  возвращает режим тарирования
 */

typedef int (*fun_add)( void*); 

// VDB - Vpi Data Block
typedef struct
{
  void     *pInstanceData;         // Pointer to per instance vpi data
  int      function;               // Function number called
  int      vpiNumber;              // Vpi number
  int      paramSize;               // Size of additional function data
  void     *pParam;                // Pointer to additional function data
  fun_add  fun;						// Alternative function
} VDB;  

extern "C" {
	
void initialization();	 
int VpiMain(VDB *pVDB);

int libVersion(VDB *pVDB);
int getTAC(VDB *pVDB);
int loadConfiguration( VDB *pVDB);
int saveConfiguration( VDB *pVDB);

int setMaximum( VDB *pVDB);
int setMinimum( VDB *pVDB);
int setRangeMode( VDB *pVDB);
int setIncrementSize( VDB *pVDB);
int setDecPoint( VDB *pVDB);
int setCalibrateZero( VDB *pVDB);
int setCalibrateGane( VDB *pVDB);
int setCalibrateZeroCorrection( VDB *pVDB);
int setLinearization( VDB *pVDB);
int saveCalibration( VDB *pVDB);
int setWarmUpTime( VDB *pVDB);
int zeroTracking( VDB *pVDB);
int setZeroManualRange( VDB *pVDB);
int setZeroInitialRange( VDB *pVDB);

int zeroSet( VDB *pVDB);
int zeroReset( VDB *pVDB);
int zeroSave( VDB *pVDB);
int zeroRestore( VDB *pVDB);

int setMaximumChannel( VDB *pVDB);
int setDiscret( VDB *pVDB);
int setAveragTime( VDB *pVDB);
int	setZeroIndication( VDB *pVDB);
int tareMode( VDB *pVDB);
int tareSet( VDB *pVDB);
int tareReset( VDB *pVDB);
int tarePreset( VDB *pVDB);
int tareSave( VDB *pVDB);
int tareRestore( VDB *pVDB);
int tareAutoSet( VDB *pVDB);
int tareAutoClear( VDB *pVDB);

int processingCurMeasurement( VDB *pVDB);
int filteringPerform( VDB *pVDB);
int calculateWeight( VDB *pVDB);
int normalizeWeight( VDB *pVDB);
int switchUnit( VDB *pVDB);
int isMotion( VDB *pVDB);
int modeX10( VDB *pVDB);
int setChannelOnOff( VDB *pVDB);

int print( VDB *pVDB);
int saveResults( VDB *pVDB);

int setWimMaxCapacity( VDB *pVDB);
int setWimMinCapacity( VDB *pVDB);
int setWimMaxPlatform( VDB *pVDB);
int setWimMinPlatform( VDB *pVDB);
int setWimMaxWagonMass( VDB *pVDB);
int setWimMinWagonMass( VDB *pVDB);
int setWimMaxTrainMass( VDB *pVDB);
int setWimMinTrainMass( VDB *pVDB);
int setWimMaxSpeed( VDB *pVDB);
int setWimMinSpeed( VDB *pVDB);
int setWimMaxTransitSpeed( VDB *pVDB);

int setWimAdjusment( VDB *pVDB);
int wimCalculateSpeed( VDB *pVDB);
int wimSolver( VDB *pVDB);
int wimPrint( VDB *pVDB);
int wimSaveResults( VDB *pVDB);
	
int	AddScale( VDB *pVDB);
int resetToFactory( VDB *pVDB);
}

#pragma pack()

#endif


