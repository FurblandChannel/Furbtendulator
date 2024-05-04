#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_Latch.h"

/*
412D	D3	Jumper cartridge
	D1	Volume + button (0=pressed)
	D0	Volume - button	(0=pressed)
4136	volume
*/
#define mode     (OneBus::reg4100[0x1D] &3)
#define chrram !!(OneBus::reg4100[0x1D] &4)
#define MODE_MMC3  0
#define MODE_MMC1  1
#define MODE_UNROM 2
#define MODE_CNROM 3

namespace {
void	sync () {
	int prgOR =OneBus::reg4100[0x2C] <<12 &0x1000 | OneBus::reg4100[0x2C] <<11 &0x2000 | OneBus::reg4100[0x2E] <<14 &0x4000;
	switch(mode) {
		case MODE_MMC3:
			OneBus::syncPRG(0x0FFF, prgOR);
			OneBus::syncMirror();
			break;
		case MODE_MMC1:
			OneBus::reg2000[0x16] =(MMC1::getCHRBank(0) <<2) |0;
			OneBus::reg2000[0x17] =(MMC1::getCHRBank(0) <<2) |2;
			OneBus::reg2000[0x12] =(MMC1::getCHRBank(1) <<2) |0;
			OneBus::reg2000[0x13] =(MMC1::getCHRBank(1) <<2) |1;
			OneBus::reg2000[0x14] =(MMC1::getCHRBank(1) <<2) |2;
			OneBus::reg2000[0x15] =(MMC1::getCHRBank(1) <<2) |3;
			OneBus::syncPRG16(MMC1::getPRGBank(0), MMC1::getPRGBank(1), 0x0FFF, prgOR);
			MMC1::syncMirror();
			break;
		case MODE_UNROM:
			OneBus::syncPRG16(Latch::data, 0xFF, 0x0FFF, prgOR);
			OneBus::syncMirror();
			break;
		case MODE_CNROM:
			OneBus::reg2000[0x16] =Latch::data <<3 |0;
			OneBus::reg2000[0x17] =Latch::data <<3 |2;
			OneBus::reg2000[0x12] =Latch::data <<3 |4;
			OneBus::reg2000[0x13] =Latch::data <<3 |5;
			OneBus::reg2000[0x14] =Latch::data <<3 |6;
			OneBus::reg2000[0x15] =Latch::data <<3 |7;
			OneBus::syncPRG(0x0FFF, prgOR);
			OneBus::syncMirror();
			break;
	}
	if (chrram) {
		EMU->SetCHR_RAM8(0x00, 0); // 2007
		EMU->SetCHR_RAM8(0x20, 0); // BG
		EMU->SetCHR_RAM8(0x28, 0); // SPR
	} else
		OneBus::syncCHR(0x7FFF, OneBus::reg4100[0x2C] <<14 &0x8000 | OneBus::reg4100[0x2C] <<13 &0x10000 | OneBus::reg4100[0x2E] <<17 &0x20000);
}

void	setMode() {
	switch(mode) {
		case MODE_MMC3:
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, OneBus::writeMMC3);
			break;
		case MODE_MMC1:
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
			break;
		case MODE_UNROM:
		case MODE_CNROM:
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, Latch::write);
			break;
	}
}

int	MAPINT	read4 (int bank, int addr) {
	if (addr ==0x12D)
		return ROM->dipValueSet? ROM->dipValue: 3;
	else
		return OneBus::readAPU(bank, addr);
}

int	MAPINT	read4Debug (int bank, int addr) {
	if (addr ==0x12D)
		return ROM->dipValueSet? ROM->dipValue: 3;
	else
		return OneBus::readAPUDebug(bank, addr);
}

FPPURead	readCHR;
int	MAPINT	descrambleCHR (int bank, int addr) {
	int result =readCHR(bank, addr);
	if (OneBus::reg4100[0x1E] &0xC0) result =result <<4 &0x90 | result >>4 &0x09 | result <<1 &0x44 | result >>1 &0x22;
	return result;
}

void	MAPINT	write4 (int bank, int addr, int val) {
	OneBus::writeAPU(bank, addr, val);
	if (addr ==0x11D) setMode();
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1A);
	Latch::load(sync, false);
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reg4100[0x1D] =0;
	OneBus::reg4100[0x2C] =0;
	OneBus::reg4100[0x2E] =0;
	MMC1::reset(resetType);
	Latch::reset(resetType);
	OneBus::reset(resetType);
	
	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);
	
	if (ROM->PRGROMSize >=256*1024*1024) {
		readCHR =EMU->GetPPUReadHandler(0x20);
		for (int bank =0; bank <8; bank++) {
			EMU->SetPPUReadHandler(0x00 +bank, descrambleCHR); // 2007
			EMU->SetPPUReadHandler(0x20 +bank, descrambleCHR); // BG
			EMU->SetPPUReadHandler(0x28 +bank, descrambleCHR); // SPR
		}
	}
}

void	MAPINT	cpuCycle() {
	if (mode ==MODE_MMC1) MMC1::cpuCycle(); else
	if (mode ==MODE_MMC3) OneBus::cpuCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (mode ==MODE_MMC3) OneBus::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =OneBus::saveLoad(stateMode, offset, data);
	offset =MMC1::saveLoad(stateMode, offset, data);
	offset =Latch::saveLoad_D(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) {
		setMode();
		sync();
	}
	return offset;
}

uint16_t mapperNum =296;
} // namespace


MapperInfo MapperInfo_296 = {
	&mapperNum,
	_T("V.R. Technology VT32"),
	COMPAT_FULL,
	load,
	reset,
	OneBus::unload,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};