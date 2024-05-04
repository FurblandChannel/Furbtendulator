#include	"h_MMC1.h"

namespace MMC1 {
FSync		sync;
MMC1Type	revision;

uint8_t		reg[4];
uint8_t		shift;
uint8_t		bits;
uint8_t		filter;

void	MAPINT	write (int bank, int addr, int val) {
	if (val &0x80) {
		reg[0] |=0x0C;
		shift =0;
		bits =0;
	} else
	if (!filter) {
		shift |=(val &1) <<bits++;
		if (bits ==5) {
			reg[(bank >>1) &3] =shift;
			shift =0;
			bits =0;
			sync();
		}
	}
	filter =2;
}

int	getPRGBank (int bank) {
	int  prg      =reg[3];
	int  result;	

	if (reg[0] &0x08)
		result =reg[0] &0x04? (prg |bank*0xF): (prg &bank*0xF);
	else
		result =prg &~1 |bank;
	
	if (reg[3] &0x10 && revision ==MMC1Type::MMC1A)
		return result &0x07 | prg &0x08;
	else
		return result &0x0F;
}

int	getCHRBank (int bank) {
	if (reg[0] &0x10)
		return reg[1 +bank];
	else
		return reg[1] &~1 |bank;
}

void	syncWRAM (int bank) {
	if (reg[3] &0x10 && revision ==MMC1Type::MMC1B) {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	} else
		EMU->SetPRG_RAM8(0x6, bank);
}

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM16(0x8, getPRGBank(0) &AND |OR);
	EMU->SetPRG_ROM16(0xC, getPRGBank(1) &AND |OR);
}

void	syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize)
		syncCHR_ROM(AND, OR);
	else
		syncCHR_RAM(AND, OR);
}

void	syncCHR_ROM (int AND, int OR) {
	EMU->SetCHR_ROM4(0x0, getCHRBank(0) &AND |OR);
	EMU->SetCHR_ROM4(0x4, getCHRBank(1) &AND |OR);
}

void	syncCHR_RAM (int AND, int OR) {
	EMU->SetCHR_RAM4(0x0, getCHRBank(0) &AND |OR);
	EMU->SetCHR_RAM4(0x4, getCHRBank(1) &AND |OR);
}

void	syncMirror (void) {
	switch (reg[0] &0x03) {
		case 0:	EMU->Mirror_S0(); break;
		case 1:	EMU->Mirror_S1(); break;
		case 2:	EMU->Mirror_V (); break;
		case 3:	EMU->Mirror_H (); break;
	}
}

void	MAPINT	load (FSync _sync, MMC1Type _revision) {
	sync =_sync;
	revision =_revision;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		shift =0;
		bits =0;
		reg[0] =0x0C;
		reg[1] =0x00;
		reg[2] =0x00;
		reg[3] =0x00;
	}
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (filter) filter--;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	SAVELOAD_BYTE(stateMode, offset, data, shift);
	SAVELOAD_BYTE(stateMode, offset, data, bits);
	SAVELOAD_BYTE(stateMode, offset, data, filter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
} // namespace MMC1