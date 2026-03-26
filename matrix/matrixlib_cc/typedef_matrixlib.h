#ifndef TYPEDEF_MATRIXLIB_H
#define TYPEDEF_MATRIXLIB_H

#include <time.h>
#include <vector>

#pragma pack(1)

#define FL_CHANNEL_RAW_DATA					0x0001		// сырые данные
#define FL_CHANNEL_IN_MOTION				0x0002		// канал участвует в взвешивании В ДВИЖЕНИИ
#define FL_CHANNEL_AVERAG_VALUE				0x0004

#define FL_SYSTEM_CONFIGURATION_READY		0x0001
#define FL_SYSTEM_SERVICE_MODE				0x0002

typedef struct {
	int		id_section;					// идентификатор секции
	int		live_rail;					// взвешиваемы участок
	int		dead_rail;					// невзвешиваемый участок
} Section_t;

// Платформа
typedef struct {
	int	  id_platform;				// идентификатор платформы
	char* description;					// описание
	std::vector<Section_t>	sections;	// секции
} Platform_t;

// Линейный участок
typedef struct {
	int	raw_value;					// "сырое" значение
	int	weight_value;				// значение веса
	float ratio;					// коэффициент линеаризации с начала данного участка до следующего
} LinearSection_t;

typedef struct {
	int	averag_time;				// число секунд на усреднение значения при калибровке канала
	float averag_weight_value;		// усредненное значение веса
	int averag_raw_value;			// усредненное значение сырых значений (counters)
	time_t	start_time;				// начало времени усреднения
} AveragValue_t;

typedef struct {
	int id_platform;
	int id_section;
} PlatformSection_t;

typedef struct {
	char* name;				// название датчика
	char* type;				// аналоговый/цифровой
	int	  count;			// кол-во датчиков
} Sensor_t;

// Измерительный канал
typedef struct {
	int	channel;					// идентификатор канала >0
	char*	description;			// описание
	int	com_port;					// номер com порта
	int device_type;				// тип обрабатывающего устройства (индекс)
	Sensor_t	sensors;			// описание датчиков, подключенных к обрабатывающему устройству
	
	unsigned flag_channel;
	int id_scale;					// идентификатор весов, к которым канал подключен 
//-- metrology	
	int max_weight_value;			// НПИ( наибольший предел измерений)
	int zero_calibration_value;		// RAW значение zero_calibration (K)
	
	int diff_zero_value;			// RAW (zero_calibration_value - zero_correction_value)
	AveragValue_t	averag_value;
	std::vector<LinearSection_t> linearSections; 	// линейные участки
	std::vector<PlatformSection_t> pluginSections;	// подключенные к каналу платформа:секция
} Channel_t;

typedef struct {
	int		 value;					// дискрет из [ 1, 2, 5]
	float	 multiplier;			// дискрет из [ 0.001, 0.01, 0.1, 1, 10, 100, 1000]
} DiscretChannel_t;

typedef struct {
	int	max_weight;
	int e;
} Span_t;

#define	ZERO_CURRENT		1
#define	ZERO_POWER_OFF		2
#define	ZERO_CALIBRATION	3

typedef struct {
	int	active;					// 1 - активна (возможна операция по нажатию кнопки), 0 - не активна
	int	percent_max;			// процент от максимальной нагрузки
	int	add_coef;				// добавочный коэффифиент
} ZeroButton_t;

typedef struct {
	float diff_center_d;		// доля дискрета весов
	int	  underload_d;			// значение для показа недогруза
} ZeroIndication_t;

typedef struct {
	int type_zero;				// тип установки нуля при включении
	int	auto_tracking_d;		// дипазон слежения (0 - нет слежения)
	int	auto_tracking_timeout;	// периодичность автоматического обнуления в секундах (0 - запрещено)
	int	percent_max;			// процент от максимальной нагрузки
	int	add_coef;				// добавочный коэффифиент
	ZeroButton_t		zero_button;
	ZeroIndication_t 	zero_indication;
} Zero_t;
	
// возможные режимы взвешивания mode_work
#define	SCALE_STATIC_MODE		1
#define	SCALE_MOTION_MODE		2

typedef struct {
	char* description;
	
} ModeWeithing_t;

// Конфигурация весов
typedef struct {
	int		id_scale;			// идентификатор весов (1..N)
	char* 	description_static;	// описание весов в статическом взвешивании
	char* 	description_motion;	// описание весов в взвешивании в движении
	int		flag_mode_work;		// возможные режимы работ весов (СТАТИКА, ДВИЖЕНИЕ)
	int		maximum_weight;		// максимальная нагрузка для весов
	int 	minimum_weight;		// минимальная нагрузка для весов
	int		idx_unit;			// индекс из массива строк единиц измерений

	std::vector<int> 	  	  id_channel;	
	DiscretChannel_t		  discretChannel;	// дискреты на каналы 
	Zero_t					  zero;				// операции с нулем
	ModeWeithing_t	mode_static;
	ModeWeithing_t	mode_motion;
	
} ScaleConfiguration_t;

// ГПУ
typedef struct {
	char *description;
	std::vector<Platform_t>	platforms;
} LoadReceptor_t;

// Системная конфигурация
typedef struct {
	char* fileConfiguration;			// файл конфигурации, по которой построена данная конфигурация
	char* json_config;					// объект, вычитанный из файла
	char* version;						// версия конфигурации
	unsigned flag_system;			 	// флаги состояния работы весовой системой
	std::vector<char*> 			  	  units;				// строки с единицами измерения
	std::vector<char*> 			  	  dev_types;			// строки с названиями устройств (LDU,...)
	std::vector<Channel_t> 			  channelConfiguration;	// конфигурации весовых каналов
	std::vector<ScaleConfiguration_t> scaleConfiguration;	// конфигурации весовых систем
	LoadReceptor_t				 	  loadReceptor;			// все ГПУ в системе
} SystemConfiguration_t;

typedef struct {
	int channel;
	int zero_calibration_value;
} ZeroCalibration_t;

typedef struct {
	int channel;
	int zero_correction_value;
} ZeroCorrection_t;

typedef struct {
	int channel;
	int raw_value;
	int weight_value;
} GaneCalibration_t;

typedef struct {
	int channel;
	int raw_value;
	int raw_weight_value;
	float weight_value;
} CurMeasurement_t;

typedef struct {
	int channel;
	int max_weight_value;
} SetMaximumChannel_t;

typedef struct {
	int id_scale;
	int idx_unit;
} SwitchUnit_t;

typedef struct {
	int id_scale;
	int value;
	float multiplier;
} Discret_t;

typedef struct {
	int channel;
	int averag_time;
} AveragTime_t;

typedef struct {
	int id_scale;
	int idx_discret;
} SpanScale_t;

typedef struct {
	int id_scale;
	int maximum_weight;
} MaximumWeight_t;

typedef struct {
	int id_scale;
	int minimum_weight;
} MinimumWeight_t;

typedef struct {
	int id_scale;
} AddScale_t;

typedef struct {
	int id_scale;
	int auto_tracking_d;
	int auto_tracking_timeout;
} ZeroTracking_t;

typedef struct {
	int id_scale;
	int type_zero;				// тип установки нуля при включении
	int	percent_max;			// процент от максимальной нагрузки
	int	add_coef;				// добавочный коэффифиент
} ZeroSet_t;

typedef struct {
	int id_scale;
	int active;					// активна ли кнопка обнуления весов
	int	percent_max;			// процент от максимальной нагрузки
	int	add_coef;				// добавочный коэффифиент
} ZeroButtonSet_t;

typedef struct {
	int	id_scale;
	float diff_center_d;		// доля дискрета весов
	int	  underload_d;			// значение для показа недогруза
} ZeroIndicationSet_t;

typedef struct  {
	char* file_configuration;
} LoadConfiguration_t;  

typedef struct  {
	char* szLibVersion;
} LibVersion_t;  

#pragma pack()

#endif
