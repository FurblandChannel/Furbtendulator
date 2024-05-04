#include "h_EEPROM_93Cx6.h"

static constexpr uint8_t
	Opcode_misc =0,
	Opcode_write =1,
	Opcode_read =2,
	Opcode_erase =3,
	Opcode_writeDisable =10,
	Opcode_writeAll =11,
	Opcode_eraseAll =12,
	Opcode_writeEnable =13
;
static constexpr uint8_t
	State_standBy =0,
	State_startBit =1,
	State_opcode =3,
	State_address8 =12,
	State_data8 =20,
	State_address16 =11,
	State_data16 =27,
	State_finished =99
;	

EEPROM_93Cx6::EEPROM_93Cx6 (uint8_t* buffer, size_t _capacity, uint8_t _wordSize) {
	capacity =_capacity;
	storage =buffer;
	address =0;
	state =State_standBy;
	lastCLK =false;
	writeEnabled =false;
	wordSize =_wordSize;
	output =true;
	
	State_address =wordSize ==16? State_address16: State_address8;
	State_data    =wordSize ==16? State_data16:    State_data8;   
	if (capacity ==128) { State_address-=2; State_data-=2; }
}

void EEPROM_93Cx6::write (bool CS, bool CLK, bool DAT) {
	if (!CS && state <=State_address) {
		state =State_standBy;
	} else
	if (state ==State_standBy && CS ==true && CLK ==true && lastCLK ==false) {
		if (DAT) state =State_startBit;
		opcode =0;
		address =0;
		output =true;
	} else if (CLK && !lastCLK && state >=State_startBit) {
		if (state >=State_startBit && state <State_opcode)  opcode  =(opcode  <<1) | DAT*1; else
		if (state >=State_opcode   && state <State_address) address =(address <<1) | DAT*1; else
		if (state >=State_address  && state <State_data) {
			if (opcode ==Opcode_write || opcode ==Opcode_writeAll) data =(data <<1) | DAT*1; else
			if (opcode ==Opcode_read) {
				if (wordSize ==16)
					output =!!(data &0x8000);
				else
					output =!!(data &0x80);
				data =data <<1;
			}
		}
		state++;
		if (state ==State_address) {
			switch (opcode) {
				case Opcode_misc:
					opcode =(address >>(wordSize ==16? 6: 7)) +10;
					switch (opcode) {
						case Opcode_writeDisable:  writeEnabled =false;
						                           state =State_finished;
									   break;
						case Opcode_writeEnable:  writeEnabled =true;
						                           state =State_finished;
									   break;
						case Opcode_eraseAll:     if (writeEnabled) for (int i =0; i <capacity; i++) storage[i] =0xFF;
						                           state =State_finished;
									   break;
						case Opcode_writeAll:	   address =0;
									   break;
					}
					break;
				case Opcode_erase:
					if (writeEnabled) {
						if (wordSize ==16) {
							storage[address <<1 |0] =0xFF;
							storage[address <<1 |1] =0xFF;
						} else
							storage[address] =0xFF;
					}
					state =State_finished;
					break;
				case Opcode_read:
					if (wordSize ==16) {
						data =storage[address <<1 |0] | storage[address <<1 |1] <<8;
						address++;
					} else
						data =storage[address++];
					break;
			}
		} else
		if (state ==State_data) {
			if (opcode ==Opcode_write) {
				if (wordSize ==16) {
					EMU->StatusOut(L"Written word %d", address);
					storage[address <<1 |0] =data &0xFF;
					storage[address <<1 |1] =data >>8;
					address++;
				} else {
					EMU->StatusOut(L"Written byte %d", address);
					storage[address++] =data;
				}
				state =State_finished;
			} else
			if (opcode ==Opcode_writeAll) {
				if (wordSize ==16) {
					storage[address <<1 |0] =data &0xFF;
					storage[address <<1 |1] =data >>8;
					address++;
				} else
					storage[address++] =data;
				state =(CS && (address <capacity))? State_address: State_finished;
			} else
			if (opcode ==Opcode_read) {
				if (address <capacity) {
					if (wordSize ==16) {
						data =storage[address <<1 |0] | storage[address <<1 |1] <<8;
					} else
						data =storage[address];
				}
				state =(CS && (++address <=capacity))? State_address: State_finished;
			}
		}
		
		if (state ==State_finished) {
			output =false;
			state =State_standBy;
		}
	}
	if (opcode ==Opcode_read && state ==(State_address -2)) output =false; // Dummy Zero
	//EMU->DbgOut(L"state %d, opcode %d, address %02X, data %02X, CS %d CLK %d DAT %d", state, opcode, address, data, CS, CLK, DAT);
	lastCLK =CLK;
}

bool EEPROM_93Cx6::read (void) {
	return output;
}

int MAPINT EEPROM_93Cx6::saveLoad (STATE_TYPE stateMode, int offset, unsigned char *stateData) {
	SAVELOAD_BYTE(stateMode, offset, stateData, opcode);
	SAVELOAD_WORD(stateMode, offset, stateData, data);
	SAVELOAD_WORD(stateMode, offset, stateData, address);
	SAVELOAD_BOOL(stateMode, offset, stateData, lastCLK);
	SAVELOAD_BOOL(stateMode, offset, stateData, writeEnabled);
	SAVELOAD_BOOL(stateMode, offset, stateData, output);
	return offset;
}

