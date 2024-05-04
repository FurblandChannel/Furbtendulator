#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "CPU.h"
#include "PPU.h"
#include "AVI.h"
#include "GFX.h"
#include "Controllers.h"
#include "Sound.h"
#include "Settings.h"
#include "APU.h"
#include "OneBus.h"
#include "plugThruDevice.hpp"

# pragma comment(lib, "dsound.lib")
# pragma comment(lib, "dxguid.lib")

#define DMC_DEBUG 0

namespace APU {
#define CORRECT_DPCM 0
#if CORRECT_DPCM
bool    prgModified[128*1024*1024];
#endif

Filter::Butterworth	filterPCM(10, 1789773, 4800.0);
Filter::LPF_RC		filterPulse(10000.0 /1789773.0);

const	unsigned char	LengthCounts[32] = {
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,
	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E
};
const	signed char	SquareDuty[4][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 1},
	{ 0, 0, 0, 0, 0, 0, 1, 1},
	{ 0, 0, 0, 0, 1, 1, 1, 1},
	{ 1, 1, 1, 1, 1, 1, 0, 0},
};
const	signed char	SquareDutyUnbiased[4][8] = { // Scaled by 2*4
	{-1,-1,-1,-1,-1,-1,-1, 7},
	{-2,-2,-2,-2,-2,-2, 6, 6},
	{-4,-4,-4,-4, 4, 4, 4, 4},
	{ 2, 2, 2, 2, 2, 2,-6,-6},
};
const	signed char	TriangleDuty[32] = {
	15,14,13,12,11,10, 9, 8,
	 7, 6, 5, 4, 3, 2, 1, 0,
	 0, 1, 2, 3, 4, 5, 6, 7,
	 8, 9,10,11,12,13,14,15
};
const	unsigned long	NoiseFreqNTSC[16] = {
	0x004,0x008,0x010,0x020,0x040,0x060,0x080,0x0A0,
	0x0CA,0x0FE,0x17C,0x1FC,0x2FA,0x3F8,0x7F2,0xFE4,
};
const	unsigned long	NoiseFreqPAL[16] = {
	0x004,0x007,0x00E,0x01E,0x03C,0x058,0x076,0x094,
	0x0BC,0x0EC,0x162,0x1D8,0x2C4,0x3B0,0x762,0xEC2,
};
const	unsigned long	DPCMFreqNTSC[16] = {
	0x1AC,0x17C,0x154,0x140,0x11E,0x0FE,0x0E2,0x0D6,
	0x0BE,0x0A0,0x08E,0x080,0x06A,0x054,0x048,0x036,
};
const	unsigned long	DPCMFreqPAL[16] = {
	0x18E,0x162,0x13C,0x12A,0x114,0x0EC,0x0D2,0x0C6,
	0x0B0,0x094,0x084,0x076,0x062,0x04E,0x042,0x032,
};
const	int	FrameCyclesNTSC[5] = { 7456,14912,22370,29828,37280 };
const	int	FrameCyclesPAL[5] = { 8312,16626,24938,33252,41560 };

Channel::Channel() {
	Reset();
}

void Channel::Reset() {
	wavehold =LengthCtr =next_wavehold =LengthCtr1 =LengthCtr2 =0;
}

void Channel::Run() {
	wavehold =next_wavehold;
	if (LengthCtr1) {
		if (LengthCtr ==LengthCtr2) LengthCtr =LengthCtr1;
		LengthCtr1 =0;
	}
}

CSquare::CSquare(int _which):
	Channel::Channel(),
	whichSquare(_which) { 
}

void	CSquare::PowerOn () {
	Reset();
} 

void	CSquare::Reset () {
	Channel::Reset();
	volume = envelope = wavehold = duty = swpspeed = swpdir = swpstep = swpenab = 0;
	freq =Vol =CurD =Envelope =Pos =0;
	Enabled =ValidFreq =Active =EnvClk =SwpClk =FALSE;
	Cycles =EnvCtr =BendCtr =1;
}

void	CSquare::CheckActive (void) {
	ValidFreq = (freq >= 0x8) && ((swpdir) || !((freq + (freq >> swpstep)) & 0x800));
	Active = LengthCtr && ValidFreq;
	if (Settings::NonlinearMixing)
		Pos = Active? (SquareDuty[duty][CurD] *Vol): 0;
	else
		Pos = Active? (SquareDutyUnbiased[duty][CurD] *Vol): 0;
}

void	CSquare::Write (int Reg, unsigned char Val) {
	switch (Reg) {
	case 0:	volume =Val &0xF;
		envelope =Val &0x10;
		next_wavehold =Val &0x20;
		duty =(Val >>6) &0x3;
		if (Settings::SwapDutyCycles && (duty ==1 || duty ==2)) duty ^=3; // Flip 25% and 50% duty cycles
		Vol =envelope? volume: Envelope;
		break;
	case 1:	swpstep =Val &0x07;
		swpdir =Val &0x08;
		swpspeed =(Val >>4) &0x7;
		swpenab =Val &0x80;
		SwpClk =TRUE;
		break;
	case 2:	freq &=0x700;
		freq |=Val;
		break;
	case 3:	freq &=0xFF;
		freq |=(Val &0x7) <<8;
		if (Enabled) {
			LengthCtr1 = LengthCounts[(Val >> 3) & 0x1F];
			LengthCtr2 = LengthCtr;
		}
		if (RI.ConsoleType !=CONSOLE_UM6578) CurD =0;
		EnvClk =TRUE;
		break;
	case 4:	Enabled =Val? TRUE :FALSE;
		if (!Enabled) LengthCtr =0;
		break;
	}
	CheckActive();
}

void	CSquare::Run (void) {
	Channel::Run();
	if (!Cycles--) {
		Cycles =(freq <<1) +1;
		CurD =(CurD -1) &0x7;
		if (Active) {
			if (Settings::NonlinearMixing) Pos =SquareDuty[duty][CurD] *Vol;               else
			                               Pos =SquareDutyUnbiased[duty][CurD] *Vol;
		}
	}
}

void	CSquare::QuarterFrame (void) {
	if (EnvClk) {
		EnvClk =FALSE;
		Envelope =0xF;
		EnvCtr =volume;
	}
	else if (!EnvCtr--) {
		EnvCtr =volume;
		if (Envelope)
			Envelope--;
		else	
			Envelope =wavehold? 0xF: 0x0;
	}
	Vol =envelope? volume: Envelope;
	CheckActive();
}

void	CSquare::HalfFrame (void) {
	if (!BendCtr--) {
		BendCtr =swpspeed;
		if (swpenab && swpstep && ValidFreq) {
			int sweep = freq >>swpstep;
			if (whichSquare &1)
				freq +=swpdir? -sweep: sweep;
			else
				freq +=swpdir? ~sweep: sweep;
		}
	}
	if (SwpClk) {
		SwpClk =FALSE;
		BendCtr =swpspeed;
	}
	if (LengthCtr && !wavehold) LengthCtr--;
	CheckActive();
}

CTriangle::CTriangle():
	Channel::Channel() { 
}

void	CTriangle::PowerOn (void) {
	Reset();
	next_wavehold =LengthCtr1 =LengthCtr2 =0;
}

void	CTriangle::Reset (void) {
	// Don't call Channel::Reset, because next_wavehold, LengthCtr1 and LengthCtr2 are only resetted on power-on
	wavehold =LengthCtr =0;
	linear =freq =CurD =LinCtr =Pos =0;
	Enabled =Active =LinClk =FALSE;
	Cycles =1;
}

void	CTriangle::CheckActive (void) {
	Active = LengthCtr && LinCtr;
	if (freq >=4) Pos = TriangleDuty[CurD];
}

void	CTriangle::Write (int Reg, unsigned char Val) {
	switch (Reg) {
	case 0:	linear = Val & 0x7F;
		next_wavehold = (Val >> 7) & 0x1;
		break;
	case 2:	freq &= 0x700;
		freq |= Val;
		break;
	case 3:	freq &= 0xFF;
		freq |= (Val & 0x7) << 8;
		if (Enabled) {
			LengthCtr1 = LengthCounts[(Val >> 3) & 0x1F];
			LengthCtr2 = LengthCtr;
		}
		LinClk = TRUE;
		break;
	case 4:	Enabled = Val ? TRUE : FALSE;
		if (!Enabled) LengthCtr = 0;
		break;
	}
	CheckActive();
}

void	CTriangle::Run (void) {
	Channel::Run();
	if (!Cycles--) {
		Cycles = freq;
		if (LengthCtr && LinCtr) {
			CurD++;
			CurD &= 0x1F;
			if (freq >=4) Pos = TriangleDuty[CurD];
		}
	}
}

void	CTriangle::QuarterFrame (void) {
	if (LinClk)
		LinCtr = linear;
	else	
		if (LinCtr) LinCtr--;
	if (!wavehold) LinClk = FALSE;
	CheckActive();
}

void	CTriangle::HalfFrame (void) {
	if (LengthCtr && !wavehold) LengthCtr--;
	CheckActive();
}


CNoise::CNoise():
	Channel::Channel() { 
}

void	CNoise::PowerOn (void) {
	Reset();
}

void	CNoise::Reset (void) {
	Channel::Reset();
	volume = envelope = datatype = 0;
	freq = 0;
	Vol = 0;
	Envelope = 0;
	Enabled = FALSE;
	EnvClk = FALSE;
	Pos = 0;
	CurD = 1;
	Cycles = 1;
	EnvCtr = 1;
}

void	CNoise::Write (int Reg, unsigned char Val) {
	switch (Reg) {
	case 0:	volume = Val & 0x0F;
		envelope = Val & 0x10;
		next_wavehold = Val & 0x20;
		Vol = envelope ? volume : Envelope;
		if (LengthCtr) Pos = ((CurD & 0x4000) ?  0 : 1) * Vol;
		break;
	case 2:	freq = Val & 0xF;
		datatype = Val & 0x80;
		if (Settings::DisablePeriodicNoise) datatype =0;
		break;
	case 3:	if (Enabled) {
			LengthCtr1 = LengthCounts[(Val >> 3) & 0x1F];
			LengthCtr2 = LengthCtr;
		}
		EnvClk = TRUE;
		break;
	case 4:	Enabled = Val ? TRUE : FALSE;
		if (!Enabled) LengthCtr = 0;
		break;
	}
}

void	CNoise::Run (void) {
	Channel::Run();
	// this uses pre-decrement due to the lookup table
	if (!--Cycles) {
		Cycles = FreqTable[freq];
		if (datatype)
			CurD = (CurD << 1) | (((CurD >> 14) ^ (CurD >> 8)) & 0x1);
		else	
			CurD = (CurD << 1) | (((CurD >> 14) ^ (CurD >> 13)) & 0x1);
		if (LengthCtr) Pos = ((CurD & 0x4000) ?  0 : 1) * Vol;
	}
}

void	CNoise::QuarterFrame (void) {
	if (EnvClk) {
		EnvClk = FALSE;
		Envelope = 0xF;
		EnvCtr = volume;
	}
	else if (!EnvCtr--) {
		EnvCtr = volume;
		if (Envelope)
			Envelope--;
		else	Envelope = wavehold ? 0xF : 0x0;
	}
	Vol = envelope ? volume : Envelope;
	if (LengthCtr) Pos = ((CurD & 0x4000) ?  0 : 1) * Vol;
}

void	CNoise::HalfFrame (void) {
	if (LengthCtr && !wavehold) LengthCtr--;
}

CDPCM::CDPCM(int _which):
	Channel::Channel(),
	whichAPU(_which) {
}

void	CDPCM::PowerOn (void) {
	Reset();
} 

void	CDPCM::Reset (void) {
	Channel::Reset();
	freq = doirq = pcmdata = addr = len = 0;
	
	CurAddr = SampleLen = 0;
	silenced = TRUE;
	shiftreg = 1;
	buffer = 0;
	LengthCtr = 0;
	Pos = 0;
	Cycles = 1;
	bufempty = TRUE;
	fetching = FALSE;
	outbits = 8; baseAddr =0xC000;
	stop =0;
}

void	CDPCM::Write (int Reg, unsigned char Val) {
	switch (Reg) {
	case 0:	if ((Val &0xF) != freq) filterPCM.recalc(20, 1789773.0, 1789773.0 / FreqTable[Val & 0xF] * 0.5 *(NES::CurRegion ==Settings::REGION_DENDY && Settings::Dendy60Hz? 5.0/6.0: 1.0));
		freq = Val & 0xF;
		wavehold = (Val >> 6) & 0x1;
		doirq = Val >> 7;
		if (!doirq) CPU::CPU[whichAPU]->WantIRQ &= ~IRQ_DPCM;
		break;
	case 1:	pcmdata = Val & 0x7F;
		Pos = pcmdata;
		break;
	case 2:	addr = Val;
		break;
	case 3:	len = Val;
		break;
	case 4:	if (Settings::RemoveDPCMBuzz && Val && !freq && !wavehold && !doirq && !addr && !len) Val =0;	// prevent buzz on Eliminator Boat Duel
		if (Val) {
			if (!LengthCtr) {
				CurAddr = baseAddr | (addr << 6);
				LengthCtr = (len << 4) + 1;
				CPU::CPU[whichAPU]->EnableDMA |=DMA_PCM_LOAD;
				#if DMC_DEBUG
				EI.DbgOut(L"%08lld: Enable DMC via 4015", CPU::CPU[whichAPU]->CycleCount);
				#endif
			}
		} else {
			bufempty =TRUE;		
			fetching =FALSE;
			outbits =8;
			LengthCtr =0;
			if (CPU::CPU[whichAPU]->scheduleDMC || CPU::CPU[whichAPU]->EnableDMA &DMA_PCM) {
				#if DMC_DEBUG
				EI.DbgOut(L"%08lld: DMC disable within schedule", CPU::CPU[whichAPU]->CycleCount);
				#endif
				CPU::CPU[whichAPU]->EnableDMA |=DMA_HALT;
			} else {
				#if DMC_DEBUG
				EI.DbgOut(L"%08lld: DMC disable, no schedule", CPU::CPU[whichAPU]->CycleCount);
				#endif
			}
		}			
		CPU::CPU[whichAPU]->WantIRQ &= ~IRQ_DPCM;
		break;
	}
}

void	CDPCM::Run (int InternalClock) {
	// Don't call base class for this one
	// this uses pre-decrement due to the lookup table
	if (!--Cycles) {
		Cycles = FreqTable[freq];
		if (!silenced) {
			#if CORRECT_DPCM
			if (shiftreg &(0x01)) {
				if (pcmdata <= 0x7D)
					pcmdata += 2;
			} else {
				if (pcmdata >= 0x02)
					pcmdata -= 2;
			}
			shiftreg >>=1;
			#else
			if (shiftreg &(Settings::ReverseDPCM? 0x80: 0x01)) {
				if (pcmdata <= 0x7D)
					pcmdata += 2;
			} else {
				if (pcmdata >= 0x02)
					pcmdata -= 2;
			}
			if (Settings::ReverseDPCM)
				shiftreg <<=1;
			else
				shiftreg >>=1;
			#endif
			Pos = pcmdata;
		}
		if (!--outbits) {
			outbits =8;
			if (!bufempty) {
				shiftreg = buffer;
				bufempty = TRUE;
				silenced = FALSE;
			}
			else	silenced = TRUE;
		}
	}
	if (bufempty && !fetching && LengthCtr && ~CPU::CPU[whichAPU]->CycleCount &1) {
		fetching = TRUE;
		CPU::CPU[whichAPU]->scheduleDMC =2;
		#if DMC_DEBUG
		EI.DbgOut(L"%08lld: Buffer empty, scheduling DMC", CPU::CPU[whichAPU]->CycleCount);
		#endif
	}
}

void	CDPCM::Fetch (void) {
	#if CORRECT_DPCM
	int pcmBank =CurAddr >>12;
	int pcmAddr =CurAddr &0xFFF;
	uint8_t* pcmPtr =EI.GetPRG_Ptr4(pcmBank) +pcmAddr;
	if (!prgModified[pcmPtr -RI.PRGROMData] && Settings::ReverseDPCM) {
		prgModified[pcmPtr -RI.PRGROMData] =true;
		uint8_t c =*pcmPtr;
		c = c >>7 &0x01 | c >>5 &0x02 | c >>3 &0x04 | c >>1 &0x08 | c <<1 &0x10 | c <<3 &0x20 | c <<5 &0x40 | c <<7 &0x80;
		*pcmPtr =c;
	}
	#endif
	#if DMC_DEBUG
	EI.DbgOut(L"%08lld: DMC fetch", CPU::CPU[whichAPU]->CycleCount);
	#endif
	buffer = CPU::CPU[whichAPU]->MemGetA(CurAddr);
	CPU::CPU[whichAPU]->EnableDMA &=~DMA_PCM_LOAD;
	bufempty = FALSE;
	fetching = FALSE;
	if (++CurAddr ==0x10000) CurAddr =0x8000;
	if (!--LengthCtr) {
		if (wavehold) {
			CurAddr = baseAddr | (addr << 6);
			LengthCtr = (len << 4) + 1;
		} else
		if (doirq)
			CPU::CPU[whichAPU]->WantIRQ |= IRQ_DPCM;
	}
}

APU_RP2A03::APU_RP2A03 (int _which, int _numRegs):
	which(_which),
	numRegs(_numRegs),
	square0(0),
	square1(1),
	dpcm(new CDPCM(_which)) {
}

APU_RP2A03::APU_RP2A03 (int _which):
	APU_RP2A03(_which, 0x20) {
}

APU_RP2A03::APU_RP2A03():
	APU_RP2A03(0) {
}

APU_RP2A03::~APU_RP2A03() {
	delete dpcm;
}

void	APU_RP2A03::WriteFrame (unsigned char Val) {
	Bits =Val &0xC0;
	// if the write happens before an odd clock, the action is delayed by 3 cycles
	// otherwise, it's delayed by 2 cycles
	// it also takes an extra cycle for the loaded value to propagate into the decoder
	if (InternalClock &1)
		Zero =3;
	else	
		Zero =2;
	if (Bits &0x40) CPU::CPU[which]->WantIRQ &= ~IRQ_FRAME;
}

void	APU_RP2A03::RunFrame (void) {		
	if (Cycles ==CycleTable[0]) { 	// step A
		Quarter =2;
	} else
	if (Cycles ==CycleTable[1]) {	// step B
		Quarter =2;
		Half =2;
	} else
	if (Cycles ==CycleTable[2]) {	// step C
		Quarter = 2;
	} else
	if (Cycles ==CycleTable[3]) {	// step D
		if (!(Bits &0x80)) {
			Quarter =2;
			Half =2;
			IRQ =3;
			Cycles =-2;
		}
	} else
	if (Cycles ==CycleTable[4]) {	// step E
		Quarter =2;
		Half =2;
		Cycles =-2;
	}
	Cycles++;

	if (Quarter && !--Quarter) {
		square0.QuarterFrame();
		square1.QuarterFrame();
		triangle.QuarterFrame();
		noise.QuarterFrame();
	}
	if (Half && !--Half) {
		square0.HalfFrame();
		square1.HalfFrame();
		triangle.HalfFrame();
		noise.HalfFrame();
	}
	if (IRQ) {
		if (!Bits) CPU::CPU[which]->WantIRQ |= IRQ_FRAME;
		IRQ--;
	}
	if (Zero && !--Zero) {
		if (Bits & 0x80) {
			Quarter =2;
			Half =2;
		}
		Cycles =0;
	}
}

void	APU_RP2A03::IntWrite (int Bank, int Addr, int Val) {	
	if (Addr <numRegs) Regs[Addr] = Val;
	switch (Addr) {
	case 0x000:	square0.Write(0, Val);		break;
	case 0x001:	square0.Write(1, Val);		break;
	case 0x002:	square0.Write(2, Val);		break;
	case 0x003:	square0.Write(3, Val);
			break;
	case 0x004:	square1.Write(0, Val);		break;
	case 0x005:	square1.Write(1, Val);		break;
	case 0x006:	square1.Write(2, Val);		break;
	case 0x007:	square1.Write(3, Val);		
			break;
	case 0x008:	triangle.Write(0, Val);	break;
	case 0x00A:	triangle.Write(2, Val);	break;
	case 0x00B:	triangle.Write(3, Val);	break;
	case 0x00C:	noise.Write(0, Val);		break;
	case 0x00E:	noise.Write(2, Val);		break;
	case 0x00F:	noise.Write(3, Val);		break;
	case 0x010:	dpcm->Write(0, Val);		break;
	case 0x011:	if (!Settings::RemoveDPCMPops) dpcm->Write(1, Val);
			break;
	case 0x012:	dpcm->Write(2, Val);		break;
	case 0x013:	dpcm->Write(3, Val);		break;
	case 0x014:	CPU::CPU[which]->EnableDMA |= DMA_SPR;
			CPU::CPU[which]->DMAPage = Val;
			break;
	case 0x015:	square0.Write(4, Val & 0x1);
			square1.Write(4, Val & 0x2);
			triangle.Write(4, Val & 0x4);
			noise.Write(4, Val & 0x8);
			dpcm->Write(4, Val & 0x10);
			break;
	case 0x016:	if (VSDUAL) {
				if (which ==0) {
					Controllers::FSPort1->Write(Val &1);
					Controllers::FSPort2->Write(Val &1);
				} else {
					Controllers::FSPort3->Write(Val &1);
					Controllers::FSPort4->Write(Val &1);
				}
			} else 
				Controllers::Write(Val);
			break;
	case 0x017:	WriteFrame(Val);
			break;
	}
}

int	APU_RP2A03::IntRead (int Bank, int Addr) {
	int result = *EI.OpenBus;
	switch (Addr) 	{
	case 0x015:
		result =
			((      square0.LengthCtr) ? 0x01 : 0) |
			((      square1.LengthCtr) ? 0x02 : 0) |
			((     triangle.LengthCtr) ? 0x04 : 0) |
			((        noise.LengthCtr) ? 0x08 : 0) |
			((        dpcm->LengthCtr) ? 0x10 : 0) |
			((CPU::CPU[which]->WantIRQ & IRQ_FRAME) ? 0x40 : 0) |
			((CPU::CPU[which]->WantIRQ &  IRQ_DPCM) ? 0x80 : 0);
			CPU::CPU[which]->WantIRQ &= ~IRQ_FRAME;	// DPCM flag doesn't get reset
		break;
	case 0x016:
		result = CPU::CPU[which]->LastRead & 0xE0; // NES: E0, Famicom: F8
		if (RI.ConsoleType ==CONSOLE_VS) {
			result &=~0x64; // CoinA/B, service coin
			// Vs. Uni- or Dual System
			if (RI.INES2_VSFlags ==VS_DUAL || RI.INES2_VSFlags ==VS_BUNGELING) {
				// Vs. Dual System
				if (which ==0) {	// Left unit
					result |=Controllers::FSPort1->Read() &0x19;
					result |=NES::coin1;
				} else {		// Right unit
					result |=Controllers::FSPort3->Read() &0x19;
					result |=NES::coin2;
				}
			} else {
				// Vs. Unisystem
				result |=NES::coin1;
				result |=Controllers::Port1->Read() & 0x19;
				result |=Controllers::PortExp->Read1() & 0x1F;
			}
		} else {	// Normal console
			result |= Controllers::Port1->Read() & 0x19;
			result |= Controllers::PortExp->Read1() & 0x1F;
			result |= NES::mic;
			if (NES::micDelay && !(NES::micDelay &0xF)) NES::mic ^=0x04;
		}
		break;
	case 0x017:
		result = CPU::CPU[which]->LastRead & 0xE0;
		if (VSDUAL) {
			if (which ==0)
				result |= Controllers::FSPort2->Read() &0x19;
			else
				result |= Controllers::FSPort4->Read() &0x19;
		} else {
			result |= Controllers::Port2->Read() & 0x19;
			result |= Controllers::PortExp->Read2() & 0x1F;
		}
		if (RI.InputType ==INPUT_PEC_586|| RI.InputType ==INPUT_GOLDEN_KEYBOARD) result =result &~0x40 | result <<5 &0x40;
		if (RI.InputType ==INPUT_PORT_TEST_CONTROLLER) result ^=0x18;
		break;
	}
	return result;
}

void	APU_RP2A03::SetRegion (void) {
	switch (NES::CurRegion) {
	case Settings::REGION_NTSC:
		noise.FreqTable = NoiseFreqNTSC;
		dpcm->FreqTable = DPCMFreqNTSC;
		CycleTable = FrameCyclesNTSC;
		break;
	case Settings::REGION_PAL:
		noise.FreqTable = NoiseFreqPAL;
		dpcm->FreqTable = DPCMFreqPAL;
		CycleTable = FrameCyclesPAL;
		break;
	case Settings::REGION_DENDY:
		noise.FreqTable = NoiseFreqNTSC;
		dpcm->FreqTable = DPCMFreqNTSC;
		CycleTable = FrameCyclesNTSC;
		break;
	default:
		EI.DbgOut(_T("Invalid APU region selected!"));
		break;
	}
}

void	APU_RP2A03::PowerOn (void) {
	#if CORRECT_DPCM
	for (auto& c: prgModified) c =false;
	#endif
	
	ZeroMemory(Regs, sizeof(Regs));
	Bits =Cycles =Quarter =Half =IRQ =Zero =0;
	square0.PowerOn();
	square1.PowerOn();
	triangle.PowerOn();
	noise.PowerOn();
	dpcm->PowerOn();
	CPU::CPU[which]->WantIRQ &= ~(IRQ_FRAME | IRQ_DPCM);
	InternalClock = 0;
}

void	APU_RP2A03::Reset (void) {
	Cycles =Quarter =Half =IRQ =Zero =0;
	ZeroMemory(Regs, sizeof(Regs));
	if (Settings::BootWithDisabledFrameIRQ) { Bits =Regs[0x17] =0x40; }
	square0.Reset();
	square1.Reset();
	triangle.Reset();
	noise.Reset();
	dpcm->Reset();
	CPU::CPU[which]->WantIRQ &= ~(IRQ_FRAME | IRQ_DPCM);
	InternalClock = 0;
}

int	APU_RP2A03::Save (FILE *out) {
	#if CORRECT_DPCM
	FILE *F =fopen("D:\\PRGPCM.BIN", "wb");
	fwrite(&RI.PRGROMData[0], 1, RI.PRGROMSize, F);
	fclose(F);
	#endif
	
	int clen = 0;
	unsigned char tpc;
	tpc = Regs[0x15] & 0xF;
	writeByte(tpc);			//	uint8		Last value written to $4015, lower 4 bits

	writeByte(Regs[0x01]);		//	uint8		Last value written to $4001
	writeWord(square0.freq);	//	uint16		Square0 frequency
	writeByte(square0.LengthCtr);	//	uint8		Square0 timer
	writeByte(square0.CurD);	//	uint8		Square0 duty cycle pointer
	tpc = (square0.EnvClk ? 0x2 : 0x0) | (square0.SwpClk ? 0x1 : 0x0);
	writeByte(tpc);			//	uint8		Boolean flags for whether Square0 envelope(2)/sweep(1) needs a reload
	writeByte(square0.EnvCtr);	//	uint8		Square0 envelope counter
	writeByte(square0.Envelope);	//	uint8		Square0 envelope value
	writeByte(square0.BendCtr);	//	uint8		Square0 bend counter
	writeWord(square0.Cycles);	//	uint16		Square0 cycles
	writeByte(Regs[0x00]);		//	uint8		Last value written to $4000

	writeByte(Regs[0x05]);		//	uint8		Last value written to $4005
	writeWord(square1.freq);	//	uint16		Square1 frequency
	writeByte(square1.LengthCtr);	//	uint8		Square1 timer
	writeByte(square1.CurD);	//	uint8		Square1 duty cycle pointer
	tpc = (square1.EnvClk ? 0x2 : 0x0) | (square1.SwpClk ? 0x1 : 0x0);
	writeByte(tpc);			//	uint8		Boolean flags for whether Square1 envelope(2)/sweep(1) needs a reload
	writeByte(square1.EnvCtr);	//	uint8		Square1 envelope counter
	writeByte(square1.Envelope);	//	uint8		Square1 envelope value
	writeByte(square1.BendCtr);	//	uint8		Square1 bend counter
	writeWord(square1.Cycles);	//	uint16		Square1 cycles
	writeByte(Regs[0x04]);		//	uint8		Last value written to $4004

	writeWord(triangle.freq);	//	uint16		Triangle frequency
	writeByte(triangle.LengthCtr);	//	uint8		Triangle timer
	writeByte(triangle.CurD);	//	uint8		Triangle duty cycle pointer
	writeByte(triangle.LinClk);	//	uint8		Boolean flag for whether linear counter needs reload
	writeByte(triangle.LinCtr);	//	uint8		Triangle linear counter
	writeByte(triangle.Cycles);	//	uint16		Triangle cycles
	writeByte(Regs[0x08]);		//	uint8		Last value written to $4008

	writeByte(Regs[0x0E]);		//	uint8		Last value written to $400E
	writeByte(noise.LengthCtr);	//	uint8		Noise timer
	writeWord(noise.CurD);		//	uint16		Noise duty cycle pointer
	writeByte(noise.EnvClk);	//	uint8		Boolean flag for whether Noise envelope needs a reload
	writeByte(noise.EnvCtr);	//	uint8		Noise envelope counter
	writeByte(noise.Envelope);	//	uint8		Noise  envelope value
	writeWord(noise.Cycles);	//	uint16		Noise cycles
	writeByte(Regs[0x0C]);		//	uint8		Last value written to $400C

	writeByte(Regs[0x10]);		//	uint8		Last value written to $4010
	writeByte(Regs[0x11]);		//	uint8		Last value written to $4011
	writeByte(Regs[0x12]);		//	uint8		Last value written to $4012
	writeByte(Regs[0x13]);		//	uint8		Last value written to $4013
	writeWord(dpcm->CurAddr);	//	uint16		DPCM current address
	writeWord(dpcm->SampleLen);	//	uint16		DPCM current length
	writeByte(dpcm->shiftreg);	//	uint8		DPCM shift register
	tpc = (dpcm->fetching ? 0x4 : 0x0) | (dpcm->silenced ? 0x0 : 0x2) | (dpcm->bufempty ? 0x0 : 0x1);	// variables were renamed and inverted
	writeByte(tpc);			//	uint8		DPCM fetching(D2)/!silenced(D1)/!empty(D0)
	writeByte(dpcm->outbits);	//	uint8		DPCM shift count
	writeByte(dpcm->buffer);	//	uint8		DPCM read buffer
	writeWord(dpcm->Cycles);	//	uint16		DPCM cycles
	writeWord(dpcm->LengthCtr);	//	uint16		DPCM length counter

	writeByte(Regs[0x17]);		//	uint8		Last value written to $4017
	writeWord(Cycles);	//	uint16		Frame counter cycles

	tpc = CPU::CPU[which]->WantIRQ & (IRQ_DPCM | IRQ_FRAME);
	writeByte(tpc);			//	uint8		APU-related IRQs (PCM and FRAME, as-is)
	return clen;
}

int	APU_RP2A03::Load (FILE *in, int version_id) {
	int clen =0;
	unsigned char tpc;

	readByte(tpc);			//	uint8		Last value written to $4015, lower 4 bits
	IntWrite(0x4, 0x015, tpc);	// this will ACK any DPCM IRQ

	readByte(tpc);			//	uint8		Last value written to $4001
	IntWrite(0x4, 0x001, tpc);
	readWord(square0.freq);	//	uint16		Square0 frequency
	readByte(square0.LengthCtr);	//	uint8		Square0 timer
	readByte(square0.CurD);	//	uint8		Square0 duty cycle pointer
	readByte(tpc);			//	uint8		Boolean flags for whether Square0 envelope(2)/sweep(1) needs a reload
	square0.EnvClk = (tpc & 0x2);
	square0.SwpClk = (tpc & 0x1);
	readByte(square0.EnvCtr);	//	uint8		Square0 envelope counter
	readByte(square0.Envelope);	//	uint8		Square0 envelope value
	readByte(square0.BendCtr);	//	uint8		Square0 bend counter
	readWord(square0.Cycles);	//	uint16		Square0 cycles
	readByte(tpc);			//	uint8		Last value written to $4000
	IntWrite(0x4, 0x000, tpc);

	readByte(tpc);			//	uint8		Last value written to $4005
	IntWrite(0x4, 0x005, tpc);
	readWord(square1.freq);	//	uint16		Square1 frequency
	readByte(square1.LengthCtr);	//	uint8		Square1 timer
	readByte(square1.CurD);	//	uint8		Square1 duty cycle pointer
	readByte(tpc);			//	uint8		Boolean flags for whether Square1 envelope(2)/sweep(1) needs a reload
	square1.EnvClk = (tpc & 0x2);
	square1.SwpClk = (tpc & 0x1);
	readByte(square1.EnvCtr);	//	uint8		Square1 envelope counter
	readByte(square1.Envelope);	//	uint8		Square1 envelope value
	readByte(square1.BendCtr);	//	uint8		Square1 bend counter
	readWord(square1.Cycles);	//	uint16		Square1 cycles
	readByte(tpc);			//	uint8		Last value written to $4004
	IntWrite(0x4, 0x004, tpc);

	readWord(triangle.freq);	//	uint16		Triangle frequency
	readByte(triangle.LengthCtr);	//	uint8		Triangle timer
	readByte(triangle.CurD);	//	uint8		Triangle duty cycle pointer
	readByte(triangle.LinClk);	//	uint8		Boolean flag for whether linear counter needs reload
	readByte(triangle.LinCtr);	//	uint8		Triangle linear counter
	readByte(triangle.Cycles);	//	uint16		Triangle cycles
	readByte(tpc);			//	uint8		Last value written to $4008
	IntWrite(0x4, 0x008, tpc);

	readByte(tpc);			//	uint8		Last value written to $400E
	IntWrite(0x4, 0x00E, tpc);
	readByte(noise.LengthCtr);	//	uint8		Noise timer
	readWord(noise.CurD);		//	uint16		Noise duty cycle pointer
	readByte(noise.EnvClk);	//	uint8		Boolean flag for whether Noise envelope needs a reload
	readByte(noise.EnvCtr);	//	uint8		Noise envelope counter
	readByte(noise.Envelope);	//	uint8		Noise  envelope value
	readWord(noise.Cycles);	//	uint16		Noise cycles
	readByte(tpc);			//	uint8		Last value written to $400C
	IntWrite(0x4, 0x00C, tpc);

	readByte(tpc);			//	uint8		Last value written to $4010
	IntWrite(0x4, 0x010, tpc);
	readByte(tpc);			//	uint8		Last value written to $4011
	IntWrite(0x4, 0x011, tpc);
	readByte(tpc);			//	uint8		Last value written to $4012
	IntWrite(0x4, 0x012, tpc);
	readByte(tpc);			//	uint8		Last value written to $4013
	IntWrite(0x4, 0x013, tpc);
	readWord(dpcm->CurAddr);	//	uint16		DPCM current address
	readWord(dpcm->SampleLen);	//	uint16		DPCM current length
	readByte(dpcm->shiftreg);	//	uint8		DPCM shift register
	readByte(tpc);			//	uint8		DPCM fetching(D2)/!silenced(D1)/!empty(D0)
	dpcm->fetching = tpc & 0x4;
	dpcm->silenced = !(tpc & 0x2);	// variable was renamed and inverted
	dpcm->bufempty = !(tpc & 0x1);	// variable was renamed and inverted
	readByte(dpcm->outbits);	//	uint8		DPCM shift count
	readByte(dpcm->buffer);		//	uint8		DPCM read buffer
	readWord(dpcm->Cycles);		//	uint16		DPCM cycles
	readWord(dpcm->LengthCtr);	//	uint16		DPCM length counter

	readByte(tpc);			//	uint8		Frame counter bits (last write to $4017)
	IntWrite(0x4, 0x017, tpc);	// and this will ACK any frame IRQ
	readWord(Cycles);	//	uint16		Frame counter cycles
	if (version_id < 1001)
		readByte(_val);		//	uint8		Frame counter phase

	readByte(tpc);			//	uint8		APU-related IRQs (PCM and FRAME, as-is)
	CPU::CPU[which]->WantIRQ |= tpc;	// so we can reload them here
	return clen;
}

const double squareSumFactor[32] ={ 1.000000, 1.352455, 1.336216, 1.320361, 1.304878, 1.289755, 1.274978, 1.260535, 1.246416, 1.232610, 1.219107, 1.205896, 1.192968, 1.180314, 1.167927, 1.155796, 1.143915, 1.132276, 1.120871, 1.109693, 1.098737, 1.087994, 1.077460, 1.067127, 1.056991, 1.047046, 1.037286, 1.027706, 1.018302, 1.009068, 1.000000 };

void	APU_RP2A03::Run (void) {
	if (which ==0) Controllers::PortExp->CPUCycle();
	if (which ==0 && NES::micDelay && !--NES::micDelay) NES::mic =0;
	if (which ==0 && NES::coinDelay1 && !--NES::coinDelay1) NES::coin1 =0;
	if (which ==1 && NES::coinDelay2 && !--NES::coinDelay2) NES::coin2 =0;	
	if (which ==0) PlugThruDevice::checkButton();
	if (NES::CurRegion ==Settings::REGION_DENDY && Settings::Dendy60Hz && !(InternalClock %6))
		;
	else {
		RunFrame();
		square0.Run();
		square1.Run();
		triangle.Run();
		noise.Run();
		dpcm->Run(InternalClock);
	}
	InternalClock++;
	if (Sound::isEnabled && (!Controllers::capsLock || Controllers::scrollLock) && (which ==NES::WhichScreenToShow || Settings::VSDualScreens ==2)) {
		double	Pulse, TND;
		if (Settings::NonlinearMixing) {
			dpcm->Pos &=0x7F;
			Pulse =(square0.Pos |square1.Pos)? 95.88 /( (8128.0 /((square0.Pos + square1.Pos))) +100.0): 0.0;
			TND   =(triangle.Pos  |noise.Pos  |dpcm->Pos)? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0 +dpcm->Pos/22638.0) +100.0): 0.0;
		} else {
			dpcm->Pos &=0x7F;
			Pulse =(square0.Pos +square1.Pos)/30.0/4.0/2.0 *squareSumFactor[square0.Vol +square1.Vol] *0.258483;
			TND   =(triangle.Pos  |noise.Pos  |dpcm->Pos)? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0 +dpcm->Pos/22638.0) +100.0): 0.0;
		}
		#if DISABLE_ALL_FILTERS
		#else
		if (Settings::LowPassFilterAPU ==1) Pulse =filterPulse.process(Pulse +1e-15);
		#endif
		
		Output =Pulse +TND;
		#if SUPPORT_STEREO
		OutputL =Pulse;
		OutputR =TND;
		#endif
	}
}

APU_RP2A03 *APU[2];

void	SetRegion (void) {
	if (APU[0]) APU[0]->SetRegion();
	if (APU[1]) APU[1]->SetRegion();
}

void	PowerOn (void) {
	if (APU[0]) APU[0]->PowerOn();
	if (APU[1]) APU[1]->PowerOn();
}

void	Reset (void) {
	if (APU[0]) APU[0]->Reset();
	if (APU[1]) APU[1]->Reset();
}

int	MAPINT	IntRead (int Bank, int Addr) {
	return APU[Bank >>4]->IntRead(Bank, Addr);
}

void	MAPINT	IntWrite (int Bank, int Addr, int Val) {	
	APU[Bank >>4]->IntWrite(Bank, Addr, Val);
}

} // namespace APU