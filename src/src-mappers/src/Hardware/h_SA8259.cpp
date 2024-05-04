#include "h_SA8259.h"

namespace SA8259 {
uint8_t		index;
uint8_t		prg;
uint8_t		chrMid;
uint8_t		chrHigh;
uint8_t		chr[4];
uint8_t		mirroring;
bool		simple;

FSync		sync;
FCPUWrite	writeAPU;

void	syncPRG (void) {
	EMU->SetPRG_ROM32(0x8, prg);
}

void	syncCHR (int shift) {
	if (ROM->CHRROMSize) for (int bank =0; bank <4; bank++) {
		int val =chr[simple? 0: bank];
		val |=chrHigh <<3;
		val <<=shift;
		val |=bank &((1 <<shift) -1);
		EMU->SetCHR_ROM2(bank *2, val);
	} else
		EMU->SetCHR_RAM8(0x0, 0);
}

void	syncMirror (void) {
	if (simple)
		EMU->Mirror_V();
	else switch(mirroring) {
		case 0:	EMU->Mirror_Custom(0, 0, 0, 1); break;
		case 1:	EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_V(); break;
		case 3:	EMU->Mirror_S0(); break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		if (addr &1) switch (index) {
			case 0: case 1: case 2: case 3:
				chr[index] =val &0x7;
				break;
			case 4:	chrHigh =val &0x7;
				break;
			case 5:	prg =val;
				break;
			case 6:	chrMid =val &0x1;
				break;
			case 7:	simple =!!(val &1);
				mirroring =(val >>1) &3;
				break;
		} else
			index =val &0x7;
		sync();
	}
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index =0;
		prg =0;
		for (int bank =0; bank <4; bank++) chr[bank] =bank;
		simple =false;
		mirroring =2;
	}
	sync();
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chrMid);
	SAVELOAD_BYTE(stateMode, offset, data, chrHigh);
	for (int i =0; i <4; i++) SAVELOAD_BYTE(stateMode, offset, data, chr[i]);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BOOL(stateMode, offset, data, simple);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

}