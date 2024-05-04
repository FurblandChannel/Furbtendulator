#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint8_t		chr[4];
uint8_t		mirroring;
uint16_t	counter;
bool		counting;
bool		lsb;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, 0xFF);

	EMU->SetCHR_ROM2(0, chr[0]);
	EMU->SetCHR_ROM2(2, chr[1]);
	EMU->SetCHR_ROM2(4, chr[2]);
	EMU->SetCHR_ROM2(6, chr[3]);

	switch (mirroring &3) {
		case 0:	EMU->Mirror_V(); break;
		case 1:	EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &0x800) switch(bank &7) {
		case 0: case 1: case 2: case 3:
			chr[bank &3] =val;
			sync();
			break;
		case 4:	if (lsb)
				counter =counter &0xFF00 |val;
			else
				counter =counter &0x00FF |val <<8;
			lsb =!lsb;
			break;
		case 5:	lsb =false;
			counting =!!(val &0x10);
			break;
		case 6:	mirroring =val;
			sync();
			break;
		case 7:	prg =val;
			sync();
			break;
	} else
		EMU->SetIRQ(1);
}
void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType == RESET_HARD) {
		prg =0;
		for (int i =0; i <4; i++) chr[i] =i;
		mirroring =0;
		counter =0;
		counting =false;
		lsb =false;
	}
	sync();
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (counting && --counter ==0xFFFF) {
		EMU->SetIRQ(0);
		counting =false;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BOOL(stateMode, offset, data, counting);
	SAVELOAD_BOOL(stateMode, offset, data, lsb);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring); 
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =67;
} // namespace

MapperInfo MapperInfo_067 ={
	&MapperNum,
	_T("Sunsoft-3"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
