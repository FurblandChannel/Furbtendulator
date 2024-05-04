#include	"h_VRC6.h"
#include	"../Dll/d_iNES.h"

namespace VRC6 {
FSync		sync;
uint8_t		A0, A1;
uint8_t		mode;
uint8_t		prg[2];
uint8_t		chr[8];
uint8_t		irqControl, irqCounter, irqLatch;
int16_t		irqCycles;

#define	INES_CHR_4MBIT	(ROM->INES_CHRSize >=512/8)

#define	B003_CHANGE_A10	(mode &0x20)
#define	B003_CHR_MODE	(mode &3)
#define	B003_MIRRORING	((mode >>2) &3)
#define	B003_USE_VROM	(mode &0x10)
#define	B003_USE_WRAM	(mode &0x80)
#define	IRQ_ACKNOWLEDGE	0x01
#define	IRQ_ENABLE	0x02
#define	IRQ_CYCLE_MODE	0x04

void	MAPINT	setCHR1K (int bank, int val) {
	if (INES_CHR_4MBIT)	// 512 KiB CHR-ROM-bearing circuit boards connect VRC6's A10-A17 to CHR-ROM's A11-A18 and PPU's A10 to CHR-ROM's A10
		iNES_SetCHR_Auto1(bank, val <<1 | bank &1);
	else			// All others connect VRC6's A10-A17 to CHR-ROM's A10-A17
		iNES_SetCHR_Auto1(bank, val);
}

void	MAPINT	setNametable (int screen, int val) {
	if (B003_USE_VROM) {
		setCHR1K(screen +0x8, val);
		setCHR1K(screen +0xC, val);
	} else if (ROM->INES_CHRSize && ROM->INES2_CHRRAM >6) { // Have CHR-ROM and >4 KiB of RAM
		EMU->SetCHR_RAM1(screen +0x8, val);
		EMU->SetCHR_RAM1(screen +0xC, val);
	} else {						// CIRAM or just 4 KiB four-screen mirroring
		EMU->SetCHR_NT1(screen +0x8, val &(CART_VRAM? 3: 1));
		EMU->SetCHR_NT1(screen +0xC, val &(CART_VRAM? 3: 1));
	}
}

void	MAPINT	syncPRG (int AND, int OR) {
	if (B003_USE_WRAM)
		EMU->SetPRG_RAM8(0x6, 0);
	else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
	}
	EMU->SetPRG_ROM16(0x8, prg[0] &AND >>1 | OR >>1);
	EMU->SetPRG_ROM8 (0xC, prg[1] &AND     | OR    );
	EMU->SetPRG_ROM8 (0xE, 0xFF   &AND     | OR    );
}

void	MAPINT	syncCHR_ROM (int AND, int OR) {
	uint8_t bank =0, reg =0;
	switch (B003_CHR_MODE) {
		case 0:	for (; bank <8; bank++) setCHR1K(bank, chr[reg++] &AND |OR);
			break;
		default:for (; bank <4; bank++) setCHR1K(bank, chr[reg++] &AND |OR);
			// Mode 2-3: banks 4-7 same as Mode 1, so fall-through
		case 1:	for (; bank <8; bank +=2) {
				int val =chr[reg++] &AND |OR;
				if (B003_CHANGE_A10) {
					setCHR1K(bank +0, val &0xFE);
					setCHR1K(bank +1, val |0x01);
				} else {
					setCHR1K(bank +0, val);
					setCHR1K(bank +1, val);
				}
			}
			break;
	}
}

void	MAPINT	syncMirror(int AND, int OR) {
	uint8_t reg;
	
	for (int screen =0; screen <4; screen++) {
		if (B003_CHR_MODE ==1)
			reg =4 | screen;
		else if ((B003_CHR_MODE >>1) ^(B003_MIRRORING &1))
			reg =6 |(screen &1);
		else
			reg =6 |(screen >>1);
		
		int val =chr[reg];
		if (B003_CHANGE_A10 && (B003_CHR_MODE ==0 || B003_CHR_MODE ==3)) {
			val &=~1;
			switch (B003_MIRRORING ^(B003_CHR_MODE &1)) {
				case 0: val |=(screen &1)? 1: 0; break; // Horizontal Mirroring
				case 1: val |=(screen &2)? 1: 0; break; // Vertical Mirroring
				case 2: val |=0; break;     	        // One-screen Mirroring, page 0
				case 3: val |=1; break;		        // One-screen Mirroring, page 1
			}
		}
		setNametable(screen, val &AND |OR);
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[(bank >>2) &1] =val;
	sync();
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	int reg =(addr &A1? 2: 0) | (addr &A0? 1: 0);
	if (reg ==3) {
		mode =val;
		sync();
	} else
		VRC6sound::Write(bank <<12 | reg, val);
}

void	MAPINT	writeSound (int bank, int addr, int val) {
	int reg =(addr &A1? 2: 0) | (addr &A0? 1: 0);
	VRC6sound::Write(bank <<12 | reg, val);
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	int reg =((bank -0xD)<<2) | (addr &A1? 2: 0) | (addr &A0? 1: 0);
	chr[reg] =val;
	sync();
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	int reg = (addr &A1? 2: 0) | (addr &A0? 1: 0);
	switch (reg) {
	case 0:	irqLatch =val;
		break;
	case 1:	irqControl =val;
		if (irqControl &IRQ_ENABLE) {
			irqCounter =irqLatch;
			irqCycles =341;
		}
		EMU->SetIRQ(1);
		break;
	case 2:	if (irqControl &IRQ_ACKNOWLEDGE)
			irqControl |= IRQ_ENABLE;
		else
			irqControl &=~IRQ_ENABLE;
		EMU->SetIRQ(1);
		break;
	}
}

void	MAPINT	load (FSync cSync, uint8_t cA0, uint8_t cA1) {
	sync =cSync;
	A0 =cA0;
	A1 =cA1;
	VRC6sound::Load();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[0] =0x00;
		prg[1] =0xFE;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		mode =0;
		irqControl =0;
		irqCounter =0;
		irqLatch =0;
		irqCycles =0;
	}
	sync();
	EMU->SetIRQ(1);
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeSound);
	EMU->SetCPUWriteHandler(0xA, writeSound);
	EMU->SetCPUWriteHandler(0xB, writeMode);
	EMU->SetCPUWriteHandler(0xC, writePRG);
	EMU->SetCPUWriteHandler(0xD, writeCHR);
	EMU->SetCPUWriteHandler(0xE, writeCHR);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
	VRC6sound::Reset(resetType);
}

void	MAPINT	unload (void) {
	VRC6sound::Unload();
}

void	MAPINT	cpuCycle (void) {
	if (irqControl &IRQ_ENABLE && (irqControl &IRQ_CYCLE_MODE || (irqCycles -=3) <=0)) {
		if (~irqControl &IRQ_CYCLE_MODE) irqCycles +=341;
		if (irqCounter ==0xFF) {
			irqCounter =irqLatch;
			EMU->SetIRQ(0);
		}
		else
			irqCounter++;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, irqControl);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BYTE(stateMode, offset, data, irqLatch);
	SAVELOAD_WORD(stateMode, offset, data, irqCycles);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	offset =VRC6sound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int cycles) {
	return VRC6sound::Get(cycles);
}

} // namespace VRC6