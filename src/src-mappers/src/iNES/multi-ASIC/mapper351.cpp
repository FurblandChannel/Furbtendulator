#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_VRC24.h"

#define mapper   (reg[0] &0x03)
#define M_MMC3   0x00
#define M_MMC1   0x02
#define M_VRC4   0x03
#define nrom   !!(reg[2] &0x10)
#define prg128 !!(reg[2] &0x04)
#define chrram !!(reg[2] &0x01)
#define chr8   !!(reg[2] &0x40)
#define chr128 !!(reg[2] &0x20)
#define chr32  !!(reg[2] &0x10 && !chr128) 

#define prgAND (prg128? 0x0F: 0x1F)
#define chrAND (chr32? 0x1F: chr128? 0x7F: 0xFF)

namespace {
uint8_t		reg[4];
size_t		actualPRGROMSize;

void	sync (void) {
	EMU->SetPRGROMSize(actualPRGROMSize +(chrram? ROM->CHRROMSize: 0));
	if (nrom) {
		if (reg[2] &0x08) {
			EMU->SetPRG_ROM8(0x8, reg[1] >>1);
			EMU->SetPRG_ROM8(0xA, reg[1] >>1);
			EMU->SetPRG_ROM8(0xC, reg[1] >>1);
			EMU->SetPRG_ROM8(0xE, reg[1] >>1);
		} else {
			EMU->SetPRG_ROM16(0x8, reg[1] >>2 &~!prg128);
			EMU->SetPRG_ROM16(0xC, reg[1] >>2 | !prg128);
		}
	} else
	switch(mapper) {
		case M_MMC1: MMC1::syncPRG(prgAND >>1, (reg[1] >>1 &~prgAND) >>1); break;
		default:
		case M_MMC3: MMC3::syncPRG(prgAND, reg[1] >>1 &~prgAND); break;
		case M_VRC4: VRC24::syncPRG(prgAND, reg[1] >>1 &~prgAND); break;
	}
	
	if (chrram)
		EMU->SetCHR_RAM8(0x0, 0);
	else
	if (chr8)
		EMU->SetCHR_ROM8(0x0, reg[0] >>2);
	else
	switch(mapper) {		
		case M_MMC1: MMC1::syncCHR_ROM(chrAND >>2, (reg[0] <<1 &~chrAND) >>2); break;
		default:
		case M_MMC3: MMC3::syncCHR_ROM(chrAND, reg[0] <<1 &~chrAND); break;
		case M_VRC4: VRC24::syncCHR_ROM(chrAND, reg[0] <<1 &~chrAND); break;
	}
	
	switch(mapper) {
		case M_MMC1: MMC1::syncMirror(); break;
		default:
		case M_MMC3: MMC3::syncMirror(); break;
		case M_VRC4: VRC24::syncMirror(); break;
	}
}

void	MAPINT	writeVRC4 (int bank, int addr, int val) {
	if (addr &0x800) addr =(addr &4? 8: 0) | (addr &8? 4: 0) | addr &~0xC;
	VRC24::write(bank, addr, val);
}

void	applyMode (void) {
	switch(mapper) {
		case M_MMC1:
			EMU->SetIRQ(1);
			for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
			break;
		default:
		case M_MMC3:
			EMU->SetCPUWriteHandler(0x8, MMC3::writeReg);
			EMU->SetCPUWriteHandler(0x9, MMC3::writeReg);
			EMU->SetCPUWriteHandler(0xA, MMC3::writeMirroringWRAM);
			EMU->SetCPUWriteHandler(0xB, MMC3::writeMirroringWRAM);
			EMU->SetCPUWriteHandler(0xC, MMC3::writeIRQConfig);
			EMU->SetCPUWriteHandler(0xD, MMC3::writeIRQConfig);
			EMU->SetCPUWriteHandler(0xE, MMC3::writeIRQEnable);
			EMU->SetCPUWriteHandler(0xF, MMC3::writeIRQEnable);
			break;
		case M_VRC4:
			EMU->SetIRQ(1);
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeVRC4);
			break;
	}
}

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue &7 |*EMU->OpenBus &~7;
}

void	MAPINT	writeFDS (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr ==0x025) {
		MMC3::mirroring =val >>3 &1;
		sync();
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &3] =val;	
	if ((addr &3) ==0) applyMode();
	sync();
}

BOOL	MAPINT	load (void) {
	if (ROM->CHRROMSize) { /* Apparently, this mapper can access CHR ROM as PRG ROM? */
		actualPRGROMSize =ROM->PRGROMSize;
		EMU->SetPRGROMSize(ROM->PRGROMSize +ROM->CHRROMSize);
		for (int i =0; i <ROM->CHRROMSize; i++) ROM->PRGROMData[actualPRGROMSize +i] =ROM->CHRROMData[i];
	}
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	MMC1::load(sync, MMC1Type::MMC1A);
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC1::reset(RESET_HARD);
	MMC3::reset(RESET_HARD);
	VRC24::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x4, writeFDS);
	EMU->SetCPUReadHandler(0x5, readDIP);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	applyMode();
	sync();
}

void	MAPINT	cpuCycle (void) {
	switch(mapper) {
		case M_MMC1: MMC1::cpuCycle(); break;
		default:
		case M_MMC3: MMC3::cpuCycle(); break;
		case M_VRC4: VRC24::cpuCycle(); break;
	}
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (mapper ==M_MMC3) MMC3::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	offset =VRC24::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) {
		applyMode();
		sync();
	}
	return offset;
}

uint16_t mapperNum =351;
} // namespace

MapperInfo MapperInfo_351 ={
	&mapperNum,
	_T("Techline XB"),
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
