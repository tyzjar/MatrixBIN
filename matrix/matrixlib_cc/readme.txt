Последовательность обработки весовых данных

НЕОБХОДИМОЕ УСЛОВИЕ: Загружена правильная конфигурация системы

#define FL_CHANNEL_DATA_RAW			0x0000
#define FL_CHANNEL_DATA_WEIGHT		0x0001
#define FL_CHANNEL_LINEARIZATION	0x0002

typedef struct {
	int	min_raw_data;
	int min_weight_data;
	int max_raw_data;
	int max_weight_data;
} linearization_section_t;

typedef struct {
	unsigned flagChannel;
	int cntLinearizationSections;	// число участков линеаризации
	linearization_section_t	*linearization_section;
	int zero_correction_value;		// новая точка скорректированного нуля
} channel_t;

Получили весовой пакет из devdrvd по каналу N.

1) Если по каналу N работаем в режиме получения первичных данных, то преобразуем первичные данные в весовые,
   используя параметры линеаризации.

С этого момента работаем только с весовыми данными.    
