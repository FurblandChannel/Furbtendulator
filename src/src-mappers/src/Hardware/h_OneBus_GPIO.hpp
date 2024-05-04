#pragma once
#include	"..\interface.h"
#include 	"h_SerialDevice.h"

namespace OneBus {
class GPIO {
	struct AttachedSerialDevice {
		SerialDevice* device;
		uint8_t       select;
		uint8_t       clock;
		uint8_t       data;
	};
	uint8_t mask; // clear bit=read, set bit=write
	uint8_t latch;
	uint8_t state;
	std::vector<AttachedSerialDevice> serialDevices;
public:
	        GPIO();
	uint8_t read(uint8_t);
	void    write(uint8_t, uint8_t);
	void    attachSerialDevice(AttachedSerialDevice);
	void    reset();
};

class SerialROM: public SerialDevice {
	int      bitPosition;
	uint8_t  command;
const	uint8_t* rom;
public:
	         SerialROM(const uint8_t*);
	void     setPins(bool, bool, bool);
};

class InverterROM: public SerialDevice {
	uint8_t  command;
     	uint8_t  result;
public:
	void     setPins(bool, bool, bool);
};

} // namespace OneBus