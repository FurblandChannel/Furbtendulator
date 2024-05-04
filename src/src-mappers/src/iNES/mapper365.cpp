#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	prg[4], chr[8], keyboardRow;
FCPURead readAPU;
FCPUWrite writeAPU;

void	sync (void) {
	prg[3] |=0x01;
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0; i <4; i++) EMU->SetPRG_ROM8(0x8 +i*2, prg[i]);
	for (int i =0; i <8; i++) EMU->SetCHR_RAM1(i, chr[i]);
}

int	MAPINT	readKeyboard (int bank, int addr) {
	int result =*EMU->OpenBus;
	switch (addr) {
		/*case 0x900:
			Result =0xE6; // reads/writes keyboard at 491F instead and flips F9 and F10;
			break;*/
		case 0x906:
			writeAPU(0x4, 0x016, 0x04 | 0x01); // Reset keyboard
			for (int i =0; i <=keyboardRow; i++) {
				writeAPU(0x4, 0x016, 0x04 | 0x00);
				result =(readAPU(0x4, 0x17) >>1) &0x0F;
				writeAPU(0x4, 0x016, 0x04 | 0x02);				
				result |= (readAPU(0x4, 0x17) <<3) &0xF0;
				//Result =(Result <<4) | (Result >>4);
			}
			//EMU->DbgOut(L"Row %X Result: %X", KeyboardRow, Result);
			break;
		default:
			result =readAPU(bank, addr);
			break;
	}
	return result;
}

void	MAPINT	writeKeyboard (int bank, int addr, int val) {
	switch (addr) {
		case 0x904:
			keyboardRow =val;
			break;
		case 0x905:
			// Keyboard, written to at end of VBlank handler
			break;
		default:
			writeAPU(bank, addr, val);
			break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[addr &3] =val;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &7] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readKeyboard);
	EMU->SetCPUWriteHandler(0x4, writeKeyboard);
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writePRG);
	EMU->SetCPUWriteHandler(0xA, writeCHR);
	EMU->SetCPUWriteHandler(0xB, writeCHR);
	iNES_SetMirroring();
	if (resetType ==RESET_HARD) for (int i =0; i <4; i++) prg[i] =i |0xFC;
	sync();	
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(stateMode, offset, data, prg[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, chr[i]);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =365;
} // namespace

MapperInfo MapperInfo_365 ={
	&mapperNum,
	_T("Asder PC-95"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
