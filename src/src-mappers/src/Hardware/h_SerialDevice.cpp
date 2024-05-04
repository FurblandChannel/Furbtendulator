#include	"h_SerialDevice.h"

// Generalized serial device
SerialDevice::SerialDevice():
	state(0),
	clock(true),
	data(true),
	output(true) {
}

void SerialDevice::reset() {
	state =0;
	clock =true;
	data =true;
	output =true;
}

bool SerialDevice::getData() const {
	return output;
}

void SerialDevice::setPins(bool select, bool newClock, bool newData) {
	if (!select) {
		clock =newClock;
		data =newData;
	}
}

int MAPINT SerialDevice::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, state);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, clock);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, data);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, output);
	return stateOffset;
}

// General I²C serial device
I2CDevice::I2CDevice(uint8_t _deviceType, uint8_t _deviceAddr):
	SerialDevice::SerialDevice(),
	deviceType(_deviceType),
	deviceAddr(_deviceAddr),
	readMode(false) {
}

void I2CDevice::reset() {
	SerialDevice::reset();
	readMode =false;
}

void I2CDevice::setPins(bool, bool newClock, bool newData) {
	if (clock && newClock && data && !newData) // I²C start
		state =1;
	else
	if (clock && newClock && !data && newData) // I²C stop
		state =0;
	else
	if (clock && !newClock)
		receiveBit();
	
	clock =newClock;
	data =newData;
}

void I2CDevice::receiveBit() {
	switch(state) {
		case 1:                                  ++state   ; break; // Start condition
		case 2: state =!!(deviceType &8) ==data? ++state: 0; break; // 1st bit of device type
		case 3: state =!!(deviceType &4) ==data? ++state: 0; break; // 2nd bit of device type
		case 4: state =!!(deviceType &2) ==data? ++state: 0; break; // 3rd bit of device type
		case 5: state =!!(deviceType &1) ==data? ++state: 0; break; // 4th bit of device type
		case 6: state =!!(deviceAddr &4) ==data? ++state: 0; break; // 1st bit of device address
		case 7: state =!!(deviceAddr &2) ==data? ++state: 0; break; // 2nd bit of device address
		case 8: state =!!(deviceAddr &1) ==data? ++state: 0; break; // 3rd bit of device address
		case 9: readMode =data;                  ++state   ; break; // Read/write
	}
	// States 10+ and output line must be defined by derived class specifying a particular I²C device.
}

int MAPINT I2CDevice::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	stateOffset =SerialDevice::saveLoad(stateMode, stateOffset, stateData);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, readMode);
	return stateOffset;
}

