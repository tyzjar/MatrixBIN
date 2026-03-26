#ifndef TYPEDEF_MATRIXLIB_H
#define TYPEDEF_MATRIXLIB_H

#pragma pack(1)

#define FL_CHANNEL_RAW_DATA					0x0001

#define FL_SCALE_CONFIGURATION_READY		0x0001
#define FL_SCALE_METROLOGICAL_VERIFICATION	0x0002

// Линейный участок
typedef struct {
	int	raw_value;					// "сырое" значение
	int	weight_value;				// значение веса
} LinearSection_t;

// Измерительный канал
typedef struct {
	unsigned flag_channel;
	int	channel;					// идентификатор канала >0
	int zero_calibration_value;		// RAW значение zero_calibration (K)
	int zero_correction_value;		// RAW значение zero_correction (К)
	int diff_zero_value;			// RAW (zero_calibration_value - zero_correction_value)
	int cntLinearSections;			// кол-во линейных участков
	LinearSection_t* linearSections; // линейные участки
} Channel_t;

// Конфигурация весовой системы
typedef	struct {
	unsigned flag_scale;
	unsigned id_scale;
	int cnt_channel;
	Channel_t* channels;
} ScaleConfiguration_t;

// Полная конфигурация 
typedef struct {
	int cntScales;
	ScaleConfiguration_t* scaleConfiguration;
} Configuration_t;

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

typedef struct  {
	char* szLibVersion;
} LibVersion_t;  

#pragma pack()

#endif
