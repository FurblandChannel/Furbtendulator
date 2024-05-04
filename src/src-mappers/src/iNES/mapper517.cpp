#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\h_Mic.h"

namespace
{
int	ADCData, ADCHigh, ADCLow, state;
void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, -1);
	EMU->SetCHR_RAM8(0, 0);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	Mic::Load();
	return TRUE;
}

int	MAPINT	Read6 (int Bank, int Addr) {
	int result =0;
	if (Addr ==0) switch(state) {
		case 0:	state =1;
			result =0;
		case 1:	state =2;
			result =1;
		case 2: if (ADCLow  >0) {
				ADCLow--;
				result =1;
			} else {
				state =0;
				result =0;
			}
	} else {
		result =ADCHigh-- >0? 0: 1;
	}
	return result;
}

void	MAPINT	Write8 (int Bank, int Addr, int Val) {
	ADCData =Mic::Read() *63.0;
	ADCHigh = ADCData >>2;
	ADCLow  = 0x40 -ADCHigh -((ADCData &3) <<2);
	state =0;
	Latch::write(Bank, Addr, Val);
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(ResetType);
	EMU->SetCPUReadHandler(0x6, Read6);
	EMU->SetCPUWriteHandler(0x8, Write8);
	ADCData =0;
	state =0;
}
void	MAPINT	Unload (void) {
	Mic::Unload();
}

uint16_t MapperNum = 517;
} // namespace

MapperInfo MapperInfo_517 =
{
	&MapperNum,
	_T("Kkachi-wa Nolae Chingu"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};

/*
	6000 must be 0
	6000 must be 1
	6001 is 0 while sample is != 0, becomes 0 once it is 0 -> result << 2;
	6000 must be 0, or sample will continue to be read -> result >> 2
*/