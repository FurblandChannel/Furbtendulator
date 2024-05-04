#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
uint8_t		reg4242;

void	sync () { 
	int OR =0;
	switch(ROM->INES2_SubMapper) {
		case 1:  OR = OneBus::reg4100[0x2C] &0x02? 0x0800: 0x0000;
		         break;
		case 2:  OR =(OneBus::reg4100[0x2C] &0x02? 0x0800: 0x0000) |
		             (OneBus::reg4100[0x2C] &0x01? 0x1000: 0x0000);
			 break;
		case 3:  OR = OneBus::reg4100[0x2C] &0x04? 0x0800: 0x0000;
		         break;
		default: OR =(OneBus::reg4100[0x2C] &0x06? 0x0800: 0x0000) |
		             (OneBus::reg4100[0x2C] &0x01? 0x1000: 0x0000);
			 break;
	}
	OneBus::syncPRG(0x07FF, OR);
	if (reg4242 &1) {
		EMU->SetCHR_RAM8(0x00, 0); // 2007
		EMU->SetCHR_RAM8(0x20, 0); // BG
		EMU->SetCHR_RAM8(0x28, 0); // SPR
	} else
		OneBus::syncCHR(0x3FFF, OR <<3);
	OneBus::syncMirror();
}

int	MAPINT	read4 (int bank, int addr) {
	if (addr ==0x12C)
		return ROM->dipValue;
	else
		return OneBus::readAPU(bank, addr);
}
int	MAPINT	read4Debug (int bank, int addr) {
	if (addr ==0x12C)
		return ROM->dipValue;
	else
		return OneBus::readAPUDebug(bank, addr);
}
void	MAPINT	write4 (int bank, int addr, int val) {
	if (addr ==0x242) {
		reg4242 =val;
		sync();
	} else
		OneBus::writeAPU(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg4242 =0;
	OneBus::reg4100[0x2C] =0;
	OneBus::reset(resetType);

	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =OneBus::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg4242);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =270;
} // namespace

MapperInfo MapperInfo_270 = {
	&mapperNum,
	_T("412C Bankswitch"),
	COMPAT_FULL,
	load,
	reset,
	OneBus::unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	saveLoad,
	NULL,
	NULL
};