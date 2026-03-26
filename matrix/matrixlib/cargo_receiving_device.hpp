#ifndef __CARGO_RECIEIVING_DEVICE_HPP__
#define __CARGO_RECIEIVING_DEVICE_HPP__  

#include "platform.hpp"

class CargoReceivingDevice {
	private:
		unsigned id;
		std::vector<Platform> platforms;
	public:
		CargoReceivingDevice(unsigned);
		~CargoReceivingDevice();
};
#endif
