#ifndef __MEASURING_CHANNEL_HPP__
#define __MEASURING_CHANNEL_HPP__ 

typedef struct {
	unsigned idPlatform;
	unsigned idSection;
} IdPlatformSection_t;

enum {
	DIGITAL_SENSOR=0,
	ANALOG_SENSOR
} TypeSensor_e;

class MeasuringChannel {
	private:
		unsigned id;
		unsigned numComPort;									// номер COM порта
		std::string processingDevice;							// устройство обработки
		std::string weighingSensor;								// весоизмерительный датчик
		TypeSensor_e typeWeighingSensor;						// тип датчика 
		unsigned cntWeighingSensor;								// кол-во весотзмерительных датчиков
	
		std::vector<IdPlatformSection_t> idPlatformSection;		// id платформ и секций в них, к которым подсоединен
																// этот канал
	public:
		MeasuringChannel(unsigned);
		~MeasuringChannel();
};
#endif