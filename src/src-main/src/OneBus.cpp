#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "Sound.h"
#include "APU.h"
#include "CPU.h"
#include "PPU.h"
#include "AVI.h"
#include "GFX.h"
#include "Controllers.h"
#include "Filter.h"
#include "Settings.h"
#include "Debugger.h"
#include "OneBus.h"
#include "OneBus_VT32.h"
#include "OneBus_VT369.h"

	uint8_t	reg2000[256];
	uint8_t	reg4100[256];
	
namespace CPU {
CPU_OneBus::CPU_OneBus(uint8_t* _RAM):
	CPU_RP2A03(_RAM) {
}

CPU_OneBus::CPU_OneBus(int which, int RAMSize, uint8_t* _RAM):
	CPU_RP2A03(which, RAMSize, _RAM) {
}

void	CPU_OneBus::PowerOn (void) {
	CPU_RP2A03::PowerOn();
	EncryptionStatus =true;
}

void	CPU_OneBus::Reset (void) {
	CPU_RP2A03::Reset();
	EncryptionStatus =(RI.INES2_SubMapper >=12)? true: false;
}

uint8_t	CPU_OneBus::GetOpcode(void) {
	uint8_t result =CPU_RP2A03::GetOpcode();
	return Unscramble(result);
}

uint8_t CPU_OneBus::Unscramble(uint8_t opcode) {
	int result =opcode;
	if (EncryptionStatus) {
		if (RI.INES2_SubMapper ==12) {
			result &=~0xC6;
			result |=(opcode &0x40)? 0x80: 0x00;
			result |=(opcode &0x80)? 0x40: 0x00;
			result |=(opcode &0x02)? 0x04: 0x00;
			result |=(opcode &0x04)? 0x02: 0x00;
		} else
		if (RI.INES2_SubMapper ==13) {
			result &=~0x12;
			result |=(opcode &0x10)? 0x02: 0x00;
			result |=(opcode &0x02)? 0x10: 0x00;
		} else
		if (RI.INES2_SubMapper ==14) {
			result &=~0xC0;
			result |=(opcode &0x80)? 0x40: 0x00;
			result |=(opcode &0x40)? 0x80: 0x00;
		} else {
			result &=~0x60;
			result |=(opcode &0x40)? 0x20: 0x00;
			result |=(opcode &0x20)? 0x40: 0x00;
		}
	} else
		result =opcode;
	return result;
}

int	CPU_OneBus::Save (FILE *out) {
	int clen =CPU_RP2A03::Save(out);
	writeBool(EncryptionStatus);
	return clen;
}

int	CPU_OneBus::Load (FILE *in, int version_id) {
	int clen =CPU_RP2A03::Load(in, version_id);
	readBool(EncryptionStatus);
	return clen;
}

CPU_VT09::CPU_VT09(uint8_t* _RAM):
	CPU_OneBus(0, 4096, _RAM) {
}
} // namespace CPU

namespace APU {
Filter::LPF_RC filterPulse2(10000.0 /1789773.0);

CDPCM_OneBus::CDPCM_OneBus (void):
	CDPCM::CDPCM(0) {
}

void	CDPCM_OneBus::Reset (void) {
	CDPCM::Reset();
	RawPCM =false;
}

void	CDPCM_OneBus::Write (int Reg, unsigned char Val) {
	switch (Reg) {
	case 5:	baseAddr = ((~Val) &3) <<14;
		RawPCM =(Val &0x10)? true: false;
		break;
	case 6:	pcmdata =Val;
		Pos =pcmdata -0x80;
		break;
	default:CDPCM::Write (Reg, Val);
		break;
	}
}

void	CDPCM_OneBus::Run (int InternalClock) {
	if (!RawPCM)
		CDPCM::Run(InternalClock);
	else {
		if (!--Cycles) {
			Cycles = FreqTable[freq];
			pcmdata =shiftreg;
			Pos = pcmdata -0x80;
			if (outbits <8) outbits =8;
			outbits -=8;
			if (!outbits) {
				outbits = 8;
				if (!bufempty)
				{
					shiftreg = buffer;
					bufempty = TRUE;
					silenced = FALSE;
				}
				else	silenced = TRUE;
			}
		}
		if (bufempty && !fetching && LengthCtr && ~CPU::CPU[whichAPU]->CycleCount &1) 	{
			fetching = TRUE;
			CPU::CPU[whichAPU]->scheduleDMC =2;
			// decrement LengthCtr now, so $4015 reads are updated in time
			//LengthCtr--;
		}
	}
}

APU_OneBus::APU_OneBus():
	APU_RP2A03(0, 0x40),
	square2(0),
	square3(1) {
	delete dpcm;
	dpcm =new CDPCM_OneBus();
}

void	APU_OneBus::RunFrame (void) {
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
		if (Regs[0x030] &0x08) {
			square2.QuarterFrame();
			square3.QuarterFrame();
			triangle2.QuarterFrame();
			noise2.QuarterFrame();
		}
	}
	if (Half && !--Half) {
		square0.HalfFrame();
		square1.HalfFrame();
		triangle.HalfFrame();
		noise.HalfFrame();
		if (Regs[0x030] &0x08) {
			square2.HalfFrame();
			square3.HalfFrame();
			triangle2.HalfFrame();
			noise2.HalfFrame();
		}
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

void	APU_OneBus::IntWrite (int Bank, int Addr, int Val) {
	// Remember values
	if (Addr <numRegs) Regs[Addr] = Val;
	if ((Addr &0xF00) ==0x100) reg4100[Addr &0xFF] =Val;

	// If the second APU is being accessed but not enabled, enable it. Necessary for Waixing's "Final Man".
	if (Addr >=0x020 && Addr <=0x02F) Regs[0x030] |=0x08;

	switch (Addr) {
	case 0x020:	square2.Write(0, Val);	break;
	case 0x021:	square2.Write(1, Val);	break;
	case 0x022:	square2.Write(2, Val);	break;
	case 0x023:	square2.Write(3, Val);	break;
	case 0x024:	square3.Write(0, Val);	break;
	case 0x025:	square3.Write(1, Val);	break;
	case 0x026:	square3.Write(2, Val);	break;
	case 0x027:	square3.Write(3, Val);	break;
	case 0x028:	triangle2.Write(0, Val);break;
	case 0x02A:	triangle2.Write(2, Val);break;
	case 0x02B:	triangle2.Write(3, Val);break;
	case 0x02C:	noise2.Write(0, Val);	break;
	case 0x02E:	noise2.Write(2, Val);	break;
	case 0x02F:	noise2.Write(3, Val);	break;
	case 0x030:	dpcm->Write(5, Val);	break;
	case 0x031:	dpcm->Write(6, Val);	break;
	case 0x034:	{
			int shift =Val >>1 &7;
			if (shift ==0) shift =8;
			CPU::CPU[which]->DMAMiddleAddr =Val &0xF0;
			CPU::CPU[which]->DMALength =1 <<shift;
			CPU::CPU[which]->DMATarget =(Val &1)? 0x2007: 0x2004;
			break;
			}
	case 0x035:	square2.Write(4, Val & 0x1);
			square3.Write(4, Val & 0x2);
			triangle2.Write(4, Val & 0x4);
			noise2.Write(4, Val & 0x8);
			break;
	case 0x11C:	if (RI.INES2_SubMapper ==12 || RI.INES2_SubMapper ==14) dynamic_cast<CPU::CPU_OneBus*>(CPU::CPU[which])->EncryptionStatus =(Val &0x40)? true: false;
			break;
	case 0x169:	if (RI.INES2_SubMapper ==13 || RI.INES2_SubMapper ==15)
				dynamic_cast<CPU::CPU_OneBus*>(CPU::CPU[which])->EncryptionStatus =(Val &1)? false: true;
			break;
	default:	APU_RP2A03::IntWrite(Bank, Addr, Val);
			break;
	}
}

int	APU_OneBus::IntRead (int Bank, int Addr) {
	int result =0xFF;
	switch (Addr) {
		case 0x035:
			result =(  square2.LengthCtr? 1: 0) |
				(  square3.LengthCtr? 2: 0) |
				(triangle2.LengthCtr? 4: 0) |
				(   noise2.LengthCtr? 8: 0);
			break;
		case 0x10E:
			result =((~reg4100[0x0D] &0x01 && reg4100[0x0D] &0x02? Controllers::PortExp->ReadIOP(0): 0x0F) &0x0F) <<0 |
			        ((~reg4100[0x0D] &0x04 && reg4100[0x0D] &0x08? Controllers::PortExp->ReadIOP(1): 0x0F) &0x0F) <<4;
			break;
		case 0x10F:
			result =((~reg4100[0x0D] &0x10 && reg4100[0x0D] &0x20? Controllers::PortExp->ReadIOP(2): 0x0F) &0x0F) <<0 |
			        ((~reg4100[0x0D] &0x40 && reg4100[0x0D] &0x80? Controllers::PortExp->ReadIOP(3): 0x0F) &0x0F) <<4;
			break;
		case 0x119:
			switch (NES::CurRegion) {
				case Settings::REGION_NTSC:	result =0x00; break;
				case Settings::REGION_PAL:	result =0x18; break;
				case Settings::REGION_DENDY:	result =0x18; break; // Shouldn't it be 0x10? Cannot be, because some games only check for 0x18
				default:			result =0x18; break;
			}
			break;
		default:
			result =APU_RP2A03::IntRead(Bank, Addr);
			break;
	}
	return result;
}

void	APU_OneBus::SetRegion (void) {
	APU_RP2A03::SetRegion();
	noise2.FreqTable =noise.FreqTable;
	filterPCM.recalc(20, 1789773.0, 1789773.0 / dpcm->FreqTable[dpcm->freq] * 0.5 *(NES::CurRegion ==Settings::REGION_DENDY && Settings::Dendy60Hz? 5.0/6.0: 1.0));
}

void	APU_OneBus::PowerOn (void) {
	APU_RP2A03::PowerOn();
	square2.PowerOn();
	square3.PowerOn();
	triangle2.PowerOn();
	noise2.PowerOn();
	for (auto& c: reg4100) c =0;
}

void	APU_OneBus::Reset (void) {
	APU_RP2A03::Reset();
	square2.Reset();
	square3.Reset();
	triangle2.Reset();
	noise2.Reset();
}

int	APU_OneBus::Save (FILE *out) {
	int clen =APU_RP2A03::Save(out);
	unsigned char tpc;
	writeByte(Regs[0x21]);		//	uint8		Last value written to $4021
	writeWord(square2.freq);	//	uint16		Square0 frequency
	writeByte(square2.LengthCtr);	//	uint8		Square0 timer
	writeByte(square2.CurD);	//	uint8		Square0 duty cycle pointer
	tpc = (square2.EnvClk ? 0x2 : 0x0) | (square2.SwpClk ? 0x1 : 0x0);
	writeByte(tpc);			//	uint8		Boolean flags for whether Square0 envelope(2)/sweep(1) needs a reload
	writeByte(square2.EnvCtr);	//	uint8		Square0 envelope counter
	writeByte(square2.Envelope);	//	uint8		Square0 envelope value
	writeByte(square2.BendCtr);	//	uint8		Square0 bend counter
	writeWord(square2.Cycles);	//	uint16		Square0 cycles
	writeByte(Regs[0x20]);		//	uint8		Last value written to $4020

	writeByte(Regs[0x25]);		//	uint8		Last value written to $4025
	writeWord(square3.freq);	//	uint16		Square1 frequency
	writeByte(square3.LengthCtr);	//	uint8		Square1 timer
	writeByte(square3.CurD);	//	uint8		Square1 duty cycle pointer
	tpc = (square3.EnvClk ? 0x2 : 0x0) | (square3.SwpClk ? 0x1 : 0x0);
	writeByte(tpc);			//	uint8		Boolean flags for whether Square1 envelope(2)/sweep(1) needs a reload
	writeByte(square3.EnvCtr);	//	uint8		Square1 envelope counter
	writeByte(square3.Envelope);	//	uint8		Square1 envelope value
	writeByte(square3.BendCtr);	//	uint8		Square1 bend counter
	writeWord(square3.Cycles);	//	uint16		Square1 cycles
	writeByte(Regs[0x24]);		//	uint8		Last value written to $4004

	writeWord(triangle2.freq);	//	uint16		Triangle frequency
	writeByte(triangle2.LengthCtr);//	uint8		Triangle timer
	writeByte(triangle2.CurD);	//	uint8		Triangle duty cycle pointer
	writeByte(triangle2.LinClk);	//	uint8		Boolean flag for whether linear counter needs reload
	writeByte(triangle2.LinCtr);	//	uint8		Triangle linear counter
	writeByte(triangle2.Cycles);	//	uint16		Triangle cycles
	writeByte(Regs[0x28]);		//	uint8		Last value written to $4028

	writeByte(Regs[0x2E]);		//	uint8		Last value written to $402E
	writeByte(noise2.LengthCtr);	//	uint8		Noise timer
	writeWord(noise2.CurD);		//	uint16		Noise duty cycle pointer
	writeByte(noise2.EnvClk);	//	uint8		Boolean flag for whether Noise envelope needs a reload
	writeByte(noise2.EnvCtr);	//	uint8		Noise envelope counter
	writeByte(noise2.Envelope);	//	uint8		Noise  envelope value
	writeWord(noise2.Cycles);	//	uint16		Noise cycles
	writeByte(Regs[0x2C]);		//	uint8		Last value written to $402C

	writeByte(Regs[0x30]);		//	uint8		Last value written to $4010
	writeByte(Regs[0x31]);		//	uint8		Last value written to $4011
	writeWord(dpcm->baseAddr);	//	uint16
	writeBool(dynamic_cast<CDPCM_OneBus*>(dpcm)->RawPCM);	//	uint8
	return clen;
}

int	APU_OneBus::Load (FILE *in, int version_id) {
	int clen =APU_RP2A03::Load(in, version_id);
	unsigned char tpc;
	readByte(tpc);			//	uint8		Last value written to $4021
	IntWrite(0x4, 0x021, tpc);
	readWord(square2.freq);	//	uint16		Square0 frequency
	readByte(square2.LengthCtr);	//	uint8		Square0 timer
	readByte(square2.CurD);	//	uint8		Square0 duty cycle pointer
	readByte(tpc);			//	uint8		Boolean flags for whether Square0 envelope(2)/sweep(1) needs a reload
	square2.EnvClk = (tpc & 0x2);
	square2.SwpClk = (tpc & 0x1);
	readByte(square2.EnvCtr);	//	uint8		Square0 envelope counter
	readByte(square2.Envelope);	//	uint8		Square0 envelope value
	readByte(square2.BendCtr);	//	uint8		Square0 bend counter
	readWord(square2.Cycles);	//	uint16		Square0 cycles
	readByte(tpc);			//	uint8		Last value written to $4020
	IntWrite(0x4, 0x020, tpc);

	readByte(tpc);			//	uint8		Last value written to $4025
	IntWrite(0x4, 0x025, tpc);
	readWord(square3.freq);	//	uint16		Square1 frequency
	readByte(square3.LengthCtr);	//	uint8		Square1 timer
	readByte(square3.CurD);	//	uint8		Square1 duty cycle pointer
	readByte(tpc);			//	uint8		Boolean flags for whether Square1 envelope(2)/sweep(1) needs a reload
	square3.EnvClk = (tpc & 0x2);
	square3.SwpClk = (tpc & 0x1);
	readByte(square3.EnvCtr);	//	uint8		Square1 envelope counter
	readByte(square3.Envelope);	//	uint8		Square1 envelope value
	readByte(square3.BendCtr);	//	uint8		Square1 bend counter
	readWord(square3.Cycles);	//	uint16		Square1 cycles
	readByte(tpc);			//	uint8		Last value written to $4024
	IntWrite(0x4, 0x024, tpc);

	readWord(triangle2.freq);	//	uint16		Triangle frequency
	readByte(triangle2.LengthCtr);	//	uint8		Triangle timer
	readByte(triangle2.CurD);	//	uint8		Triangle duty cycle pointer
	readByte(triangle2.LinClk);	//	uint8		Boolean flag for whether linear counter needs reload
	readByte(triangle2.LinCtr);	//	uint8		Triangle linear counter
	readByte(triangle2.Cycles);	//	uint16		Triangle cycles
	readByte(tpc);			//	uint8		Last value written to $4028
	IntWrite(0x4, 0x028, tpc);

	readByte(tpc);			//	uint8		Last value written to $402E
	IntWrite(0x4, 0x02E, tpc);
	readByte(noise2.LengthCtr);	//	uint8		Noise timer
	readWord(noise2.CurD);		//	uint16		Noise duty cycle pointer
	readByte(noise2.EnvClk);	//	uint8		Boolean flag for whether Noise envelope needs a reload
	readByte(noise2.EnvCtr);	//	uint8		Noise envelope counter
	readByte(noise2.Envelope);	//	uint8		Noise  envelope value
	readWord(noise2.Cycles);	//	uint16		Noise cycles
	readByte(tpc);			//	uint8		Last value written to $402C
	IntWrite(0x4, 0x02C, tpc);
	readByte(tpc);			//	uint8		Last value written to $4030
	IntWrite(0x4, 0x030, tpc);
	readByte(tpc);			//	uint8		Last value written to $4031
	IntWrite(0x4, 0x031, tpc);
	readWord(dpcm->baseAddr);	//	uint16
	readBool(dynamic_cast<CDPCM_OneBus*>(dpcm)->RawPCM);	//	uint8
	return clen;
}
void	APU_OneBus::Run (void) {
	Controllers::PortExp->CPUCycle();
	if (NES::CurRegion ==Settings::REGION_DENDY && Settings::Dendy60Hz && !(InternalClock %6))
		;
	else {
		RunFrame();
		square0.Run();
		square1.Run();
		triangle.Run();
		noise.Run();
		if (Regs[0x030] &0x08) {
			square2.Run();
			square3.Run();
			triangle2.Run();
			noise2.Run();
		}
		dpcm->Run(InternalClock);
	}
	InternalClock++;

	if (Sound::isEnabled && !Controllers::capsLock) {
		double	Pulse, TN, Pulse2, TN2;
		
		bool	RawPCM =!!(Regs[0x30] &0x10);
		if (Settings::NonlinearMixing) {
			Pulse =(square0.Pos |square1.Pos)? 95.88 /( (8128.0 /((square0.Pos + square1.Pos))) +100.0): 0.0;
			Pulse2=(square2.Pos |square3.Pos)? 95.88 /( (8128.0 /((square2.Pos + square3.Pos))) +100.0): 0.0;
			if (RawPCM)
				TN =(triangle.Pos  |noise.Pos)? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0) +100.0): 0.0;
			else
				TN =(triangle.Pos  |noise.Pos  |(dpcm->Pos &0x7F))? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0 +(dpcm->Pos &0x7F)/22638.0) +100.0): 0.0;
			TN2 =(triangle2.Pos  |noise2.Pos)? 159.79 /(1.0/(triangle2.Pos  /8227.0 +noise2.Pos /12241.0) +100.0): 0.0;
		} else {
			Pulse =(square0.Pos +square1.Pos)/30.0/4.0/2.0 *squareSumFactor[square0.Vol +square1.Vol] *0.258483;
			Pulse2=(square2.Pos +square3.Pos)/30.0/4.0/2.0 *squareSumFactor[square2.Vol +square3.Vol] *0.258483;
			if (RawPCM)
				TN =(triangle.Pos  |noise.Pos)? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0) +100.0): 0.0;
			else
				TN =(triangle.Pos  |noise.Pos  |(dpcm->Pos &0x7F))? 159.79 /(1.0/(triangle.Pos  /8227.0 +noise.Pos /12241.0 +(dpcm->Pos &0x7F)/22638.0) +100.0): 0.0;
			TN2 =(triangle2.Pos  |noise2.Pos)? 159.79 /(1.0/(triangle2.Pos  /8227.0 +noise2.Pos /12241.0) +100.0): 0.0;
		}
		if (Settings::LowPassFilterAPU ==1) {
			Pulse =filterPulse.process(Pulse);
			Pulse2=filterPulse2.process(Pulse2);
		}
		#if DISABLE_ALL_FILTERS
		if (RawPCM) TN2 +=dpcm->Pos /240.0;
		#else
		if (RawPCM) {
			TN2 +=(Settings::LowPassFilterOneBus? filterPCM.output(dpcm->Pos +1e-15): dpcm->Pos) /240.0;
		}
		#endif
		Output =Pulse +TN;
		
		if (Regs[0x30] &8 && (Regs[0x35] || Regs[0x30] &0x10)) {
			Output +=Pulse2 +TN2;
		#if SUPPORT_STEREO
			OutputL =Pulse +TN;
			OutputR =Pulse2 +TN2;
		} else {
			OutputL =Pulse +TN +Pulse2 +TN2;
			OutputR =Pulse +TN +Pulse2 +TN2;
		#endif
		}
	}
}
} // namespace APU

namespace PPU {
PPU_OneBus::PPU_OneBus():
	PPU_RP2C02(0),
	EVA(0),
	ReadType(0) {
}

void	PPU_OneBus::Reset (void) {
	for (auto& c: reg2000) c =0;
	PPU_RP2C02::Reset();
}

int	PPU_OneBus::IntRead (int Bank, int Addr) {
	switch(Addr) {
		case 0: return PPU_RP2C02::IntRead(Bank, 2); // ??? Wanted by "Mole Whack"
		case 7:	IOMode = 5;
			if ((VRAMAddr & 0x3F00) == 0x3F00) {
				readLatch &=0xC0;
				if (Reg2001 &0x01)
					return readLatch |= Palette[VRAMAddr] & 0x30;
				else
					return readLatch |= Palette[VRAMAddr];
			} else
				return readLatch = buf2007;
			break;
		case 0x1F:
			// Used on VT369 games as an index into a table containing 240,240,240,320,240,240,240,240. Value 3 causes glitches, all other not.
			return 0;
		default:
			if (Addr <=7)
				return PPU_RP2C02::IntRead(Bank, Addr);
			else
				return readLatch;
	}
}

void	PPU_OneBus::IntWrite (int Bank, int Addr, int Val) {
	if ((Addr &0xF00) ==0x000) reg2000[Addr &0xFF] =Val;
	switch(Addr) {
		case 4:	if (IsRendering)
				SprAddr = ((SprAddr + 1) &~3) | (SprAddr &3);
			else {
				Sprite[SprAddr] = (unsigned char)Val;
				SprAddr = (SprAddr + 1) & 0xFF;
			}
			#ifdef	ENABLE_DEBUGGER
			Debugger::SprChanged = TRUE;
			#endif	/* ENABLE_DEBUGGER */
			break;
		case 7:	if ((VRAMAddr &0x3F00) ==0x3F00) {
				Addr =VRAMAddr &0xFF;
				if ((RI.ConsoleType ==CONSOLE_VT03) && NES::CurRegion ==Settings::REGION_NTSC && CPU::CPU[which]->DMAStateOAM) {
					Addr =(Addr -1) &0xFF;
				}
				Palette[Addr] =(unsigned char)Val;

				#ifdef	ENABLE_DEBUGGER
				Debugger::PalChanged = TRUE;
				#endif	/* ENABLE_DEBUGGER */
				if (!(Addr &0x63))
					Palette[Addr ^0x10] = (unsigned char)Val;
				IncrementAddr();
				if (RI.ConsoleType ==CONSOLE_VT03 && NES::CurRegion ==Settings::REGION_NTSC && (CPU::CPU[which]->EnableDMA &DMA_IN_SPR) && VRAMAddr ==0x4000) {
					VRAMAddr =0x3F00;
				}
			} else {
				IOVal = (unsigned char)Val;
				IOMode = 6;
			}
			break;
		default:
			if (Addr <=7) PPU_RP2C02::IntWrite(Bank, Addr, Val);
			break;
	}
}

int	PPU_OneBus::Save (FILE *out) {
	int clen =PPU_RP2C02::Save(out);
	writeArray(Palette+0x20, 0x100 -0x20);	//	PRAM	uint8[0xE0]	remaining 256 minus 32 bytes of palette index RAM
	writeByte(reg2000[0x10]);
	return clen;
}

int	PPU_OneBus::Load (FILE *in, int version_id) {
	int clen =PPU_RP2C02::Load(in, version_id);
	readArray(Palette+0x20, 0x100 -0x20);	//	PRAM	uint8[0xE0]	remaining 256 minus 32 bytes of palette index RAM
	readByte(reg2000[0x10]);
	return clen;
}

void	PPU_OneBus::RunNoSkip (int NumTicks) {
	register unsigned long TL;
	register unsigned char TC;
	register unsigned char BG;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i, y;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;

		if (Clockticks ==256) {
			if (SLnum <240)	ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 && Clockticks <=303) {
			if (IsRendering && SLnum ==-1) {
				VRAMAddr &=~0x7BE0;
				VRAMAddr |=IntReg &0x7BE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick)
					EndSLTicks =340;
				else
					EndSLTicks =341;
			} else
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			}
			if (SLnum ==SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[0]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				if (which ==NES::WhichScreenToShow) GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline

		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[(Clockticks >> 1) &3] =0;
					if (IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
				} else
				if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData[Clockticks >>1 &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
				else
					RenderData[Clockticks >>1 &3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				RenderAddr =0x2000 | VRAMAddr &0xFFF; // Fetch name table byte
				EVA =0;
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =RenderData[0]<<4 | VRAMAddr >>12 | Reg2000 <<8 &0x1000; // Set up CHR-ROM address for selected tile, don't fetch yet
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr =0x23C0 | VRAMAddr &0xC00 | VRAMAddr >>4 &0x38 | VRAMAddr >>2 &0x07; // Fetch attribute table byte
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
				CurTileData = &TileData[Clockticks +13];
				BG =RenderData[1] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3;
				if (reg2000[0x10] &BKEXTEN) {
					TL =0;
					EVA =BG <<10;
				} else
					TL =BG *0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case 323:	case 331:
				CurTileData = &TileData[Clockticks -323];
				BG =RenderData[1] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3;
				if (reg2000[0x10] &BKEXTEN) {
					TL =0;
					EVA =BG <<10;
				} else
					TL =BG *0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				/* Rendering address:
					FEDC BA98 7654 3210
					-------------------
					10.B TTTT TTTT PRRR
					|| | |||| |||| |+++- Tile row
					|| | |||| |||| +---- Bitplane
					|| | ++++-++++------ Tile number from NT
					|| +---------------- Pattern table left/right
					++------------------ Indicate BG read, bitplanes 0-1
				*/
				RenderAddr =PatAddr |0x8000;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				TC =ReverseCHR[RenderData[2]]; // CHR-ROM data plane 0
				CurTileData =&TileData[Clockticks +11];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[2]]; // CHR-ROM data plane 2
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4]  <<5;
				}
				break;
			case 325:	case 333:
				TC =ReverseCHR[RenderData[2]]; // CHR-ROM data plane 0
				CurTileData =&TileData[Clockticks -325];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[2]]; // CHR-ROM data plane 2
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4]  <<5;
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr =PatAddr |8 |0x8000;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				TC = ReverseCHR[RenderData[3]]; // CHR-ROM data plane 1
				CurTileData =&TileData[Clockticks +9];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[3]]; // CHR-ROM data plane 3
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4]  <<5;
				}
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				TC =ReverseCHR[RenderData[3]]; // CHR-ROM data plane 1
				CurTileData = &TileData[Clockticks -327];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];

				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data plane 3
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4]  <<5;
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				// Fall-through
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
				EVA =0;
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum =Clockticks >>1 &0x1C;
				TC = SprBuff[SprNum |1];
				SprSL = (unsigned char)(SLnum -SprBuff[SprNum]);
 				if (Reg2000 &0x20) // 8x16 sprites
					PatAddr =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1));
				else	
					PatAddr =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9);
				if (reg2000[0x10] &SPEXTEN) EVA =SprBuff[SprNum |2] <<8 &0x1C00;
				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr =PatAddr |0xA000;
				break;
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				SprNum =Clockticks >>1 &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[2];
				else
					TC =ReverseCHR[RenderData[2]];
				TL =(SprBuff[SprNum] &0x3) *0x04040404;
				CurTileData =SprData[SprNum >>2];
				((unsigned long *)CurTileData)[0] =CHRLoBit[TC &0xF] |TL;
				((unsigned long *)CurTileData)[1] =CHRLoBit[TC >>4]  |TL;

				if (reg2000[0x10] &SP16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);

					if (SprBuff[SprNum] &0x40)
						TC = RenderDataHi[2];
					else
						TC = ReverseCHR[RenderDataHi[2]];
					if (reg2000[0x10] &PIX16EN) {
						((unsigned long *)CurTileData)[2] =CHRLoBit[TC &0xF] |TL;
						((unsigned long *)CurTileData)[3] =CHRLoBit[TC >>4]  |TL;
					} else {
						((unsigned long *)CurTileData)[0]|=CHRLoBit[TC &0xF] <<5;
						((unsigned long *)CurTileData)[1]|=CHRLoBit[TC >>4]  <<5;
					}
				}
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr =PatAddr |0xA008;
				break;
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				SprNum =Clockticks >>1 &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[3];
				else	
					TC =ReverseCHR[RenderData[3]];
				CurTileData =SprData[SprNum >>2];
				((unsigned long *)CurTileData)[0] |= CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >>4];
				CurTileData[16] =SprBuff[SprNum];
				CurTileData[17] =SprBuff[SprNum |1];

				if (reg2000[0x10] & SP16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					if (SprBuff[SprNum] & 0x40)
						TC = RenderDataHi[3];
					else	
						TC = ReverseCHR[RenderDataHi[3]];
					if (reg2000[0x10] &PIX16EN) {
						((unsigned long *)CurTileData)[2] |=CHRHiBit[TC &0xF];
						((unsigned long *)CurTileData)[3] |=CHRHiBit[TC >>4];
						if (SprBuff[SprNum] &0x40) {
							unsigned long temp1 =((unsigned long *)CurTileData)[2];
							unsigned long temp2 =((unsigned long *)CurTileData)[3];
							((unsigned long *)CurTileData)[2] =((unsigned long *)CurTileData)[0];
							((unsigned long *)CurTileData)[3] =((unsigned long *)CurTileData)[1];
							((unsigned long *)CurTileData)[0] =temp1;
							((unsigned long *)CurTileData)[1] =temp2;
						}
					} else {
						((unsigned long *)CurTileData)[0] |= CHRHiBit[TC &0xF] <<5;
						((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >>4]  <<5;
					}
				}
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (~Clockticks &1) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2) WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, RenderAddr & 0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr = (unsigned short)(VRAMAddr &0x3FFF);
			if (IOMode >=5 && !IsRendering)
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else
			if (IOMode ==2) {
				if (!IsRendering) WriteHandler[addr >>10](addr >>10, addr &0x3FF, IOVal);
			}
			else if (IOMode == 1) {
				IOMode++;
				if (!IsRendering) {
					if (ReadHandler[addr >> 10] ==BusRead)
						buf2007 =CHRPointer[addr >> 10][addr &0x3FF];
					else	
						buf2007 =(unsigned char)ReadHandler[addr >>10](addr >>10, addr &0x3FF);
				}
			}
			IOMode -=2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks ==255)) IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!((Clockticks & 7) == 7 && !(255 <= Clockticks && Clockticks <= 319))) IncrementH();
				} else
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);

		register int PalIndex, DisplayedTC;
		if (Clockticks <256 && OnScreen) {
			int maxPixel = ~(reg2000[0x10] &SP16EN && reg2000[0x10] &PIX16EN? 15: 7);
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileData[Clockticks + IntX];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;
			
			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (y = 0; y < SprCount; y += 4) {
				register int SprPixel = Clockticks - SprData[y >>2][17];
				register unsigned char SprDat;
				if (SprPixel &maxPixel) continue;
				SprDat = SprData[y >>2][SprPixel];
				if (SprDat &0x63) {
					if (Spr0InLine && y ==0 && TC &0x63 && Clockticks <255) {
						Spr0Hit =1;
						Spr0InLine =FALSE;
					}
					if (showOBJ && !(TC &0x63 && SprData[y >> 2][16] & 0x20)) DisplayedTC = SprDat |0x10;
					break;
				}
			
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		PalIndex &= GrayScale | 0xFFC0;
		if (!(reg2000[0x10] &COLCOMP)) PalIndex |= ColorEmphasis;
		*GfxData++ =PalIndex;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) *GfxData++ =PalIndex;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

void	PPU_OneBus::RunSkip (int NumTicks) {
	register unsigned char TC;
	register unsigned char BG;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i;
	for (i = 0; i < NumTicks; i++)
	{
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;
		if (Clockticks == 256)
		{
			if (SLnum < 240)
			{
				if (Spr0InLine)
					ZeroMemory(TileData, sizeof(TileData));
			}
		}
		else if (Clockticks >= 279 && Clockticks <= 303)
		{
			if ((IsRendering) && (SLnum == -1))
			{
				VRAMAddr &= ~0x7BE0;
				VRAMAddr |= IntReg & 0x7BE0;
			}
		}
		else if (Clockticks == 338)
		{
			if (SLnum == -1)
			{
				if ((ShortSL) && (IsRendering) && (SkipTick))
					EndSLTicks = 340;
				else	EndSLTicks = 341;
			}
			else	EndSLTicks = 341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			} else
			if (SLnum == SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[0]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (Reg2001 &0x18) IsRendering = TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		else if ((SLnum == -1) && (Clockticks == 1))
			Reg2002 = 0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline
		if (IsRendering)
		{
			ProcessSprites();
			if (Clockticks & 1)
			{
				if (IOMode)
				{
					RenderData[(Clockticks >> 1) & 3] = 0;	// seems to force black in this case
					RenderDataHi[(Clockticks >> 1) & 3] = 0;	// seems to force black in this case
					if (IOMode == 2)
						WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, IOVal = RenderData[(Clockticks >> 1) & 3]);
				}
				else if (ReadHandler[RenderAddr >> 10] == BusRead)
					RenderData[(Clockticks >> 1) & 3] = CHRPointer[RenderAddr >> 10][RenderAddr & 0x3FF];
				else {
					int ReadResult =ReadHandler[RenderAddr >> 10](RenderAddr >> 10, (RenderAddr & 0x3FF) | (ReadType <<10) | (EVA <<12));
					RenderData[(Clockticks >> 1) & 3] = ReadResult &0xFF;
					RenderDataHi[(Clockticks >> 1) & 3] = ReadResult >>8;
				}
			}
			switch (Clockticks)
			{
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				RenderAddr =0x2000 | VRAMAddr &0xFFF; // Fetch name table byte
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr = (RenderData[0] << 4) | (VRAMAddr >> 12) | ((Reg2000 & 0x10) << 8);
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr = 0x23C0 | (VRAMAddr & 0xC00) | ((VRAMAddr & 0x380) >> 4) | ((VRAMAddr & 0x1C) >> 2);
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			case 323:	case 331:
				BG = ((RenderData[1] >> (((VRAMAddr & 0x40) >> 4) | (VRAMAddr & 0x2))) & 3);
				EVA =BG;
				if (reg2000[0x10] & BKEXTEN) BG =0;
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				RenderAddr = PatAddr;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				if (Spr0InLine)
				{
					TC = ReverseCHR[RenderData[2]];
					CurTileData = &TileData[Clockticks + 11];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];

					TC = ReverseCHR[RenderDataHi[2]]; // CHR-ROM data
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<5;
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<5;
				}
				break;
			case 325:	case 333:
				if (Spr0InLine)
				{
					TC = ReverseCHR[RenderData[2]];
					CurTileData = &TileData[Clockticks - 325];
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4];

					TC = ReverseCHR[RenderDataHi[2]]; // CHR-ROM data
					((unsigned long *)CurTileData)[0] |= CHRLoBit[TC & 0xF] <<5;
					((unsigned long *)CurTileData)[1] |= CHRLoBit[TC >> 4] <<5;
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr = PatAddr | 8;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				if (Spr0InLine)
				{
					TC = ReverseCHR[RenderData[3]];
					CurTileData = &TileData[Clockticks + 9];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
					TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<5;
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<5;
				}
				IncrementH();
				if (Clockticks == 255)
					IncrementV();
				break;
			case 327:	case 335:
				if (Spr0InLine)
				{
					TC = ReverseCHR[RenderData[3]];
					CurTileData = &TileData[Clockticks - 327];
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4];
					TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data
					((unsigned long *)CurTileData)[0] |= CHRHiBit[TC & 0xF] <<5;
					((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >> 4] <<5;
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &= ~0x41F;
				VRAMAddr |= IntReg & 0x41F;
					case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				EVA =0;
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum = (Clockticks >> 1) & 0x1C;
				TC = SprBuff[SprNum | 1];
				SprSL = (unsigned char)(SLnum - SprBuff[SprNum]);
 				if (Reg2000 & 0x20)
					PatAddr = ((TC & 0xFE) << 4) | ((TC & 0x01) << 12) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x17 : 0x00) ^ ((SprSL & 0x8) << 1));
				else	PatAddr = (TC << 4) | ((SprSL & 7) ^ ((SprBuff[SprNum | 2] & 0x80) ? 0x7 : 0x0)) | ((Reg2000 & 0x08) << 9);
				EVA =(SprBuff[SprNum |2] >>2) &7;
				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr = PatAddr;
				break;
			case 261:
				if (Spr0InLine)
				{
					if (SprBuff[2] & 0x40)
						TC = RenderData[2];
					else	TC = ReverseCHR[RenderData[2]];
					((unsigned long *)SprData[0])[0] = CHRLoBit[TC & 0xF];
					((unsigned long *)SprData[0])[1] = CHRLoBit[TC >> 4];
					if (reg2000[0x10] & SP16EN) {
						if (SprBuff[2] & 0x40)
							TC = RenderDataHi[2];
						else	TC = ReverseCHR[RenderDataHi[2]];
						if (reg2000[0x10] &PIX16EN || (reg2000[0x10] &SPOPEN && EVA&4)) {
							((unsigned long *)SprData[0])[2] = CHRLoBit[TC & 0xF];
							((unsigned long *)SprData[0])[3] = CHRLoBit[TC >> 4];
						} else {
							((unsigned long *)SprData[0])[0] |= CHRLoBit[TC & 0xF] <<5;
							((unsigned long *)SprData[0])[1] |= CHRLoBit[TC >> 4] <<5;
						}
					}
				}
					case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr = PatAddr | 8;
				break;
			case 263:
				if (Spr0InLine)
				{
					if (SprBuff[2] & 0x40)
						TC = RenderData[3];
					else	TC = ReverseCHR[RenderData[3]];
					((unsigned long *)SprData[0])[0] |= CHRHiBit[TC & 0xF];
					((unsigned long *)SprData[0])[1] |= CHRHiBit[TC >> 4];
					SprData[0][16] = SprBuff[2];
					SprData[0][17] = SprBuff[3];
					if (reg2000[0x10] & SP16EN) {
						if (SprBuff[2] & 0x40)
							TC = RenderDataHi[3];
						else	TC = ReverseCHR[RenderDataHi[3]];
						if (reg2000[0x10] &PIX16EN || (reg2000[0x10] &SPOPEN && EVA&4)) {
							((unsigned long *)SprData[0])[2] |= CHRHiBit[TC & 0xF];
							((unsigned long *)SprData[0])[3] |= CHRHiBit[TC >> 4];
							if (SprBuff[2] & 0x40) {
								unsigned long temp1 = ((unsigned long *)SprData[0])[2];
								unsigned long temp2 = ((unsigned long *)SprData[0])[3];
								((unsigned long *)SprData[0])[2] = ((unsigned long *)SprData[0])[0];
								((unsigned long *)SprData[0])[3] = ((unsigned long *)SprData[0])[1];
								((unsigned long *)SprData[0])[0] = temp1;
								((unsigned long *)SprData[0])[1] = temp2;
							}
						} else {
							((unsigned long *)SprData[0])[0] |= CHRHiBit[TC & 0xF] <<5;
							((unsigned long *)SprData[0])[1] |= CHRHiBit[TC >> 4] <<5;
						}
					}
				}
			case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr = 0x2000 | (VRAMAddr & 0xFFF);
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (!(Clockticks & 1))
			{
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode == 2)
					WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, RenderAddr & 0xFF);
			}
		}
		if (IOMode)
		{
			unsigned short addr = (unsigned short)(VRAMAddr & 0x3FFF);
			if ((IOMode >= 5) && (!IsRendering))
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else if (IOMode == 2)
			{
				if (!IsRendering)
					WriteHandler[addr >> 10](addr >> 10, addr & 0x3FF, IOVal);
			}
			else if (IOMode == 1)
			{
				IOMode++;
				if (!IsRendering)
				{
					if (ReadHandler[addr >> 10] == BusRead)
						buf2007 = CHRPointer[addr >> 10][addr & 0x3FF];
					else	buf2007 = (unsigned char)ReadHandler[addr >> 10](addr >> 10, addr & 0x3FF);
				}
			}
			IOMode -= 2;
			if (!IOMode)
			{
				if (IsRendering)
				{
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks == 255))
						IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!(((Clockticks & 7) == 7) && !(255 <= Clockticks && Clockticks <= 319)))
						IncrementH();
				}
				else	IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode)
			PPUCycle(VRAMAddr, SLnum, Clockticks, 0);
		if ((Spr0InLine) && (Clockticks < 255) && (OnScreen) && ((Reg2001 & 0x18) == 0x18) && ((Clockticks >= 8) || ((Reg2001 & 0x06) == 0x06)))
		{
			int maxPixel = ~(((reg2000[0x10] &SP16EN) && (reg2000[0x10] &PIX16EN || (reg2000[0x10] &SPOPEN && EVA&4)))? 15: 7);
			register int SprPixel = Clockticks - SprData[0][17];
			if (!(SprPixel & maxPixel) && (SprData[0][SprPixel] & 0x63) && (TileData[Clockticks + IntX] & 0x63)) {
				//Reg2002 |=0x40;	// Sprite 0 hit
				Spr0Hit =1;
				Spr0InLine = FALSE;
			}
		}
		GfxData++;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) GfxData++;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

int	PPU_VT03::GetPalIndex(int TC) {
	if (!(TC &0x63)) TC =0;
	if (reg2000[0x10] &COLCOMP)
		return (Palette[TC |0x80] <<6) +Palette[TC |0x00] +PALETTE_VT03;
	else
		return Palette[TC];
}
} // namespace PPU