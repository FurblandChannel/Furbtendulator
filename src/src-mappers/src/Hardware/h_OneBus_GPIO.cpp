#include "h_OneBus_GPIO.hpp"

namespace OneBus {

GPIO::GPIO():
	mask(0),
	latch(0),
	state(0xFF) {
}
uint8_t GPIO::read(uint8_t address) {
	switch(address &7) {
		case 0:	return mask;
		case 2: return latch;
		case 3: state =~mask;
			for (auto& d: serialDevices) state &=d.device->getData() <<d.data | ~(1 <<d.data);
			return state;
		default:return 0xFF;
	}
}
void    GPIO::write(uint8_t address, uint8_t value) {
	switch(address &7) {
		case 0:	mask =value;
			state =state &~mask | latch &mask;
			for (auto& d: serialDevices) d.device->setPins(state >>d.select &1, state >>d.clock &1, state >>d.data &1);
			break;
		case 2:
		case 3:	latch =value;
			state =state &~mask | latch &mask;
			for (auto& d: serialDevices) d.device->setPins(state >>d.select &1, state >>d.clock &1, state >>d.data &1);
			break;
	}
}
void    GPIO::attachSerialDevice(AttachedSerialDevice device) {
	serialDevices.push_back(device);
}

void    GPIO::reset() {
	mask =0;
	state =0xFF;
	serialDevices.clear();
}

SerialROM::SerialROM(const uint8_t* _rom):
	bitPosition(0),
	command(0),
	rom(_rom) {
}
	
void SerialROM::setPins(bool select, bool newClock, bool newData) {
	if (select)
		state =0;
	else
	if (!clock && newClock) {
		if (state <8) {
			command =command <<1 | newData*1;
			if (++state ==8 && command !=0x30)
				state =0;
			else
				bitPosition =0;
		} else {
			output =rom[bitPosition >>3] >>(7 -(bitPosition &7)) &1? true: false;
			if (++bitPosition >=256 *8) state =0;
		}
	}
	clock =newClock;
}

void InverterROM::setPins(bool select, bool newClock, bool newData) {
	if (clock && newClock && data && !newData) // START in I²C
		state =1;  // Write command and data
	else
	if (clock && newClock && !data && newData) // STOP in I²C
		state =19; // Read result
	else
	if (!clock && newClock) {
		if (state ==0)
			data =newData;
		else
		if (state >=1 && state < 9) { // command byte
			command =command <<1 | newData*1;
			if (++state ==9 && command !=0x80) state =0;
		} else
		if (state ==9) // terminating bit
			state++;
		else
		if (state >=10 && state <18) { // data byte
			result =result <<1 | newData*1;
			if (++state ==18) result =-result >>4 &0xF | -result <<4 &0xF0;
		} else
		if (state ==18) // terminating bit
			state =0;
		else {
			output =!!(result &0x80);
			result <<=1;
		}
	}
	clock =newClock;
	data =newData;
}
} // namespace OneBus