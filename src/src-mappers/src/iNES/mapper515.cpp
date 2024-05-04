//#define		VGMCAPTURE
#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\h_Mic.h"
#include	"..\Hardware\Sound\s_VRC7.h"
#ifdef		VGMCAPTURE
#include	"vgmcapture.h"
#endif

namespace {
uint8_t	ADCData;
#ifdef	VGMCAPTURE
class	VGMCapture *vgm;
int	opllReg =0;
#endif

void	Sync (void) {
	int externalROM =(Latch::data &0x80)? 0: 1;
	int ROMImageBank =(Latch::data &0x3F) + (externalROM? 0: (ROM->INES_PRGSize -64));
	EMU->SetPRG_ROM16(0x8, ROMImageBank);
	EMU->SetPRG_ROM16(0xC, ROM->INES_PRGSize -1);
	EMU->SetCHR_RAM8(0, 0);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, false);
	VRC7sound::Load(false);
	Mic::Load();
#ifdef	VGMCAPTURE
	FILE *F =fopen("515.VGM", "wb");
	vgm = new VGMCapture(F);
#endif
	return TRUE;
}

int	MAPINT	Read6 (int Bank, int Addr) {
	if (Addr ==3) { // SPI (ADC) data
		int Val =ADCData &0x80;
		ADCData <<=1;
		return Val;
	}
	return *EMU->OpenBus;
}

void	MAPINT	Write6 (int Bank, int Addr, int Val) {
	VRC7sound::Write(Addr &1, Val);
#ifdef	VGMCAPTURE
	if (~Addr &1)
		opllReg =Val;
	else
		vgm->ioWrite_YM2413(opllReg, Val);
#endif
}

void	MAPINT	WriteC (int Bank, int Addr, int Val) {
	switch(Addr) {
		case 2:	// Bit 7: SPI (ADC) chip select
			ADCData =Mic::Read() *64.0;
			break;
		case 3: // Bit 7: SPI clock / ADC conversion clock
			break;
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	VRC7sound::Reset(ResetType);
	Latch::reset(ResetType);
	EMU->SetCPUReadHandler(0x6, Read6);
	EMU->SetCPUWriteHandler(0x6, Write6);
	EMU->SetCPUWriteHandler(0xC, WriteC);
	Sync();	
}
void	MAPINT	Unload (void) {
	Mic::Unload();
	VRC7sound::Unload();
#ifdef	VGMCAPTURE
	delete vgm;
#endif
}

void	MAPINT	CPUCycle (void) {
#ifdef	VGMCAPTURE
	vgm->nextTick();
#endif
}

int	MAPINT	MapperSnd (int Cycles) {
	return VRC7sound::Get(Cycles) *2;
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset =VRC7sound::SaveLoad(mode, offset, data);
	offset =Latch::saveLoad_D(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, ADCData);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =515;
} // namespace

MapperInfo MapperInfo_515 ={
	&MapperNum,
	_T("Family Noraebang"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	CPUCycle,
	NULL,
	SaveLoad,
	MapperSnd,
	NULL
};