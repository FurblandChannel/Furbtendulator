#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_VRC24.h"

#define	modeMMC3 mode &1
#define	modeMMC1 mode &2
#define chrA18   mode <<6 &0x100

namespace {
uint8_t		mode;
uint8_t		game;
FCPUWrite	writeAPU;
FCPUWrite	writeNothing;

void	sync (void) {
	uint8_t	prgAND =ROM->INES2_SubMapper !=3? 0x3F: game? 0x0F: 0x1F;
	uint8_t	chrAND =game? 0x7F: 0xFF;
	uint8_t	prgOR =game? (game +1) *0x10: 0x00;
	int	chrOR =game? (game +1) *0x80: 0x00;
	
	if (modeMMC1) {
		MMC1::syncMirror();
		if (ROM->INES2_SubMapper ==2) {
			EMU->SetPRG_ROM16(0x8, MMC1::getPRGBank(0) >>1);
			EMU->SetPRG_ROM16(0xC, MMC1::getPRGBank(1) >>1);
		} else
			MMC1::syncPRG(prgAND >>1, prgOR >>1);
		MMC1::syncCHR_ROM(chrAND >>2, chrOR >>2);
	} else
	if (modeMMC3) {
		MMC3::syncMirror();
		MMC3::syncPRG(prgAND, prgOR);
		MMC3::syncCHR_ROM(chrAND, chrOR | chrA18);
	} else {
		VRC24::syncPRG(prgAND, prgOR);
		VRC24::syncCHR_ROM(chrAND, chrOR | chrA18);
		VRC24::syncMirror();
	}
}

void	applyMode (void) {
	if (modeMMC1) {
		EMU->SetIRQ(1);
		for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
		if (ROM->INES2_SubMapper !=1) MMC1::write(0x8, 0, 0x80);
	} else if (modeMMC3) {
		EMU->SetCPUWriteHandler(0x8, MMC3::writeReg);
		EMU->SetCPUWriteHandler(0x9, MMC3::writeReg);
		EMU->SetCPUWriteHandler(0xA, MMC3::writeMirroringWRAM);
		EMU->SetCPUWriteHandler(0xB, MMC3::writeMirroringWRAM);
		EMU->SetCPUWriteHandler(0xC, MMC3::writeIRQConfig);
		EMU->SetCPUWriteHandler(0xD, MMC3::writeIRQConfig);
		EMU->SetCPUWriteHandler(0xE, MMC3::writeIRQEnable);
		EMU->SetCPUWriteHandler(0xF, MMC3::writeIRQEnable);
	} else {
		EMU->SetIRQ(1);
		EMU->SetCPUWriteHandler(0x8, VRC24::writePRG);
		EMU->SetCPUWriteHandler(0x9, VRC24::writeMisc);
		EMU->SetCPUWriteHandler(0xA, VRC24::writePRG);
		EMU->SetCPUWriteHandler(0xB, VRC24::writeCHR);
		EMU->SetCPUWriteHandler(0xC, VRC24::writeCHR);
		EMU->SetCPUWriteHandler(0xD, VRC24::writeCHR);
		EMU->SetCPUWriteHandler(0xE, VRC24::writeCHR);
		EMU->SetCPUWriteHandler(0xF, writeNothing);
	}
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		mode =val;
		applyMode();
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, false, 0x01, 0x02, NULL, true, 0);
	MMC1::load(sync, MMC1Type::MMC1A);
	MMC3::load(sync);
	game =ROM->INES2_SubMapper ==3? 4: 0;
	switch (ROM->INES2_SubMapper) {
		default:
		case 0: MapperInfo_116.Description =_T("哥德 SOMARI-W, Huang-1"); break;
		case 1: MapperInfo_116.Description =_T("哥德 SOMARI-P, Huang-1"); break;
		case 2: MapperInfo_116.Description =_T("哥德 SOMARI-P, Huang-2"); break;
		case 3: MapperInfo_116.Description =_T("哥德 SL-FC5-1"); break;
	}
	if (ROM->INES2_SubMapper ==3) {
		MapperInfo_116.Description =_T("哥德 SL-FC5-1");
		game =4; // Will be increased to 0 by Reset
	} else {
		MapperInfo_116.Description =_T("哥德 SOMARI-P");
		game =0;
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	writeNothing =EMU->GetCPUWriteHandler(0xF);
	MMC1::reset(resetType);
	MMC3::reset(resetType);
	VRC24::reset(resetType); // Must be done before VRC24::chr[] are initialized
	if (resetType ==RESET_HARD) {
		// Game increases in power cycle
		if (ROM->INES2_SubMapper ==3) if (++game >4) game =0;
		mode =1; // Always start in MMC3 mode
		applyMode();
		VRC24::chr[0] =0xFF;
		VRC24::chr[1] =0xFF;
		VRC24::chr[2] =0xFF;
		VRC24::chr[3] =0xFF;
	}
	sync();
	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank<=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeMode);
}

void	MAPINT	cpuCycle (void) {
	if (modeMMC3)
		MMC3::cpuCycle();
	else
	if (modeMMC1)
		MMC1::cpuCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (modeMMC3) MMC3::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	offset =VRC24::saveLoad(stateMode, offset, data);
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	if (ROM->INES2_SubMapper ==3) SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) {
		applyMode();
		sync();
	}
	return offset;
}

uint16_t mapperNum =116;
} // namespace

MapperInfo MapperInfo_116 ={
	&mapperNum,
	_T("哥德 SOMARI-P"),
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