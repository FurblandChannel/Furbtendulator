#include	"h_FME7.h"
#include	"Sound\s_SUN5.h"

namespace FME7 {
uint8_t		index;
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		mirroring;
uint8_t		irqControl;
uint16_t	counter;
FSync		sync;

#define	counting   (irqControl &0x80)
#define irqEnabled (irqControl &0x01)

void	syncMirror (void) {
	switch (mirroring &3) {
		case 0:	EMU->Mirror_V(); break;
		case 1:	EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	syncPRG (int AND, int OR) {
	if (prg[0] &0x40)
		if (prg[0] &0x80)
			EMU->SetPRG_RAM8(0x6, ROM->ROMType ==ROM_INES && ROM->INES_Version >=2? prg[0] &0x3F: 0x00);
		else
			for (int bank =0x6; bank <=0x7; bank++) EMU->SetPRG_OB4(bank);
	else
		EMU->SetPRG_ROM8(0x6, prg[0] &0x3F &AND |OR);
	EMU->SetPRG_ROM8(0x8, prg[1] &AND |OR);
	EMU->SetPRG_ROM8(0xA, prg[2] &AND |OR);
	EMU->SetPRG_ROM8(0xC, prg[3] &AND |OR);
	EMU->SetPRG_ROM8(0xE, 0xFF   &AND |OR);
}

void	syncCHR (int AND, int OR) {
	for (int bank =0; bank <8; bank++) {
		if (ROM->CHRROMSize)
			EMU->SetCHR_ROM1(bank, chr[bank] &AND |OR);
		else
			EMU->SetCHR_RAM1(bank, chr[bank] &AND |OR);
	}
}

void	MAPINT	writeIndex (int bank, int addr, int val) {
	index =val &0xF;
}

void	MAPINT	writeData (int bank, int addr, int val) {
	switch (index) {
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			chr[index] =val;
			break;
		case 0x8: case 0x9: case 0xA: case 0xb:
			prg[index &3] =val;
			break;
		case 0xC:
			mirroring =val;
			break;
		case 0xD:
			irqControl =val;
			EMU->SetIRQ(1);
			break;
		case 0xE:
			counter =counter &0xFF00 |val;
			break;
		case 0xF:
			counter =counter &0x00FF |val <<8;
			break;
	}
	sync();
}

void	MAPINT	writeSound (int bank, int addr, int val) {
	SUN5sound::Write(bank <<12 |addr, val);
}

void	MAPINT	load (FSync _sync) {
	SUN5sound::Load();
	sync =_sync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index  =0;
		prg[0] =0x00;
		prg[1] =0x00;
		prg[2] =0x01;
		prg[3] =0xFE;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		mirroring =0;
		irqControl =0;
		counter =0;
	}
	sync();

	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeIndex);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeData);
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeSound);
	SUN5sound::Reset(resetType);
}

void	MAPINT	unload (void) {
	SUN5sound::Unload();
}

void	MAPINT	cpuCycle (void) {
	if (counting && !--counter && irqEnabled) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, irqControl);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	offset = SUN5sound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace FME7