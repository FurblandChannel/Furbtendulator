#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\h_VRC6.h"
#include	"..\..\Hardware\h_FlashROM.h"

#define locked     !!(reg[0] &0x80)
#define mirrorV    !!(reg[4] &0x01)
#define mapper       (reg[0] &0x1F)
#define protectCHR !!(reg[5] &0x04)
#define prgOR        (reg[1] | reg[2] <<8)
#define prgAND       ~reg[3]
#define outerCHR      reg[6]

#define MAPPER_UNROM     0x00
#define MAPPER_MMC3      0x01
#define MAPPER_NROM      0x02
#define MAPPER_CNROM     0x03
#define MAPPER_ANROM     0x04
#define MAPPER_SLROM     0x05
#define MAPPER_SNROM     0x06
#define MAPPER_SUROM     0x07
#define MAPPER_GNROM     0x08
#define MAPPER_PNROM     0x09  // punchout
#define MAPPER_HKROM     0x0A // startropics
#define MAPPER_BANDAI152 0x0B // ark 2
#define MAPPER_TLSROM    0x0E
#define MAPPER_189       0x0F
#define MAPPER_VRC6      0x10
#define MAPPER_VRC2_22   0x12	// Twinbee 3 (m022)
#define MAPPER_VRC4_25   0x15	// Gradius 2 (m025)
#define MAPPER_VRC4_23   0x18	// Crisis Force (m023)
#define MAPPER_VRC1      0x1A

namespace {
FlashROM	*flash =NULL;

uint8_t		reg[8], latch;
FCPUWrite	dummy;

void	sync (void) {
	if (locked) switch(mapper) {
		case MAPPER_NROM:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM8(0x8, 0 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xA, 1 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xC, 2 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xE, 3 &prgAND | prgOR);
			EMU->SetCHR_RAM8(0x0, outerCHR);
			if (mirrorV)
				EMU->Mirror_V();
			else
				EMU->Mirror_H();
			break;
		case MAPPER_CNROM:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM8(0x8, 0 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xA, 1 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xC, 2 &prgAND | prgOR);
			EMU->SetPRG_ROM8(0xE, 3 &prgAND | prgOR);
			EMU->SetCHR_RAM8(0x0, Latch::data &3);
			if (mirrorV)
				EMU->Mirror_V();
			else
				EMU->Mirror_H();
			break;
		case MAPPER_UNROM:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM16(0x8, Latch::data &(prgAND >>1)      | prgOR >>1);
			EMU->SetPRG_ROM16(0xC,               prgAND >>1 &0x1F | prgOR >>1);
			EMU->SetCHR_RAM8(0x0, outerCHR);
			if (mirrorV)
				EMU->Mirror_V();
			else
				EMU->Mirror_H();
			break;
		case MAPPER_BANDAI152:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM16(0x8, Latch::data >>4 &(prgAND >>1)      | prgOR >>1);
			EMU->SetPRG_ROM16(0xC, 0xFF            &(prgAND >>1)      | prgOR >>1);
			EMU->SetCHR_RAM8(0x0, Latch::data &0xF);
			if (Latch::data &0x80)
				EMU->Mirror_S1();
			else
				EMU->Mirror_S0();
			break;
		case MAPPER_ANROM:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM32(0x8, Latch::data &(prgAND >>2)      | prgOR >>2);
			EMU->SetCHR_RAM8(0x0, outerCHR);
			if (Latch::data &0x10)
				EMU->Mirror_S1();
			else
				EMU->Mirror_S0();
			break;
		case MAPPER_GNROM:
			EMU->SetPRG_OB4(0x6);
			EMU->SetPRG_OB4(0x7);
			EMU->SetPRG_ROM32(0x8, Latch::data >>4 &(prgAND >>2)      | prgOR >>2);
			EMU->SetCHR_RAM8(0x0, Latch::data &3);
			if (mirrorV)
				EMU->Mirror_V();
			else
				EMU->Mirror_H();
			break;
		case MAPPER_SLROM:
		case MAPPER_SNROM:
			MMC1::syncWRAM(0);
			MMC1::syncPRG(prgAND >>1, prgOR >>1);
			MMC1::syncCHR(0x1F, outerCHR >>2);
			MMC1::syncMirror();
			break;
		case MAPPER_MMC3:
			MMC3::syncWRAM();
			MMC3::syncPRG(prgAND, prgOR);
			MMC3::syncCHR(0xFF, outerCHR);
			MMC3::syncMirror();
			break;
		case MAPPER_TLSROM:
			MMC3::syncWRAM();
			MMC3::syncPRG(prgAND, prgOR);
			MMC3::syncCHR(0x7F, outerCHR);
			MMC3::syncMirrorA17();
			break;
		case MAPPER_189:
			MMC3::syncWRAM();
			EMU->SetPRG_ROM32(0x8, latch &3 | prgOR >>2);
			MMC3::syncCHR(0xFF, outerCHR);
			MMC3::syncMirror();
			break;
		case MAPPER_VRC2_22:
			VRC24::syncPRG(prgAND, prgOR);
			for (int i =0; i <8; i++) EMU->SetCHR_RAM1(i, VRC24::chr[i] >>1);
			VRC24::syncMirror();
			break;
		case MAPPER_VRC4_23:
		case MAPPER_VRC4_25:
			VRC24::syncPRG(prgAND, prgOR);
			VRC24::syncCHR(0xFF, 0x00);
			VRC24::syncMirror();
			break;
		case MAPPER_VRC6:
			VRC6::syncPRG(prgAND, prgOR);
			VRC6::syncCHR_ROM(0xFF, 0x00);
			VRC6::syncMirror(0xFF, 0x00);
			break;
	} else {
		EMU->SetPRG_ROM8(0x8, prgOR);
		EMU->SetPRG_ROM8(0xA, 0x3D);
		EMU->SetPRG_ROM8(0xC, 0x3E);
		EMU->SetPRG_ROM8(0xE, 0x3F);
		EMU->SetCHR_RAM8(0x0, outerCHR);
		if (mirrorV)
			EMU->Mirror_V();
		else
			EMU->Mirror_H();		
	}
	if (protectCHR) protectCHRRAM();
}

void	applyMode ();
void	MAPINT	writeReg (int bank, int addr, int val) {
	addr &=7;
	if (addr ==0 && ROM->INES2_SubMapper ==0) switch (val &0x1F) { /* Translate submapper 0 fusemap numbers (single-game cartridges) to submapper 1 fusemap numbers (Pixel multicart) */
		case 1: val =val &~0x1F | MAPPER_SNROM; break;
	}
	reg[addr] =val;
	if (locked) applyMode();
	sync();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	latch =addr &0xFF;
	sync();
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	flash->write(bank, addr, val);
}

void	applyMode () {
	for (int bank =0x5; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, dummy);
	if (locked) switch(mapper) {
		case MAPPER_ANROM:
		case MAPPER_CNROM:
		case MAPPER_UNROM:
		case MAPPER_GNROM:
		case MAPPER_BANDAI152:
			Latch::reset(RESET_HARD);
			break;			
		case MAPPER_SLROM:
		case MAPPER_SNROM:
			MMC1::reset(RESET_HARD);
			break;			
		case MAPPER_189:
			EMU->SetCPUWriteHandler(0x6, writeLatch);
			// Fall-through
		case MAPPER_MMC3:
		case MAPPER_TLSROM:
			MMC3::reset(RESET_HARD);
			break;			
		case MAPPER_VRC2_22:
			VRC24::vrc4 =false;
			VRC24::A0 =0x02;
			VRC24::A1 =0x01;
			VRC24::reset(RESET_HARD);
			break;
		case MAPPER_VRC4_23:
			VRC24::vrc4 =true;
			VRC24::A0 =0x05;
			VRC24::A1 =0x0A;
			VRC24::reset(RESET_HARD);
			break;
		case MAPPER_VRC4_25:
			VRC24::vrc4 =true;
			VRC24::A0 =0x0A;
			VRC24::A1 =0x05;
			VRC24::reset(RESET_HARD);
			break;
		case MAPPER_VRC6:
			VRC6::reset(RESET_HARD);
			break;
	} else {
		EMU->SetCPUWriteHandler(0x5, writeReg);
		for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeFlash);		
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, false);
	MMC1::load(sync, MMC1Type::MMC1B);
	MMC3::load(sync, MMC3Type::Sharp);
	VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
	VRC6::load(sync, 0x01, 0x02);
	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(0x01, 0x7E, ROM->ChipRAMData, ROM->ChipRAMSize, 131072, 0xAAA, 0x555); // Cypress S29GL512S
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	dummy =EMU->GetCPUWriteHandler(0x8);
	applyMode();
	sync();
}

void	MAPINT	cpuCycle (void) {
	if (locked) switch(mapper) {
		case MAPPER_SLROM:
		case MAPPER_SNROM:
			MMC1::cpuCycle();
			break;			
		case MAPPER_MMC3:
		case MAPPER_TLSROM:
		case MAPPER_189:
			MMC3::cpuCycle();
			break;			
		case MAPPER_VRC2_22:
		case MAPPER_VRC4_23:
		case MAPPER_VRC4_25:
			VRC24::cpuCycle();
			break;
		case MAPPER_VRC6:
			VRC6::cpuCycle();
			break;
	}
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (locked) switch(mapper) {
		case MAPPER_MMC3:
		case MAPPER_TLSROM:
		case MAPPER_189:
			MMC3::ppuCycle(addr, scanline, cycle, isRendering);
			break;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) applyMode();
	offset =Latch::saveLoad_AD(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =VRC24::saveLoad(stateMode, offset, data);
	offset =VRC6::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =446;
} // namespace

MapperInfo MapperInfo_446 ={
	&mapperNum,
	_T("SMD172B_FPGA"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};