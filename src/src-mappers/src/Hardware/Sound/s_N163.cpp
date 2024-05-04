#include	"..\..\interface.h"
#include	"s_N163.h"

/* There are two rendering modes: "original", and "clean".
   "Original" runs the chip at 1.78 MHz and each channel at 1.78 MHz / (channels*15), yielding a sampling rate of just 14,914 Hz in the case of eight channels.
   "Clean" runs the each channel at 1.78 MHz. This requires additional bits of Phase information. */
namespace N163sound {
uint8_t ChipRAM[128];
uint8_t Index; 		// Current Index into chip RAM
uint8_t CurrentChannel;	// Current channel being serviced
uint8_t	CurrentTick;	// Current CurrentTick (0-14) within current channel being serviced
uint8_t	PhaseMSB[8];	// Eight more bits of phase position per channel for "clean" rendering.

int	GenerateWaveClean (int Cycles, int ChannelOffset, int ChannelNum) {
	int Channels = (ChipRAM[0x7F] >>4)+1;
	int Phase    = (PhaseMSB[ChannelNum] <<24) | (ChipRAM[ChannelOffset +5] <<16) | (ChipRAM[ChannelOffset+3] <<8) | ChipRAM[ChannelOffset+1];
	int Freq     = ((ChipRAM[ChannelOffset +4] &3) <<16) | (ChipRAM[ChannelOffset+2] <<8) | (ChipRAM[ChannelOffset+0]);
	int Length   = 256 -(ChipRAM[ChannelOffset +4] &0xFC);
	int Offset   = ChipRAM[ChannelOffset +6];
	int Volume   = ChipRAM[ChannelOffset +7] &0xF;
	
	while (Cycles--)
		Phase =(Phase +Freq) % (Length *(65536 *15 *Channels));

	int sample=(Phase /(65536 *15 *Channels) + Offset) &0xFF;
	int output= (ChipRAM[sample >>1] >>((sample&1)<<2)) &0x0F;
	
	// update Phase
	PhaseMSB[ChannelNum      ] =(Phase >>24) &0xFF;
	ChipRAM [ChannelOffset +5] =(Phase >>16) &0xFF;
	ChipRAM [ChannelOffset +3] =(Phase >> 8) &0xFF;
	ChipRAM [ChannelOffset +1] =(Phase >> 0) &0xFF;
	return (output -8) *Volume;
}

int	GenerateWaveOriginal (int Cycles, int ChannelOffset) {
	int Phase = (ChipRAM[ChannelOffset +5] <<16) | (ChipRAM[ChannelOffset+3] <<8) | ChipRAM[ChannelOffset+1];
	int Freq  = ((ChipRAM[ChannelOffset +4] &3) <<16) | (ChipRAM[ChannelOffset+2] <<8) | (ChipRAM[ChannelOffset+0]);
	int Length= 256 -(ChipRAM[ChannelOffset +4] &0xFC);
	int Offset= ChipRAM[ChannelOffset +6];
	int Volume= ChipRAM[ChannelOffset +7] &0xF;
	
	while (Cycles--)
		Phase =(Phase +Freq) % (Length <<16);

	int sample=((Phase >> 16) + Offset) &0xFF;			
	int output= (ChipRAM[sample >>1] >>((sample&1)<<2)) &0x0F;
	
	// update Phase
	ChipRAM[ChannelOffset +5] =(Phase >>16) &0xFF;
	ChipRAM[ChannelOffset +3] =(Phase >> 8) &0xFF;
	ChipRAM[ChannelOffset +1] =(Phase >> 0) &0xFF;
	return (output -8) *Volume;
}

void	Load (void) {
	ZeroMemory(ChipRAM, 128);
	ZeroMemory(PhaseMSB, 8);
	Index =CurrentChannel =CurrentTick =0;
}

void	Reset (RESET_TYPE ResetType) {
}

void	Unload (void) {
}

void	Write (int Addr, int Val) {
	switch (Addr & 0xF800) {
		case 0xF800:
			Index =Val;
			break;
		case 0x4800:
			//EMU->DbgOut(L"%02X=%02X", Index, Val);
			ChipRAM[Index &0x7F] = Val;
			if (Index &0x80) Index =(Index +1) |0x80;
			break;
	}
}

int	Read (int Addr) {
	int data = ChipRAM[Index &0x7F];
	if (Index &0x80) Index =(Index +1) |0x80;
	return data;
}

int	MAPINT	GetClean (int Cycles) {
	int out =0;
	for (int Cycle =0; Cycle <Cycles; Cycle++)
		for (CurrentChannel =(~ChipRAM[0x7F] >>4) &7; CurrentChannel <=7; CurrentChannel++)
			out += GenerateWaveClean(1, CurrentChannel*8 +0x40, CurrentChannel);
	return out *128 /Cycles / ((ChipRAM[0x7F] >>4)+1);
}

int	MAPINT	GetOriginal (int Cycles) {
	int out =0;
	for (int Cycle =0; Cycle <Cycles; Cycle++) {
		if (CurrentChannel >7) CurrentChannel = (~ChipRAM[0x7F] >>4) &7;
		out += GenerateWaveOriginal(CurrentTick? 0: 1, CurrentChannel*8 +0x40);
		if (++CurrentTick >=15) {
			CurrentTick =0;
			CurrentChannel++;
		}
	}
	return out *128 /Cycles;
}

int	MAPINT	Get (int Cycles) {
	if (*EMU->CleanN163)
		return GetClean(Cycles);
	else
		return GetOriginal(Cycles);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int Offset, unsigned char *data) {
	for (int i =0; i <128; i++) SAVELOAD_BYTE(mode, Offset, data, ChipRAM[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, Offset, data, PhaseMSB[i]);
	SAVELOAD_BYTE(mode, Offset, data, Index);
	SAVELOAD_BYTE(mode, Offset, data, CurrentChannel);
	SAVELOAD_BYTE(mode, Offset, data, CurrentTick);
	return Offset;
}
} // namespace N163sound