#pragma once
#include	"..\interface.h"
#include	"h_SerialDevice.h"

class EEPROM_I2C: public I2CDevice {
protected:
	uint16_t   address;
const	uint16_t   addressMask;
	uint8_t*   rom;
	uint8_t    bit;
	uint8_t    latch;
public:
	           EEPROM_I2C(uint16_t, uint8_t, uint8_t*);
virtual	void       reset();
virtual void       receiveBit();
virtual	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};

class EEPROM_24C01: public EEPROM_I2C {
public:
	           EEPROM_24C01(uint8_t, uint8_t*);
	void       receiveBit();
};

class EEPROM_24C02: public EEPROM_I2C {
public:
	           EEPROM_24C02(uint8_t, uint8_t*);
};

class EEPROM_24C04: public EEPROM_I2C {
public:
	           EEPROM_24C04(uint8_t, uint8_t*);
	void       receiveBit();
};

class EEPROM_24C08: public EEPROM_I2C {
public:
	           EEPROM_24C08(uint8_t, uint8_t*);
	void       receiveBit();
};

class EEPROM_24C16: public EEPROM_I2C {
public:
	           EEPROM_24C16(uint8_t, uint8_t*);
	void       receiveBit();
};
