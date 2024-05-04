#include	"h_VS.h"

namespace VS {
FCPURead _Read4, _Read4Second;
FCPUWrite _Write4, _Write4Second;
uint8_t ProtectionCount;

FCPURead	_Read6;
FCPURead	_Read6Debug;
FCPUWrite	_Write6;

int	MAPINT	Read6_2KiB (int bank, int addr) {
	return _Read6(bank &~1, addr &0x7FF);
}
int	MAPINT	Read6Debug_2KiB (int bank, int addr) {
	return _Read6Debug(bank &~1, addr &0x7FF);
}
void	MAPINT	Write6_2KiB (int bank, int addr, int val) {
	_Write6(bank &~1, addr &0x7FF, val);
}

void	Load (void) {
	if (ROM->INES_Flags &2) EMU->Set_SRAMSize(2048);
}

static const uint8_t TKOKey[32] = {
	0xFF,0xBF,0xB7,0x97,0x97,0x17,0x57,0x4F,0x6F,0x6B,0xEB,0xA9,0xB1,0x90,0x94,0x14,
	0x56,0x4E,0x6F,0x6B,0xEB,0xA9,0xB1,0x90,0xD4,0x5C,0x3E,0x26,0x87,0x83,0x13,0x51
};

static const uint8_t SuperXeviousKey[8] = {
	0x05, 0x01, 0x89, 0x37, 0x05, 0x00, 0xD1, 0x3E
};

int	MAPINT	Read5 (int Bank, int Addr) {
	switch(ROM->INES2_VSFlags) {
		case 1:	// Atari R.B.I. Baseball
			if (~Addr &1)
				ProtectionCount =0;
			else
				return (++ProtectionCount ==5)? 0xB4: 0x6F;
			break;
		case 2: // Vs. TKO Boxing
			if (~Addr &1)
				ProtectionCount =0;
			else
				return TKOKey[ProtectionCount++ &31];
			break;
		case 3: // Super Xevious
			if (Addr &0x400)
				return SuperXeviousKey[ProtectionCount++ &7];
			break;
		default:
			return *EMU->OpenBus;
	}
	return *EMU->OpenBus;
}

void	Reset (RESET_TYPE ResetType) {
	ProtectionCount = 0;

	_Read4 = EMU->GetCPUReadHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	
	_Read6 = EMU->GetCPUReadHandler(0x6);
	_Read6Debug = EMU->GetCPUReadHandlerDebug(0x6);
	_Write6 = EMU->GetCPUWriteHandler(0x6);

	EMU->SetCPUReadHandler     (0x6, Read6_2KiB);
	EMU->SetCPUReadHandlerDebug(0x6, Read6Debug_2KiB);
	EMU->SetCPUWriteHandler    (0x6, Write6_2KiB);
	EMU->SetCPUReadHandler     (0x7, Read6_2KiB);
	EMU->SetCPUReadHandlerDebug(0x7, Read6Debug_2KiB);
	EMU->SetCPUWriteHandler    (0x7, Write6_2KiB);

	EMU->SetPRG_RAM8(0x6, 0);
	if (VSDUAL) {
		EMU->SetCPUReadHandler     (0x16, Read6_2KiB);
		EMU->SetCPUReadHandlerDebug(0x16, Read6Debug_2KiB);
		EMU->SetCPUWriteHandler    (0x16, Write6_2KiB);
		EMU->SetCPUReadHandler     (0x17, Read6_2KiB);
		EMU->SetCPUReadHandlerDebug(0x17, Read6Debug_2KiB);
		EMU->SetCPUWriteHandler    (0x17, Write6_2KiB);

		EMU->SetPRG_RAM8(0x16, 0);
		EMU->SetIRQ2(0, 1);
		EMU->SetIRQ2(1, 1);
		_Write4 = EMU->GetCPUWriteHandler(0x4);
		_Read4Second = EMU->GetCPUReadHandler(0x14);
		_Write4Second = EMU->GetCPUWriteHandler(0x14);
		EMU->SetCPUWriteHandler(0x4, Write4);
		EMU->SetCPUReadHandler(0x14, Read4);
		EMU->SetCPUWriteHandler(0x14, Write4);
	}
	
	if (ROM->INES2_VSFlags >=VS_RBI && ROM->INES2_VSFlags <=VS_SUPERXEVIOUS) EMU->SetCPUReadHandler(0x5, Read5);	
}

void	Unload (void) {
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_LONG(mode, offset, data, ProtectionCount);
	return offset;
}

void	MAPINT	CPUCycle (void) {
}

int	MAPINT	Read4 (int Bank, int Addr) {
	uint8_t DipSwitch = ROM->dipValue &0xFF;
	uint8_t DipSwitch2=(ROM->dipValue >>8) &0xFF;
	int Val = (Bank &0x10)? _Read4Second(Bank, Addr): _Read4(Bank, Addr);
	if (Addr ==0x016) {
		Val &= 0x67;	// Preserve controllers and coins
		Val |= (Bank &0x10)? 0x80: 0x00;
		Val |= ((( (Bank &0x10)? DipSwitch2: DipSwitch) &0x03) <<3);
	} else if (Addr == 0x017) {
		Val &= 0x03;
		Val |= ((Bank &0x10)? DipSwitch2: DipSwitch) & 0xFC;
	}
	return Val;
}

void	MAPINT	Write4 (int Bank, int Addr, int Val) {
	if (Addr ==0x16) {
		EMU->SetIRQ2((Bank &0x10)? 0: 1, (Val &2)? 1: 0);
	}
	if (Bank &0x10)
		_Write4Second(Bank, Addr, Val);
	else
		_Write4(Bank, Addr, Val);
}

} // namespace VS