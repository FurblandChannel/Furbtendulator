#include "h_FlashROM.h"

FlashROM::FlashROM (uint8_t _manufacturerID, uint8_t _modelID, uint8_t* _data, size_t _chipSize, size_t _sectorSize):
	manufacturerID(_manufacturerID),
	modelID(_modelID),
	state(0),
	data(_data),
	chipSize(_chipSize),
	sectorSize(_sectorSize),
	timeOut(0),
	magicAddr1(0x5555),
	magicAddr2(0x2AAA) {
}

FlashROM::FlashROM (uint8_t _manufacturerID, uint8_t _modelID, uint8_t* _data, size_t _chipSize, size_t _sectorSize, uint16_t _magicAddr1, uint16_t _magicAddr2):
	manufacturerID(_manufacturerID),
	modelID(_modelID),
	state(0),
	data(_data),
	chipSize(_chipSize),
	sectorSize(_sectorSize),
	timeOut(0),
	magicAddr1(_magicAddr1),
	magicAddr2(_magicAddr2) {
}

int FlashROM::read (int bank, int addr) const {
	if (state ==0x90) // software ID
		return addr &1? modelID: manufacturerID;
	else
	if (timeOut)
		return (EMU->GetPRG_Ptr4(bank)[addr &0xFFF] ^(timeOut &1? 0x40: 0x00)) &~0x88;
	else
		return EMU->GetPRG_Ptr4(bank)[addr &0xFFF];
}

void FlashROM::write (int bank, int addr, int val) {
	uint8_t* sector =NULL;
	switch (state) {
		// command start
		default:   if (addr ==magicAddr1 && val ==0xAA) state++; break;
		case 0x01: if (addr ==magicAddr2 && val ==0x55) state++; break;
		case 0x02: if (addr ==magicAddr1) state =val; break;
		// sector or chip erase
		case 0x80: if (addr ==magicAddr1 && val ==0xAA) state++; break;
		case 0x81: if (addr ==magicAddr2 && val ==0x55) state++; break;
		case 0x82: if (val ==0x30) { // sector erase
				// Determine the offset into the chip 
				int32_t offset =EMU->GetPRG_Ptr4(bank) -data +addr;
				if (offset >=0 && (unsigned) offset <chipSize) {
					// Determine the actual sector start by masking with the sector size
					offset &=~(sectorSize -1);
					sector =data +offset;
					for (uint32_t i =0; i <sectorSize; i++) sector[i] =0xFF;
					timeOut =sectorSize;
				}
			   } else
			   if (val ==0x10 && addr ==magicAddr1) { // chip erase
				for (uint32_t i =0; i<=chipSize; i++) data[i] =0xFF;
				timeOut =chipSize;
			   } else
			   if (val ==0xF0) state =0;
			   break;
		// software ID
		case 0x90: if (val ==0xF0) state =0; break;
		// byte program
		case 0xA0: sector =EMU->GetPRG_Ptr4(bank);
			   if (sector) sector[addr &0xFFF] =val;
			   state =0;
			   break;
	}
	//EMU->DbgOut(L"state %02X addr=%04X val=%02X magicAddr1=%04X magicAddr2=%04X", state, addr, val, magicAddr1, magicAddr2);
}

void FlashROM::cpuCycle() {
	if (timeOut && !--timeOut) state =0;
}