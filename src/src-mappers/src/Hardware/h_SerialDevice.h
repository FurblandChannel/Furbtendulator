#pragma once
#include	"..\interface.h"

class SerialDevice {
protected:
	uint8_t    state;
	bool       clock;
	bool       data;
	bool       output;
public:
	           SerialDevice();
virtual	void       reset();
virtual	bool       getData() const;
virtual	void       setPins(bool, bool, bool);
virtual	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};

class I2CDevice: public SerialDevice {
protected:
const   uint8_t    deviceType;
const   uint8_t    deviceAddr;
	bool       readMode;
public:
	           I2CDevice(uint8_t, uint8_t);
virtual	void       reset();
virtual	void       setPins(bool, bool, bool);
virtual	void       receiveBit();
virtual	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};

