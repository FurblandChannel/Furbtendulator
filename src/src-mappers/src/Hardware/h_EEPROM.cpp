#include	"h_EEPROM.h"

// General I²C EEPROM device
EEPROM_I2C::EEPROM_I2C(uint16_t _addressMask, uint8_t _deviceAddr, uint8_t* _rom):
	I2CDevice::I2CDevice(0b1010, _deviceAddr),
	address(0),
	addressMask(_addressMask),
	rom(_rom),
	bit(0),
	latch(0) {
}

void EEPROM_I2C::reset() {
	I2CDevice::reset();
	address =0;
	bit =0;
	latch =0;
}

void EEPROM_I2C::receiveBit() {
	switch(state) {
		default: // Initial states
			I2CDevice::receiveBit();
			break;
		case 10: // ACK state after address+mode/read transfer. Load latch in read mode.
			bit =0;
			if (readMode) {
				latch =rom[address &addressMask];
				state =11;
			} else {
				latch =0;
				state =12;
			}
			break;
		case 11: // Read mode: return next bit, go back to state 10 after all bits were returned.
			if (++bit ==8) {
				address =address &~0xFF | (address +1) &0xFF;
				state--;
			}
			break;			
		case 12: // Write mode: receive address byte LSB. MSB must be set by derived class, if applicable.
			latch |=data? (0x80 >>bit): 0;
			if (++bit ==8) {
				address =address &~0xFF | latch;
				state++;
			}
			break;
		case 13: // Write mode: ACK state after address load/write.
			bit =0;
			latch =0;
			state++;
			break;
		case 14: // Write mode: Transfer next bit, go back to state 14 after all bits were written.
			latch |=data? (0x80 >>bit): 0;
			if (++bit ==8) {
				rom[address &addressMask] =latch;
				address =address &~0xFF | (address +1) &0xFF;
				state--;
			}
			break;
	}
	
	// Set data line based on (new) state
	switch (state) {
		case 10:
		case 13:
			output =false; // ACK
			break;
		case 11:
			output =!!(latch &(0x80 >>bit)); // Return bit in read mode
			break;
		default:
			output =true; // Default state is high
			break;
	}
}

int MAPINT EEPROM_I2C::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	stateOffset =I2CDevice::saveLoad(stateMode, stateOffset, stateData);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, address);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, bit);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, latch);
	// Leave it to caller to save/restore the ROM content.
	return stateOffset;
}

EEPROM_24C01::EEPROM_24C01(uint8_t _deviceAddr, uint8_t* _rom):
	EEPROM_I2C(0x07F, _deviceAddr, _rom) {
}

EEPROM_24C02::EEPROM_24C02(uint8_t _deviceAddr, uint8_t* _rom):
	EEPROM_I2C(0x0FF, _deviceAddr, _rom) {
}

EEPROM_24C04::EEPROM_24C04(uint8_t _deviceAddr, uint8_t* _rom):
	EEPROM_I2C(0x1FF, _deviceAddr, _rom) {
}

EEPROM_24C08::EEPROM_24C08(uint8_t _deviceAddr, uint8_t* _rom):
	EEPROM_I2C(0x3FF, _deviceAddr, _rom) {
}

EEPROM_24C16::EEPROM_24C16(uint8_t _deviceAddr, uint8_t* _rom):
	EEPROM_I2C(0x7FF, _deviceAddr, _rom) {
}

void EEPROM_24C01::receiveBit() {
	switch(state) { // 24C01 uses a non-standard bit sequence, in which the device address itself is the address within the ROM.
		case 1: // Start condition
			bit =0;
			latch =0;
			state++;
			break;
		case 2:	// Receive address bit.
			latch |=data? (0x01 <<bit): 0;
			if (++bit ==7) {
				address =latch;
				state++;
			}
			break;
		case 3: // Read/write mode
			readMode =data;
			++state;
			break;
		case 4: // ACK state after address+mode/read transfer. Load latch in read mode.
			bit =0;
			if (readMode) {
				latch =rom[address &addressMask];
				state =5;
			} else {
				latch =0;
				state =6;
			}
			break;
		case 5: // Read mode: return next bit, go back to state 4 after all bits were returned.
			if (++bit ==8) {
				address =++address &addressMask;
				state =4;
			}
			break;			
		case 6: // Write mode: Transfer next bit, go back to state 4 after all bits were written.
			latch |=data? (0x01 <<bit): 0;
			if (++bit ==8) {
				rom[address &addressMask] =latch;
				address =++address &addressMask;
				state =4;
			}
			break;
	}
	// Set data line based on (new) state
	switch (state) {
		case 4:
			output =false; // ACK
			break;
		case 5:
			output =!!(latch &(0x01 <<bit)); // Return bit in read mode
			break;
		default:
			output =true; // Default state is high
			break;
	}
}


void EEPROM_24C04::receiveBit() {
	switch(state) {
		case 8:	// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x100 | (data *0x100);
			state++;
			output =true;
			break;
		default: // All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}

void EEPROM_24C08::receiveBit() {
	switch(state) {
		case 7:	// 2nd bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x200 | (data *0x200);
			state++;
			output =true;
			break;
		case 8:	// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x100 | (data *0x100);
			state++;
			output =true;
			break;
		default: // All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}

void EEPROM_24C16::receiveBit() {
	switch(state) {
		case 6:	// 1st bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x400 | (data *0x400);
			state++;
			output =true;
			break;
		case 7:	// 2nd bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x200 | (data *0x200);
			state++;
			output =true;
			break;
		case 8:	// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address =address &~0x100 | (data *0x100);
			state++;
			output =true;
			break;
		default: // All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}
