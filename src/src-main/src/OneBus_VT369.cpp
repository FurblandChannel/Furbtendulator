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
#include "Cheats.hpp"
#include "OneBus.h"
#include "OneBus_VT369.h"
#include "ADPCM_VT1682.hpp"
#include "ADPCM_VT369.hpp"

struct VT369DMARequest {
	uint16_t fromAddress;
	uint16_t targetRegister;
	uint16_t targetAddress;
	uint16_t transferLength;
};
std::vector<VT369Sprite>    vt369Sprites;
std::vector<VT369SpriteHi>  vt369SpritesHi;
std::queue<VT369DMARequest> vt369DMARequests;
VT369DMARequest             vt369DMARequest;
uint8_t                     vt369SoundRAM[8192];
int16_t                     vt369DAC;
uint8_t                     vt369Prescaler;
uint8_t                     vt369PrescalerHLE;
int16_t                     vt369TimerPeriod;
int16_t                     vt369LastPeriod;
int16_t                     vt369TimerCount;
uint8_t                     vt369TimerControl;
uint64_t                    vt369adpcmFrame[3];
uint8_t                     vt369adpcmFrameCount[3];

uint8_t                     oamHiRes[512];

void updatePrescaler () {
	if (reg4100[0x1F] &1)
		vt369Prescaler =reg4100[0x1C] &0x80? 1: 3;
	else
	if (reg4100[0x1C] &0x80) // *3 speed of CPU
		vt369Prescaler =2;
	else
		vt369Prescaler =6;
	vt369PrescalerHLE =6;
}

namespace CPU {

void	MAPINT	writeVT369Palette (int bank, int addr, int val) {
	PPU::PPU[0]->Palette[addr &0x3FF & (reg2000[0x1E] ==0? 0x0FF: 0x3FF)] =val;
}

CPU_VT369::CPU_VT369(uint8_t* _RAM):
	CPU_OneBus(0, 4096, _RAM) {
}

int	prescale =0;

void CPU_VT369::RunCycle (void) {
	++CycleCount;

	LastNMI = WantNMI;
	LastIRQ = WantIRQ && !FI;
	if (scheduleDMC && !--scheduleDMC) EnableDMA |=DMA_PCM;
	CPUCycle();

	if (reg4100[0x1F] &0x01) { // half speed of PPU/APU
		if (reg4100[0x1C] &0x80) { // *3 speed of CPU
			if (~CycleCount &1) PPU::PPU[which]->Run();
			if (++prescale ==3) {
				prescale =0;
				if (~CycleCount &1) {
					if (APU::APU[which]) APU::APU[which]->Run();
				}
				if (which ==NES::WhichScreenToShow) Sound::Run();
			}
		} else {
			if (~CycleCount &1) {
				if (PPU::PPU[which]) PPU::PPU[which]->Run();
				if (APU::APU[which]) APU::APU[which]->Run();
			}
			if (which ==NES::WhichScreenToShow) Sound::Run();
		}
	} else
	if (reg4100[0x1C] &0x80) { // *3 speed of CPU
		PPU::PPU[which]->Run();
		if (++prescale ==3) {
			prescale =0;
			if (APU::APU[which]) APU::APU[which]->Run();
			if (which ==NES::WhichScreenToShow) Sound::Run();
		}
	} else { // normal speed
		if (PPU::PPU[which]) PPU::PPU[which]->Run();
		if (APU::APU[which]) APU::APU[which]->Run();
		if (which ==NES::WhichScreenToShow) Sound::Run();
	}
}

#define STATE_DMC_HALT  1
#define STATE_DMC_DUMMY 2
#define STATE_DMC_FETCH 3
#define STATE_OAM_HALT 1
#define STATE_OAM_FETCH 2
#define STATE_OAM_STORE 3
#define BUS_OWNER_NONE 0
#define BUS_OWNER_DMC 1
#define BUS_OWNER_OAM 2

unsigned char	CPU_VT369::MemGet (unsigned int Addr) {
	int buf;

	while (EnableDMA &DMA_PCM || EnableDMA &DMA_SPR || DMAStateDMC || DMAStateOAM) {
		bool isGetCycle =!!(~CycleCount &1);
		bool isLoadDMA  =!!(EnableDMA &DMA_PCM_LOAD);
		
		if (EnableDMA &DMA_PCM && (isLoadDMA && isGetCycle || !isLoadDMA && !isGetCycle)) { /* The beginning of the DMC Halt Cycle is aligned, although it can be delayed to an unaligned cycle */
			EnableDMA &=~DMA_PCM;
			DMAStateDMC =STATE_DMC_HALT;
		}
		if (EnableDMA &DMA_SPR) { /* The beginning of the OAM Halt Cycle is not aligned */
			EnableDMA &=~DMA_SPR;
			OAMCount =0;
			if (DMALength) DMAStateOAM =STATE_OAM_HALT;
		}
		if (!DMAStateDMC && !DMAStateOAM) break;

		bool dmcAlignment =DMAStateDMC ==STATE_DMC_FETCH && !isGetCycle;
		bool oamAlignment =DMAStateOAM ==STATE_OAM_FETCH && !isGetCycle || DMAStateOAM ==STATE_OAM_STORE && isGetCycle;
		
		int busOwner =BUS_OWNER_NONE;
		if (!busOwner && DMAStateDMC ==STATE_DMC_FETCH && !dmcAlignment) busOwner =BUS_OWNER_DMC;
		if (!busOwner && DMAStateOAM >=STATE_OAM_FETCH && !oamAlignment) busOwner =BUS_OWNER_OAM;
		
		#if DMC_DEBUG
		if (DMAStateDMC) EI.DbgOut(L"%08lld: DMC (%s) %s cycle%s", CycleCount, isLoadDMA? L"Load": L"Reload", DMCStates[DMAStateDMC], dmcAlignment? L", Alignment": L"");
		//if (DMAStateOAM) EI.DbgOut(L"%08lld: OAM %s cycle, %s",      CycleCount, OAMStates[DMAStateOAM], oamAlignment? L"Alignment": busOwner ==BUS_OWNER_DMC? L"Fetching": L"Waiting");
		#endif
		
		if (busOwner ==BUS_OWNER_DMC) {
			APU::APU[which]->dpcm->Fetch();
			DMAStateDMC =0;
			EnableDMA &=~DMA_HALT;
			if (DMAStateOAM ==STATE_OAM_HALT) DMAStateOAM++;
		} else
		if (busOwner ==BUS_OWNER_OAM) {
			if (DMAStateOAM ==STATE_OAM_FETCH) {
				OAMByte =MemGetA((DMAPage << 8) | DMAMiddleAddr | OAMCount++);
				DMAStateOAM =STATE_OAM_STORE;
				if (DMAStateDMC ==STATE_DMC_HALT || DMAStateDMC ==STATE_DMC_DUMMY) DMAStateDMC++;
			} else {
				MemSet(DMATarget, OAMByte); // Will also update DMC DMA State
				DMAStateOAM = OAMCount ==DMALength? 0: STATE_OAM_FETCH;
			}
		} else {
			if (NES::CurRegion ==Settings::REGION_NTSC && Settings::DMCJoypadCorruption) MemGetA(Addr); else RunCycle();
			if (DMAStateOAM ==STATE_OAM_HALT) DMAStateOAM++;
			if (DMAStateDMC ==STATE_DMC_HALT || DMAStateDMC ==STATE_DMC_DUMMY) DMAStateDMC++;
			if (DMAStateDMC ==STATE_DMC_DUMMY && EnableDMA &DMA_HALT) {
				EnableDMA &=~DMA_PCM;
				EnableDMA &=~DMA_HALT;
				DMAStateDMC =0;
				#if DMC_DEBUG
				EI.DbgOut(L"%08lld: DMA aborted after Halt", CycleCount);
				#endif
			}
		}
	}
	
	// VT369 high-speed DMA
	while (~EnableDMA &DMA_IN_SPR && !vt369DMARequests.empty()) {
		EnableDMA |= DMA_IN_SPR;
		VT369DMARequest& dr =vt369DMARequests.front();
		if (dr.transferLength) {
			uint8_t val =ReadHandler[dr.fromAddress >>12](dr.fromAddress >>12, dr.fromAddress &0xFFF);
			dr.fromAddress++;
			if (dr.targetRegister ==0x2007) {
				if (dr.targetAddress <0x3C00)
					PPU::PPU[0]->WriteHandler[dr.targetAddress >>10](dr.targetAddress >>10, dr.targetAddress &0x3FF, val);
				else
					PPU::PPU[0]->Palette[dr.targetAddress &0x3FF] =val;
				dr.targetAddress +=PPU::PPU[0]->Reg2000 &0x04? 32: 1;
			} else
				PPU::PPU[0]->IntWrite(0x2, 0x004, val);
			if (!--dr.transferLength) vt369DMARequests.pop();
		}
		EnableDMA &=~DMA_IN_SPR;
	}

	RunCycle();

	// avoid a function call if possible - if it's ReadPRG, then run it inline
	if (ReadHandler[(Addr >> 12) & 0xF] == ReadPRG) {
		if (Readable[(Addr >> 12) & 0xF])
			buf = PRGPointer[(Addr >> 12) & 0xF][Addr & 0xFFF];
		else
			buf = LastRead; //-1;
	}
	else
		buf = ReadHandler[(Addr >> 12) & 0xF]((which <<4) | ((Addr >>12) &0xF), Addr & 0xFFF);
	//if (buf != -1)
		LastRead = (unsigned char)buf;

	return Cheats::readCPUMemory(Addr, LastRead);
	//return LastRead;
}

CPU_VT369_Sound::CPU_VT369_Sound(uint8_t* _RAM):
	CPU_RP2A03(1, 8192, _RAM), // May also be 4096, but certainly not 2048
	dataBank(0),
	prescaler(0),
	WantTimerIRQ(FALSE),
	LastTimerIRQ(FALSE)
{
}

void	CPU_VT369_Sound::IN_ADX (void) {
	unsigned char Val = MemGet(rCalcAddr.Full);
	int result = X + Val +FC;
	FV = !!(~(X ^ Val) & (X ^ result) & 0x80);
	FC = !!(result & 0x100);
	X = result & 0xFF;
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}

__forceinline	void	CPU_VT369_Sound::IN_LDAXD (void) {
	A =RI.PRGROMData[((dataBank <<16 | rCalcAddr.Full) +RI.vt369relative) &(RI.PRGROMSize -1)];
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}

__forceinline	void	CPU_VT369_Sound::IN_LDAD (void) {
	A =RI.PRGROMData[((dataBank <<16 | rCalcAddr.Full) +RI.vt369relative) &(RI.PRGROMSize -1)];
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_VT369_Sound::IN_PHX (void) {
	Push(X);
}
__forceinline	void	CPU_VT369_Sound::IN_PHY (void) {
	Push(Y);
}
__forceinline	void	CPU_VT369_Sound::IN_PLX (void) {
	MemGet(0x100 | SP);
	X =Pull();
}
__forceinline	void	CPU_VT369_Sound::IN_PLY (void) {
	MemGet(0x100 | SP);
	Y =Pull();
}
__forceinline	void	CPU_VT369_Sound::IN_TAD (void) {
	dataBank =A;
}
__forceinline	void	CPU_VT369_Sound::IN_TDA (void) {
	A =dataBank;
}

void CPU_VT369_Sound::ExecOp(void) {
	if (reg4100[0x62] ==0x0D) {
		Opcode =GetOpcode();
		switch (Opcode)	{
		#define OP(op,adr,ins) case 0x##op: AM_##adr(); ins(); break;

		OP(00, IMM, IN_BRK) OP(10, REL, IN_BPL) OP(08, IMP, IN_PHP) OP(18, IMP, IN_CLC) OP(04, ZPG, IV_NOP) OP(14, ZPX, IV_NOP) OP(0C, ABS, IV_NOP) OP(1C, ABX, IV_NOP)
		OP(20, NON, IN_JSR) OP(30, REL, IN_BMI) OP(28, IMP, IN_PLP) OP(38, IMP, IN_SEC) OP(24, ZPG, IN_BIT) OP(34, IMP, IN_PLY) OP(2C, ABS, IN_BIT) OP(3C, IMP, IN_PLX)
		OP(40, IMP, IN_RTI) OP(50, REL, IN_BVC) OP(48, IMP, IN_PHA) OP(58, IMP, IN_CLI) OP(44, ZPG, IV_NOP) OP(54, ZPX, IV_NOP) OP(4C, ABS, IN_JMP) OP(5C, ABX, IV_NOP)
		OP(60, IMP, IN_RTS) OP(70, REL, IN_BVS) OP(68, IMP, IN_PLA) OP(78, IMP, IN_SEI) OP(64, ZPG, IV_NOP) OP(74, ZPX, IV_NOP) OP(6C, ABS,IN_JMPI) OP(7C, ABX, IV_NOP)
		OP(80, IMM, IV_NOP) OP(90, REL, IN_BCC) OP(88, IMP, IN_DEY) OP(98, IMP, IN_TYA) OP(84, ZPG, IN_STY) OP(94, ZPX, IN_STY) OP(8C, ABS, IN_STY) OP(9C,ABXW, IV_NOP)
		OP(A0, IMM, IN_LDY) OP(B0, REL, IN_BCS) OP(A8, IMP, IN_TAY) OP(B8, IMP, IN_CLV) OP(A4, ZPG, IN_LDY) OP(B4, ZPX, IN_LDY) OP(AC, ABS, IN_LDY) OP(BC, ABX, IN_LDY)
		OP(C0, IMM, IN_CPY) OP(D0, REL, IN_BNE) OP(C8, IMP, IN_INY) OP(D8, IMP, IN_CLD) OP(C4, ZPG, IN_CPY) OP(D4, ZPX, IV_NOP) OP(CC, ABS, IN_CPY) OP(DC, ABX, IV_NOP)
		OP(E0, IMM, IN_CPX) OP(F0, REL, IN_BEQ) OP(E8, IMP, IN_INX) OP(F8, IMP, IN_SED) OP(E4, ZPG, IN_CPX) OP(F4, ZPX, IV_NOP) OP(EC, ABS, IN_CPX) OP(FC, ABX, IV_NOP)

		OP(02, NON, IV_HLT) OP(12, NON, IV_HLT) OP(0A, IMP,IN_ASLA) OP(1A, IMP,IV_NOP2) OP(06, ZPG, IN_ASL) OP(16, ZPX, IN_ASL) OP(0E, ABS, IN_ASL) OP(1E,ABXW, IN_ASL)
		OP(22, NON, IV_HLT) OP(32, NON, IV_HLT) OP(2A, IMP,IN_ROLA) OP(3A, IMP,IV_NOP2) OP(26, ZPG, IN_ROL) OP(36, ZPX, IN_ROL) OP(2E, ABS, IN_ROL) OP(3E,ABXW, IN_ROL)
		OP(42, NON, IV_HLT) OP(52, NON, IV_HLT) OP(4A, IMP,IN_LSRA) OP(5A, IMP, IN_TAD) OP(46, ZPG, IN_LSR) OP(56, ZPX, IN_LSR) OP(4E, ABS, IN_LSR) OP(5E,ABXW, IN_LSR)
		OP(62, ABS, IN_ADX) OP(72, NON, IV_HLT) OP(6A, IMP,IN_RORA) OP(7A, IMP, IN_TDA) OP(66, ZPG, IN_ROR) OP(76, ZPX, IN_ROR) OP(6E, ABS, IN_ROR) OP(7E,ABXW, IN_ROR)
		OP(82, IMM, IV_NOP) OP(92, NON, IV_HLT) OP(8A, IMP, IN_TXA) OP(9A, IMP, IN_TXS) OP(86, ZPG, IN_STX) OP(96, ZPY, IN_STX) OP(8E, ABS, IN_STX) OP(9E,ABYW, IV_NOP)
		OP(A2, IMM, IN_LDX) OP(B2, NON, IV_HLT) OP(AA, IMP, IN_TAX) OP(BA, IMP, IN_TSX) OP(A6, ZPG, IN_LDX) OP(B6, ZPY, IN_LDX) OP(AE, ABS, IN_LDX) OP(BE, ABY, IN_LDX)
		OP(C2, IMP, IN_PHX) OP(D2, IMP, IN_PHY) OP(CA, IMP, IN_DEX) OP(DA, IMP,IV_NOP2) OP(C6, ZPG, IN_DEC) OP(D6, ZPX, IN_DEC) OP(CE, ABS, IN_DEC) OP(DE,ABXW, IN_DEC)
		OP(E2, IMM, IV_NOP) OP(F2, NON, IV_HLT) OP(EA, IMP, IN_NOP) OP(FA, IMP,IV_NOP2) OP(E6, ZPG, IN_INC) OP(F6, ZPX, IN_INC) OP(EE, ABS, IN_INC) OP(FE,ABXW, IN_INC)

		OP(01, INX, IN_ORA) OP(11, INY, IN_ORA) OP(09, IMM, IN_ORA) OP(19, ABY, IN_ORA) OP(05, ZPG, IN_ORA) OP(15, ZPX, IN_ORA) OP(0D, ABS, IN_ORA) OP(1D, ABX, IN_ORA)
		OP(21, INX, IN_AND) OP(31, INY, IN_AND) OP(29, IMM, IN_AND) OP(39, ABY, IN_AND) OP(25, ZPG, IN_AND) OP(35, ZPX, IN_AND) OP(2D, ABS, IN_AND) OP(3D, ABX, IN_AND)
		OP(41, INX, IN_EOR) OP(51, INY, IN_EOR) OP(49, IMM, IN_EOR) OP(59, ABY, IN_EOR) OP(45, ZPG, IN_EOR) OP(55, ZPX, IN_EOR) OP(4D, ABS, IN_EOR) OP(5D, ABX, IN_EOR)
		OP(61, INX, IN_ADC) OP(71, INY, IN_ADC) OP(69, IMM, IN_ADC) OP(79, ABY, IN_ADC) OP(65, ZPG, IN_ADC) OP(75, ZPX, IN_ADC) OP(6D, ABS, IN_ADC) OP(7D, ABX, IN_ADC)
		OP(81, INX, IN_STA) OP(91,INYW, IN_STA) OP(89, IMM, IV_NOP) OP(99,ABYW, IN_STA) OP(85, ZPG, IN_STA) OP(95, ZPX, IN_STA) OP(8D, ABS, IN_STA) OP(9D,ABXW, IN_STA)
		OP(A1, INX, IN_LDA) OP(B1, INY, IN_LDA) OP(A9, IMM, IN_LDA) OP(B9, ABY, IN_LDA) OP(A5, ZPG, IN_LDA) OP(B5, ZPX, IN_LDA) OP(AD, ABS, IN_LDA) OP(BD, ABX, IN_LDA)
		OP(C1, INX, IN_CMP) OP(D1, INY, IN_CMP) OP(C9, IMM, IN_CMP) OP(D9, ABY, IN_CMP) OP(C5, ZPG, IN_CMP) OP(D5, ZPX, IN_CMP) OP(CD, ABS, IN_CMP) OP(DD, ABX, IN_CMP)
		OP(E1, INX, IN_SBC) OP(F1, INY, IN_SBC) OP(E9, IMM, IN_SBC) OP(F9, ABY, IN_SBC) OP(E5, ZPG, IN_SBC) OP(F5, ZPX, IN_SBC) OP(ED, ABS, IN_SBC) OP(FD, ABX, IN_SBC)

		OP(03, INX, IV_SLO) OP(13,INYW, IV_SLO) OP(0B, IMM, IV_AAC) OP(1B,ABYW, IV_SLO) OP(07, ZPG, IV_SLO) OP(17, ZPX, IV_SLO) OP(0F, ABS, IV_SLO) OP(1F,ABXW, IV_SLO)
		OP(23, INX, IV_RLA) OP(33,INYW, IV_RLA) OP(2B, IMM, IV_AAC) OP(3B,ABYW, IV_RLA) OP(27, ZPG, IV_RLA) OP(37, ZPX, IV_RLA) OP(2F, ABS, IV_RLA) OP(3F,ABXW, IV_RLA)
		OP(43, INX, IV_SRE) OP(53,INYW, IV_SRE) OP(4B, IMM, IV_ASR) OP(5B,ABYW, IV_SRE) OP(47, ZPG, IV_SRE) OP(57, ZPX, IV_SRE) OP(4F, ABS, IV_SRE) OP(5F,ABXW, IV_SRE)
		OP(63, INX, IV_RRA) OP(73,INYW, IV_RRA) OP(6B, IMM, IV_ARR) OP(7B,ABYW, IV_RRA) OP(67, ZPG, IV_RRA) OP(77, ZPX, IV_RRA) OP(6F, ABS, IV_RRA) OP(7F,ABXW, IV_RRA)
		OP(83, INX, IV_SAX) OP(93, INY, IV_UNK) OP(8B, IMM, IV_XAA) OP(9B, ABY, IV_UNK) OP(87, ZPG, IV_SAX) OP(97, ZPY, IV_SAX) OP(8F, ABS, IV_SAX) OP(9F, ABY, IV_UNK)
		OP(A3, INX, IV_LAX) OP(B3, INY, IV_LAX) OP(AB, IMM, IV_ATX) OP(BB, ABY, IV_UNK) OP(A7, INY,IN_LDAXD) OP(B7, ZPY, IV_LAX) OP(AF, ABS, IV_LAX) OP(BF, ABX, IN_LDAD)
		OP(C3, INX, IV_DCP) OP(D3,INYW, IV_DCP) OP(CB, IMM, IV_AXS) OP(DB,ABYW, IV_DCP) OP(C7, ZPG, IV_DCP) OP(D7, ZPX, IV_DCP) OP(CF, ABS, IV_DCP) OP(DF,ABXW, IV_DCP)
		OP(E3, INX, IV_ISB) OP(F3,INYW, IV_ISB) OP(EB, IMM, IV_SBC) OP(FB,ABYW, IV_ISB) OP(E7, ZPG, IV_ISB) OP(F7, ZPX, IV_ISB) OP(EF, ABS, IV_ISB) OP(FF,ABXW, IV_ISB)
		#undef OP
		}

		if (LastTimerIRQ)
			DoTimerIRQ();
		#ifdef	ENABLE_DEBUGGER
		else
			GotInterrupt = 0;
		#endif	/* ENABLE_DEBUGGER */
	} else {
		WantTimerIRQ =FALSE;
		CycleCount++;
	}
}

void CPU_VT369_Sound::RunCycle (void) {
	if (vt369TimerControl &0x01) {
		if (--vt369TimerCount ==vt369TimerPeriod && vt369TimerControl &0x02) { vt369TimerCount =0; WantTimerIRQ =TRUE; }
	}
	prescaler++;
	if (prescaler >=vt369Prescaler) {
		prescaler -=vt369Prescaler;
		CycleCount++;
	}
	LastTimerIRQ = WantTimerIRQ && !FI;

}

void	CPU_VT369_Sound::Reset (void) {
	MemGet(PC);
	MemGet(PC);
	MemGet(0x100 | SP--);
	MemGet(0x100 | SP--);
	MemGet(0x100 | SP--);
	FI = 1;
	dataBank =0;
	PCL = MemGet(0x1FFC);
	PCH = MemGet(0x1FFD);
	WantTimerIRQ =WantIRQ =WantNMI =0;
	EnableDMA =FALSE;
	vt369TimerPeriod =0;
	vt369TimerControl =0;
	updatePrescaler();
}

unsigned char	CPU_VT369_Sound::MemGet (unsigned int addr) {
	RunCycle();
	if (addr <0x2000)
		return RAM[addr &0x1FFF];
	else
		return ReadHandler[addr >>12](0x10 | addr >>12, addr &0xFFF);
}

int	CPU_VT369_Sound::readReg (int addr) {
	switch(addr) {
		case 0x210: return summerResult >>8 &0xFF;
		case 0x211: return summerResult     &0xFF;
		case 0x402: return multiplierResult >>8  &0xFF;
		case 0x403: return multiplierResult      &0xFF;
		case 0x404: return 0x00; // ALU status
		default:    return 0xFF;
	}
}

void	CPU_VT369_Sound::writeReg (int addr, int val) {
	switch(addr) {
		case 0x100: vt369TimerPeriod =vt369TimerPeriod &0xFF00 | val <<0;
			    break;
		case 0x101: vt369TimerPeriod =vt369TimerPeriod &0x00FF | val <<8;
		            APU::filterPCM.recalc(20, 1789773.0, vt369TimerPeriod ==~0x1BE? 3995: 5369318.2 / ~vt369TimerPeriod);
			    break;
		case 0x102: vt369TimerControl =val; break;
		case 0x103: WantTimerIRQ =FALSE; break;
		case 0x205: summerAddr =summerAddr &0x00FF | val <<8; break;
		case 0x206:{summerAddr =summerAddr &0xFF00 | val <<0;
		            summerAddr &=RAMMask;
			    int16_t summand1 =RAM[summerAddr +0] | RAM[summerAddr +1] <<8;
			    int16_t summand2 =RAM[summerAddr +2] | RAM[summerAddr +3] <<8;
			    int16_t summand3 =RAM[summerAddr +4] | RAM[summerAddr +5] <<8;
			    int16_t summand4 =RAM[summerAddr +6] | RAM[summerAddr +7] <<8;
			    summerResult =(int32_t) summand1 +summand2 +summand3 +summand4;
		            break; }
		/*
		case 0x210: summerResult =summerResult &0xFFFFFF00 | val <<0; break;
		case 0x211: summerResult =summerResult &0xFFFF00FF | val <<8; break;
		case 0x212: summerResult =summerResult &0xFF00FFFF | val <<16;break;
		case 0x213: summerResult =summerResult &0x00FFFFFF | val <<24;break;
		*/
		case 0x400: multiplierAddr =multiplierAddr &0x00FF | val <<8; break;
		case 0x401: multiplierAddr =multiplierAddr &0xFF00 | val <<0;
			    multiplierAddr &=RAMMask;
			    multiplierResult =vt369DecodeADPCM(&RAM[multiplierAddr]);
			    break;
		case 0x800: vt369DAC =vt369DAC &0x00FF | val <<8;
			    break;
		case 0x801: vt369DAC =vt369DAC &0xFF00 | val;
			    break;
	}
}

void	CPU_VT369_Sound::DoTimerIRQ (void) {
	MemGet(PC);
	MemGet(PC);
	Push(PCH);
	Push(PCL);
	JoinFlags();
	Push(P);
	FI = 1;
	PCL = MemGet(0x1FF8);
	PCH = MemGet(0x1FF9);
#ifdef	ENABLE_DEBUGGER
	GotInterrupt =INTERRUPT_IRQ;
#endif	/* ENABLE_DEBUGGER */
}

int	CPU_VT369_Sound::Save (FILE *out) {
	int clen = 0;
	writeByte(PCH);
	writeByte(PCL);
	writeByte(A);
	writeByte(X);
	writeByte(Y);
	writeByte(SP);
	JoinFlags();
	writeByte(P);
	writeByte(LastRead);
	writeByte(WantNMI);
	writeByte(WantIRQ);
	write64(CycleCount);
	writeByte(dataBank);
	writeByte(prescaler);
	writeBool(LastTimerIRQ);
	writeBool(WantTimerIRQ);
	writeWord(summerAddr);
	writeLong(summerResult);
	writeWord(multiplierAddr);
	writeLong(multiplierResult);
	// Sound RAM will be saved by APU_VT369
	return clen;
}

int	CPU_VT369_Sound::Load (FILE *in, int version_id) {
	int clen = 0;
	readByte(PCH);
	readByte(PCL);
	readByte(A);
	readByte(X);
	readByte(Y);
	readByte(SP);
	readByte(P);
	SplitFlags();
	readByte(LastRead);
	readByte(WantNMI);
	readByte(WantIRQ);
	read64(CycleCount);
	readByte(dataBank);
	readByte(prescaler);
	readBool(LastTimerIRQ);
	readBool(WantTimerIRQ);
	readWord(summerAddr);
	readLong(summerResult);
	readWord(multiplierAddr);
	readLong(multiplierResult);
	return clen;
}

int	MAPINT	readVT369Sound (int bank, int addr) {
	return dynamic_cast<CPU::CPU_VT369_Sound*>(CPU::CPU[1])->readReg(addr);
}

int	MAPINT	readVT369SoundRAM (int bank, int addr) {
	return vt369SoundRAM[0x1800 |addr &0x7FF];
}

void	MAPINT	writeVT369Sound (int bank, int addr, int val) {
	dynamic_cast<CPU::CPU_VT369_Sound*>(CPU::CPU[1])->writeReg(addr, val);
}

} // namespace CPU

namespace APU {
void	APU_VT369::IntWrite (int bank, int addr, int val) {
	if ((addr &0xF00) ==0x100) reg4100[addr &0xFF] =val;

	// VT369 has no DPCM channel.
	// We allow it in non-enhanced modes at least.
	// Emulating it in enhanced modes would cause "Crazy Coins" to crash due to unexpected DPCM IRQs.
	if (reg2000[0x10] !=0 || reg2000[0x1E] !=0) {
		if (addr ==0x010) return;
		if (addr ==0x015) val &=~0x10;
	}

	// Write to sound CPU RAM?
	if (addr &0x800)
		vt369SoundRAM[addr &0x7FF |0x1800] =val;
	else
	switch (addr) {
	case 0x014:
		if (reg4100[0x1C] &0x80) { // 5.37 MHz mode: Fast DMA mode
			vt369DMARequest.fromAddress =vt369DMARequest.fromAddress &0x00FF |val <<8;
			vt369DMARequests.push(vt369DMARequest);
			vt369DMARequest.targetAddress +=vt369DMARequest.transferLength;
		} else  // 1.78 MHz mode: NES/VT03-compatible DMA mode
			APU_RP2A03::IntWrite(bank, addr, val);
		break;
	case 0x024:
		if (reg2000[0x1E] !=0x00) { // ??? Some kind of condition needed to keep "Final Man" from corrupting the DMA Middle Address
			CPU::CPU[which]->DMAMiddleAddr =val;
			vt369DMARequest.fromAddress =vt369DMARequest.fromAddress &0xFF00 |val;
		}
		break;
	case 0x034: { // DMA target, length and middle address
		int shift =val >>1 &7;
		if (shift ==0) shift =8;

		// Denote target, length and middle address for NES/VT03-compatible DMA mode
		if (~reg2000[0x1D] &0x01) CPU::CPU[which]->DMAMiddleAddr =val &0xF0; // Middle address for old sprite mode
		CPU::CPU[which]->DMALength =1 <<shift;
		CPU::CPU[which]->DMATarget =val &1? 0x2007: 0x2004;

		// Denote target, length and middle address for Fast DMA mode
		if (~reg2000[0x1D] &0x01) vt369DMARequest.fromAddress =vt369DMARequest.fromAddress &0xFF00 |val &0xF0; // Middle address for old sprite mode
		vt369DMARequest.transferLength =1 <<shift;
		vt369DMARequest.targetRegister =val &1? 0x2007: 0x2004;
		break;
		}
	case 0x11C:
	case 0x11F:
		updatePrescaler();
		break;
	case 0x12D:
		if (val ==0) {
			CPU::CPU[which]->DMAMiddleAddr =val;
			vt369DMARequest.fromAddress =vt369DMARequest.fromAddress &0xFF00;
		}
		break;
	case 0x130:
		aluOperand14 =aluOperand14 &0xFFFFFF00 | val << 0;
		break;
	case 0x131:
		aluOperand14 =aluOperand14 &0xFFFF00FF | val << 8;
		break;
	case 0x132:
		aluOperand14 =aluOperand14 &0xFF00FFFF | val <<16;
		break;
	case 0x133:
		aluOperand14 =aluOperand14 &0x00FFFFFF | val <<24;
		break;
	case 0x134:
		aluOperand56 =aluOperand56 &0xFF00 | val << 0;
		break;
	case 0x135:
		aluOperand56 =aluOperand56 &0x00FF | val << 8;
		aluOperand14 =(aluOperand14 &0xFFFF) *aluOperand56;
		aluBusy =16;
		break;
	case 0x136:
		aluOperand67 =aluOperand67 &0xFF00 | val << 0;
		break;
	case 0x137:
		aluOperand67 =aluOperand67 &0x00FF | val << 8;
		if (aluOperand67 !=0) {			
			aluOperand56 =aluOperand14 %aluOperand67;
			aluOperand14 =aluOperand14 /aluOperand67;
			aluBusy =32;
		}
		break;
	case 0x162:
		if (val ==0x0D && !Settings::VT369SoundHLE) CPU::CPU[1]->Reset();
		break;
	case 0x169:
		if (RI.INES2_SubMapper ==13 || RI.INES2_SubMapper ==15)
			dynamic_cast<CPU::CPU_OneBus*>(CPU::CPU[which])->EncryptionStatus =(val &1)? false: true;
		break;
	case 0x201:
		for (int i =0; i <0x200; i++) oamHiRes[i] =NES::CPU_RAM[(val <<8) +i];
		break;
	default:
		APU_RP2A03::IntWrite(bank, addr, val);
		break;
	}
}

int	APU_VT369::IntRead (int bank, int addr) {
	if (addr &0x800)
		return vt369SoundRAM[addr &0x7FF |0x1800];
	else
	switch (addr) {
	case 0x119: // TV system on VT03
		return 0;
	case 0x130:
	case 0x138:
		return aluOperand14 >> 0 &0xFF;
	case 0x131:
	case 0x139:
		return aluOperand14 >> 8 &0xFF;
	case 0x132:
	case 0x13A:
		return aluOperand14 >>16 &0xFF;
	case 0x133:
	case 0x13B:
		return aluOperand14 >>24 &0xFF;
	case 0x134:
	case 0x13C:
		return aluOperand56 >> 0 &0xFF;
	case 0x135:
	case 0x13D:
		return aluOperand56 >> 8 &0xFF;
	case 0x136:
		return aluBusy;
	default:
		return APU_RP2A03::IntRead(bank, addr);
	}
}

void	APU_VT369::PowerOn (void) {
	APU_RP2A03::PowerOn();
	aluOperand14 =0;
	aluOperand56 =0;
	aluOperand67 =0;
}

void	APU_VT369::Reset (void) {
	APU_RP2A03::Reset();
	vt369TimerPeriod =0;
	vt369TimerControl =0;
	vt369LastPeriod =0;
	updatePrescaler();
	// Default DMA values, to allow NES games to run in VT369 5.37 MHz mode
	vt369DMARequest.transferLength =256;
	vt369DMARequest.targetRegister =0x2004;
}

int	APU_VT369::Save (FILE *out) {
	int clen =APU_RP2A03::Save(out);
	writeLong(aluOperand14);
	writeWord(aluOperand56);
	writeWord(aluOperand67);
	writeByte(aluBusy);
	writeArray(vt369SoundRAM, sizeof(vt369SoundRAM)*sizeof(uint8_t));
	writeWord(vt369DAC);
	writeByte(vt369Prescaler);
	writeByte(vt369PrescalerHLE);
	writeWord(vt369TimerPeriod);
	writeWord(vt369LastPeriod);
	writeWord(vt369TimerCount);
	writeByte(vt369TimerControl);
	write64(vt369adpcmFrame[0]);
	write64(vt369adpcmFrame[1]);
	write64(vt369adpcmFrame[2]);
	writeByte(vt369adpcmFrameCount[0]);
	writeByte(vt369adpcmFrameCount[1]);
	writeByte(vt369adpcmFrameCount[2]);
	writeByte(reg4100[0x1C]);
	writeByte(reg4100[0x1F]);
	writeByte(reg4100[0x62]);
	return clen;
}

int	APU_VT369::Load (FILE *in, int version_id) {
	int clen =APU_RP2A03::Load(in, version_id);
	readLong(aluOperand14);
	readWord(aluOperand56);
	readWord(aluOperand67);
	readByte(aluBusy);
	readArray(vt369SoundRAM, sizeof(vt369SoundRAM)*sizeof(uint8_t));
	readWord(vt369DAC);
	readByte(vt369Prescaler);
	readByte(vt369PrescalerHLE);
	readWord(vt369TimerPeriod);
	readWord(vt369LastPeriod);
	readWord(vt369TimerCount);
	readByte(vt369TimerControl);
	read64(vt369adpcmFrame[0]);
	read64(vt369adpcmFrame[1]);
	read64(vt369adpcmFrame[2]);
	readByte(vt369adpcmFrameCount[0]);
	readByte(vt369adpcmFrameCount[1]);
	readByte(vt369adpcmFrameCount[2]);
	readByte(reg4100[0x1C]);
	readByte(reg4100[0x1F]);
	readByte(reg4100[0x62]);
	return clen;
}

void	APU_VT369::Run (void) {
	if (reg2000[0x1E] &0x10)
		Output =0.0;
	else
		APU_RP2A03::Run();

	if (reg4100[0x62] ==0x0D && Settings::VT369SoundHLE) {
		vt369SoundRAM[0x1FF5] =0x01; // Signal "initialized"

		uint16_t resetVector =vt369SoundRAM[0x1FF8] | vt369SoundRAM[0x1FF9] <<8;
		if (resetVector ==0x0293) { // 3-bit ADPCM streaming
			switch(vt369SoundRAM[0x1FA2]) {
				case 0x01: // Start channel
					vt369SoundRAM[0x1CB0 +vt369SoundRAM[0x1FA3]  ] =0xFF;
					vt369SoundRAM[0x1CB7 +vt369SoundRAM[0x1FA3]*2] =0x00; // read addr
					vt369SoundRAM[0x1980 +vt369SoundRAM[0x1FA3]*2] =0x00; // predictor
					vt369SoundRAM[0x1981 +vt369SoundRAM[0x1FA3]*2] =0x00; // index
					vt369SoundRAM[0x1FA2] =0x00;
					vt369adpcmFrameCount[vt369SoundRAM[0x1FA3]] =0x00;
					break;
				case 0x02: // Clear output buffer (we don't have one), set rate to 1FA1:1FA0
					vt369TimerPeriod =vt369SoundRAM[0x1FA0] | vt369SoundRAM[0x1FA1] <<8;
					vt369TimerPeriod *=3;
					APU::filterPCM.recalc(20, 1789773.0, 3995);
					vt369SoundRAM[0x1FA2] =0x00;
					break;
				case 0x03: // Stop channel
					vt369SoundRAM[0x1CB0 +vt369SoundRAM[0x1FA3]] =0x00;
					vt369SoundRAM[0x1FA2] =0x00;
					break;
			}
			vt369TimerCount -=vt369PrescalerHLE;
			while (vt369TimerPeriod && vt369TimerCount <=vt369TimerPeriod) {
				int32_t adpcmOutput =0;
				vt369TimerCount -=vt369TimerPeriod;
				for (int ch =0; ch <3; ch++) {
					if (vt369SoundRAM[0x1CB0 +ch] ==0) continue;

					if (vt369adpcmFrameCount[ch] ==0) {
						vt369adpcmFrame[ch] =0;
						uint16_t readAddr =0x1800 +ch*0x80 +vt369SoundRAM[0x1CB7 +ch*2];
						for (int i =0; i <8; i++) vt369adpcmFrame[ch] |= (long long) vt369SoundRAM[readAddr +i] <<(i *8);
						if (vt369adpcmFrame[ch] &0x8000000000000000) vt369adpcmFrame[ch] =0;
						vt369adpcmFrameCount[ch] =21;

						vt369SoundRAM[0x1CB7 +ch*2] +=0x08;
						vt369SoundRAM[0x1CB7 +ch*2] &=0x7F;
					}

					vt1682DecodeADPCM(vt369adpcmFrame[ch] &7, (int8_t&) vt369SoundRAM[0x1980 +ch*2], vt369SoundRAM[0x1981 +ch*2]);
					int8_t predictor =(int8_t) vt369SoundRAM[0x1980 +ch*2];
					adpcmOutput +=predictor <<7;

					vt369adpcmFrame[ch] >>=3;
					vt369adpcmFrameCount[ch]--;
				}
				adpcmOutput =adpcmOutput >32767? 32767: adpcmOutput <-32768? -32768: adpcmOutput;
				vt369DAC =adpcmOutput;
			}
		} else {
			uint16_t adrMasks =0;
			uint16_t adrPeriod =0;
			uint16_t adrLoop =0;
			uint8_t  adrLoopInc =1;
			bool     stream =false;
			switch(resetVector) {
				case 0x0203: // VT369 48-nibble-frame direct access, runs in sound RAM
				case 0x02A0:
					adrMasks  =0x184C;
					adrPeriod =0x183B;
					adrLoop   =0x18FC; // +ch
					break;
				case 0x02E0: // Same, but different loop address
					adrMasks  =0x184C;
					adrPeriod =0x183B;
					adrLoop   =0x1873; // +ch*4
					adrLoopInc=4;
					break;
				case 0x0250: // VT369 48-nibble-frame streaming, runs in sound RAM
					adrMasks  =0x184C;
					adrPeriod =0x183B;
					adrLoop   =0x18FC;
					stream    =true;
					break;
				case 0x1C4C:
				case 0x40AE: // VT369 48-nibble-frame direct access, runs in embedded ROM or a copy of it in sound RAM
					adrMasks  =0x18A4;
					adrPeriod =0x186B;
					break;
			}

			vt369TimerPeriod =vt369SoundRAM[adrPeriod +0] | vt369SoundRAM[adrPeriod +4] <<8;
			if (vt369TimerPeriod !=vt369LastPeriod) {
				vt369LastPeriod =vt369TimerPeriod;
				APU::filterPCM.recalc(20, 1789773.0, vt369TimerPeriod ==~0x1BE? 3995: 5369318.2 / ~vt369TimerPeriod);
			}
			vt369TimerCount -=vt369PrescalerHLE;

			while (vt369TimerPeriod && vt369TimerCount <=vt369TimerPeriod) {
				vt369TimerCount -=vt369TimerPeriod;

				vt369SoundRAM[0x18F6]++;     // Timer IRQ counter;
				vt369SoundRAM[adrMasks +1] =vt369SoundRAM[adrMasks +1] &~vt369SoundRAM[adrMasks +2]; // Apply "stop" mask immediately

				int32_t adpcmOutput =0;
				for (int ch =0; ch <4; ch++) {
					if (vt369SoundRAM[adrMasks +0] &(1 <<ch)) {
						vt369SoundRAM[0x1830 +ch*4] =vt369SoundRAM[0x1860 +ch*4];
						vt369SoundRAM[0x1831 +ch*4] =vt369SoundRAM[0x1861 +ch*4];
						vt369SoundRAM[0x1832 +ch*4] =vt369SoundRAM[0x1862 +ch*4];
					}
					if (vt369SoundRAM[adrMasks +1] &(1 <<ch)) {
						uint32_t waveAddr; 
						if (stream)
							waveAddr =vt369SoundRAM[0x1D21 +ch*2];
						else
							waveAddr =vt369SoundRAM[0x1830 +ch*4] |vt369SoundRAM[0x1831 +ch*4] <<8 |vt369SoundRAM[0x1832 +ch*4] <<16;
						if (vt369SoundRAM[0x1805 +ch*8] ==48) {
							if (stream) {
								vt369SoundRAM[0x1800 +ch*8] =vt369SoundRAM[0x1900 +ch*0x100 +(waveAddr++ &0xFF)];
								vt369SoundRAM[0x1801 +ch*8] =vt369SoundRAM[0x1900 +ch*0x100 +(waveAddr++ &0xFF)];
							} else {
								vt369SoundRAM[0x1800 +ch*8] =RI.PRGROMData[(waveAddr++ +RI.vt369relative) &(RI.PRGROMSize -1)];
								vt369SoundRAM[0x1801 +ch*8] =RI.PRGROMData[(waveAddr++ +RI.vt369relative) &(RI.PRGROMSize -1)];
							}
							if (vt369SoundRAM[0x1800 +ch*8] ==0xFF) {
								if (adrLoop && vt369SoundRAM[adrLoop +ch*adrLoopInc]) {
									vt369SoundRAM[0x1830 +ch*4] =vt369SoundRAM[0x1860 +ch*4];
									vt369SoundRAM[0x1831 +ch*4] =vt369SoundRAM[0x1861 +ch*4];
									vt369SoundRAM[0x1832 +ch*4] =vt369SoundRAM[0x1862 +ch*4];
									vt369SoundRAM[0x1803 +ch*8] =0;
									vt369SoundRAM[0x1804 +ch*8] =0;
									vt369SoundRAM[0x1805 +ch*8] =48;
								} else
									vt369SoundRAM[adrMasks +1] &=~(1 <<ch);
								continue;
							}
						} else
						if (~vt369SoundRAM[0x1805 +ch*8] &1) {
							if (stream)
								vt369SoundRAM[0x1801 +ch*8] =vt369SoundRAM[0x1900 +ch*0x100 +waveAddr++];
							else
								vt369SoundRAM[0x1801 +ch*8] =RI.PRGROMData[(waveAddr++ +RI.vt369relative) &(RI.PRGROMSize -1)];
						}
						if (stream)
							vt369SoundRAM[0x1D21 +ch*2] =waveAddr &0xFF;
						else {
							vt369SoundRAM[0x1830 +ch*4] =waveAddr >>0  &0xFF;
							vt369SoundRAM[0x1831 +ch*4] =waveAddr >>8  &0xFF;
							vt369SoundRAM[0x1832 +ch*4] =waveAddr >>16 &0xFF;
						}
						adpcmOutput +=vt369DecodeADPCM(&vt369SoundRAM[0x1800 +ch*8]);
					}
				}
				vt369SoundRAM[adrMasks +1] =vt369SoundRAM[adrMasks +1] |vt369SoundRAM[adrMasks +0]; // Apply "start" mask at end, so that a zero-length sample will not cause hanging ("Rescue the Girl")
				adpcmOutput =adpcmOutput >32767? 32767: adpcmOutput <-32768? -32768: adpcmOutput;
				vt369DAC =adpcmOutput;
			}
		}
	}
	if (Sound::isEnabled && !Controllers::capsLock) {
		#if DISABLE_ALL_FILTERS
		Output +=vt369DAC /32767.0;
		#else
		if (Settings::LowPassFilterOneBus) {
			Output +=filterPCM.output(vt369DAC +1e-15) /32767.0;
		} else
			Output +=vt369DAC /32767.0;
		#endif
	}

	if (aluBusy) aluBusy--;
}
} // namespace APU

namespace PPU {
void	PPU_VT369::Reset (void) {
	PPU_OneBus::Reset();
	GetGFXPtr();
}

void	PPU_VT369::IntWrite (int Bank, int Addr, int Val) {
	switch(Addr) {
		case 0x004:
			if (reg2000[0x1E] ==0)
				PPU_OneBus::IntWrite(Bank, Addr, Val);
			else {
				if (~reg2000[0x1D] &0x01) SprAddrHigh =0;
				Sprite[SprAddr | SprAddrHigh <<8] =(unsigned char)Val;
				SprAddr++;
				SprAddr &=0xFF;
				if (SprAddr ==0) SprAddrHigh ^=1;
			}
			break;
		case 0x006:	// Store the written value as a target for DMA $2007 transfers, so they work *even if rendering is enabled* (!?), which some games require.
			if (HVTog)
				vt369DMARequest.targetAddress =vt369DMARequest.targetAddress &0x00FF | Val <<8 &0x3F00;
			else
				vt369DMARequest.targetAddress =vt369DMARequest.targetAddress &0x3F00 | Val;
			PPU_OneBus::IntWrite(Bank, Addr, Val);
			break;
		case 0x007:
			if (reg2000[0x1E] ==0)
				PPU_OneBus::IntWrite(Bank, Addr, Val);
			else {
				if ((VRAMAddr &0x3C00) ==0x3C00) {
					Addr =VRAMAddr &0x3FF;
					Palette[Addr] =(unsigned char)Val;

					#ifdef	ENABLE_DEBUGGER
					Debugger::PalChanged = TRUE;
					#endif	/* ENABLE_DEBUGGER */
					IncrementAddr();
				} else {
					IOVal = (unsigned char)Val;
					IOMode = 6;
				}
			}
			break;
		case 0x008:
			SprAddrHigh =Val &1;
			break;
		case 0x049: {
			bool newVerticalMode =false;
			bool newSmallLCDMode =false;
			if (reg2000[0x41] ==0xFF && reg2000[0x47] ==0xA8) {
				newVerticalMode =false;
				newSmallLCDMode =true;
			} else
			if (reg2000[0x41] ==0x3F && reg2000[0x47] ==0x54) {
				newVerticalMode =true;
				newSmallLCDMode =true;
			} else
			if (reg2000[0x41] ==0xFF && reg2000[0x47] ==0x22 && RI.PRGROMSize !=16*1024*1024) { // HiQ Classic 30-in-1 sets these values erroneously
				newVerticalMode =true;
				newSmallLCDMode =false;
			}

			if (GFX::smallLCDMode !=newSmallLCDMode || GFX::verticalMode !=newVerticalMode) {
				GFX::smallLCDMode =newSmallLCDMode;
				GFX::verticalMode =newVerticalMode;
				GFX::apertureChanged =true;
				NES::DoStop |=STOPMODE_NOW;
			}
			break;
		}
		default:
			PPU_OneBus::IntWrite(Bank, Addr, Val);
	}
}

int	PPU_VT369::GetPalIndex(int TC) {
	if (reg2000[0x1E] !=0) {
		return (Palette[TC <<1 |0] | Palette[TC <<1 |1] <<8) +PALETTE_VT369;
	}
	if (!(TC &0x63)) TC =0;
	if (reg2000[0x10] &COLCOMP)
		return (Palette[TC |0x80] <<8) +Palette[TC |0x00] +PALETTE_VT369;
	else
		return Palette[TC];
}

void	PPU_VT369::ProcessSpritesEnhanced (void) {
	vt369Sprites.clear();
	Spr0InLine =0;
	for (int spriteNum =reg2000[0x1D] &0x01? 127: 63; spriteNum !=-1; spriteNum--) {
		VT369Sprite vt369Sprite;
		int  spriteX    =0;
		int  spriteY    =0;
		int  spriteTile =0;
		int  palette    =0;
		bool negX       =false;
		bool negY       =false;
		bool flipX      =false;
		bool flipY      =false;
		bool priority   =false;
		bool rightTable =false; // Only relevant for 8x16 sprites and 201E &0x01==0x00

		if (~reg2000[0x1E] &0x04) { // NES-compatible OAM memory arrangement
			uint8_t* oam =&Sprite[spriteNum <<2];
			spriteY    = oam[0];
			spriteX    = oam[3];
			spriteTile = oam[1] | oam[2] <<6 &0x700;
			if (reg2000[0x1D] &0x08) {
				negX    =!!(oam[2] &0x40);
				negY    =!!(oam[2] &0x80);
				palette =   oam[2] <<4 &0x30 | oam[2] <<1 &0x40;
			} else {
				flipX   =!!(oam[2] &0x40);
				flipY   =!!(oam[2] &0x80);
				palette =   oam[2] <<4 &0x30;
				priority=!!(oam[2] &0x20);
			}
		} else { // New OAM memory arrangement
			spriteY    =   Sprite[0x000 |spriteNum];
			spriteX    =   Sprite[0x180 |spriteNum];
			spriteTile =   Sprite[0x080 |spriteNum] | Sprite[0x100 |spriteNum] <<6 &0x700;
			palette    =   Sprite[0x100 |spriteNum] <<4 &0x30 | Sprite[0x100 |spriteNum] <<1 &0x40;
			if (reg2000[0x1D] &0x08) {
				negX    =!!(Sprite[0x100 |spriteNum] &0x40);
				negY    =!!(Sprite[0x100 |spriteNum] &0x80);
				palette =   Sprite[0x100 |spriteNum] <<4 &0x30 | Sprite[0x100 |spriteNum] <<1 &0x40;
			} else {
				flipX   =!!(Sprite[0x100 |spriteNum] &0x40);
				flipY   =!!(Sprite[0x100 |spriteNum] &0x80);
				palette =   Sprite[0x100 |spriteNum] <<4 &0x30;
				priority=!!(Sprite[0x100 |spriteNum] &0x20);
			}
		}
		if (negX) spriteX -=256;
		if (negY) spriteY -=256;
		if (reg2000[0x1C] &0x80) spriteTile &=0xFF;
		if (reg2000[0x1E] ==0xF && reg2000[0x1D] ==0xB && reg2000[0x10] &0x10) spriteTile =spriteTile &0x3F | spriteTile >>2 &~0x3F; // ??? Needed for "Cross River"

		if (~reg2000[0x1E] &0x01) { // Determine correct register for 2012-2017 operation
			if (Reg2000 &0x20)
				rightTable =!!(spriteTile &0x01);
			else
				rightTable =!!(Reg2000 &0x08);
		}
		if (Reg2000 &0x20) spriteTile &=~0x01;

		// Y in view? Fetch its data bytes and add to vt369 sprites vector
		int spriteSL =SLnum -spriteY;
		int spriteHeight =(reg2000[0x1D] &0x04 || Reg2000 &0x20)? 15: 7;
		if (spriteSL >=0 && spriteSL <=spriteHeight) {
			if (flipY) spriteSL =spriteHeight -spriteSL;
			uint32_t patternAddr =spriteTile <<6;            // 8*8 sprites, 4 bits per pixel or 8*8 sprites, 8 bits per pixel => 64 bytes per tile
			if (reg2000[0x1D] &0x04) patternAddr <<=1;       // 16 pixel high sprites, but one sprite per tile => 128 bytes per tile
			patternAddr +=spriteSL <<3;                      // 8 bytes per scanline, either because 8 pixels*8 bpp, or 16 pixels *4 bpp

			if (~reg2000[0x1D] &0x02 && ~reg2000[0x1D] &0x04 && ~reg2000[0x1C] &0x20) patternAddr >>=1; // 4 bytes per scanline because 8 pixels *4bpp

			if (reg2000[0x1E] &0x01)
				patternAddr +=RI.vt369sprData;
			else
				patternAddr +=RI.vt369relative +(reg2000[rightTable? 0x12: 0x16] <<(reg2000[0x1C] &0x80? 10: 13) <<(reg2000[0x1C] >>4 &3));

			patternAddr &=RI.PRGROMSize -1;
			if (reg2000[0x1D] &0x02) {
				if (flipX) {
					vt369Sprite.data[15] =palette |RI.PRGROMData[patternAddr+0] &0x0F; vt369Sprite.data[14] =palette |RI.PRGROMData[patternAddr+0] >>4;
					vt369Sprite.data[13] =palette |RI.PRGROMData[patternAddr+1] &0x0F; vt369Sprite.data[12] =palette |RI.PRGROMData[patternAddr+1] >>4;
					vt369Sprite.data[11] =palette |RI.PRGROMData[patternAddr+2] &0x0F; vt369Sprite.data[10] =palette |RI.PRGROMData[patternAddr+2] >>4;
					vt369Sprite.data[ 9] =palette |RI.PRGROMData[patternAddr+3] &0x0F; vt369Sprite.data[ 8] =palette |RI.PRGROMData[patternAddr+3] >>4;
					vt369Sprite.data[ 7] =palette |RI.PRGROMData[patternAddr+4] &0x0F; vt369Sprite.data[ 6] =palette |RI.PRGROMData[patternAddr+4] >>4;
					vt369Sprite.data[ 5] =palette |RI.PRGROMData[patternAddr+5] &0x0F; vt369Sprite.data[ 4] =palette |RI.PRGROMData[patternAddr+5] >>4;
					vt369Sprite.data[ 3] =palette |RI.PRGROMData[patternAddr+6] &0x0F; vt369Sprite.data[ 2] =palette |RI.PRGROMData[patternAddr+6] >>4;
					vt369Sprite.data[ 1] =palette |RI.PRGROMData[patternAddr+7] &0x0F; vt369Sprite.data[ 0] =palette |RI.PRGROMData[patternAddr+7] >>4;
				} else {
					vt369Sprite.data[ 0] =palette |RI.PRGROMData[patternAddr+0] &0x0F; vt369Sprite.data[ 1] =palette |RI.PRGROMData[patternAddr+0] >>4;
					vt369Sprite.data[ 2] =palette |RI.PRGROMData[patternAddr+1] &0x0F; vt369Sprite.data[ 3] =palette |RI.PRGROMData[patternAddr+1] >>4;
					vt369Sprite.data[ 4] =palette |RI.PRGROMData[patternAddr+2] &0x0F; vt369Sprite.data[ 5] =palette |RI.PRGROMData[patternAddr+2] >>4;
					vt369Sprite.data[ 6] =palette |RI.PRGROMData[patternAddr+3] &0x0F; vt369Sprite.data[ 7] =palette |RI.PRGROMData[patternAddr+3] >>4;
					vt369Sprite.data[ 8] =palette |RI.PRGROMData[patternAddr+4] &0x0F; vt369Sprite.data[ 9] =palette |RI.PRGROMData[patternAddr+4] >>4;
					vt369Sprite.data[10] =palette |RI.PRGROMData[patternAddr+5] &0x0F; vt369Sprite.data[11] =palette |RI.PRGROMData[patternAddr+5] >>4;
					vt369Sprite.data[12] =palette |RI.PRGROMData[patternAddr+6] &0x0F; vt369Sprite.data[13] =palette |RI.PRGROMData[patternAddr+6] >>4;
					vt369Sprite.data[14] =palette |RI.PRGROMData[patternAddr+7] &0x0F; vt369Sprite.data[15] =palette |RI.PRGROMData[patternAddr+7] >>4;
				}
				for (int i =0; i <16; i++) if (vt369Sprite.data[i] ==palette) vt369Sprite.data[i] =0;
			} else {
				if (~reg2000[0x1D] &0x02 && ~reg2000[0x1D] &0x04 && ~reg2000[0x1C] &0x20) {
					if (flipX) {
						vt369Sprite.data[ 7] =palette |RI.PRGROMData[patternAddr+0] &0x0F; vt369Sprite.data[ 6] =palette |RI.PRGROMData[patternAddr+0] >>4;
						vt369Sprite.data[ 5] =palette |RI.PRGROMData[patternAddr+1] &0x0F; vt369Sprite.data[ 4] =palette |RI.PRGROMData[patternAddr+1] >>4;
						vt369Sprite.data[ 3] =palette |RI.PRGROMData[patternAddr+2] &0x0F; vt369Sprite.data[ 2] =palette |RI.PRGROMData[patternAddr+2] >>4;
						vt369Sprite.data[ 1] =palette |RI.PRGROMData[patternAddr+3] &0x0F; vt369Sprite.data[ 0] =palette |RI.PRGROMData[patternAddr+3] >>4;
					} else {
						vt369Sprite.data[ 0] =palette |RI.PRGROMData[patternAddr+0] &0x0F; vt369Sprite.data[ 1] =palette |RI.PRGROMData[patternAddr+0] >>4;
						vt369Sprite.data[ 2] =palette |RI.PRGROMData[patternAddr+1] &0x0F; vt369Sprite.data[ 3] =palette |RI.PRGROMData[patternAddr+1] >>4;
						vt369Sprite.data[ 4] =palette |RI.PRGROMData[patternAddr+2] &0x0F; vt369Sprite.data[ 5] =palette |RI.PRGROMData[patternAddr+2] >>4;
						vt369Sprite.data[ 6] =palette |RI.PRGROMData[patternAddr+3] &0x0F; vt369Sprite.data[ 7] =palette |RI.PRGROMData[patternAddr+3] >>4;
					}
					for (int i =0; i <8; i++) if (vt369Sprite.data[i] ==palette) vt369Sprite.data[i] =0;
				} else {
					if (flipX) {
						vt369Sprite.data[ 7] =RI.PRGROMData[patternAddr+ 0];
						vt369Sprite.data[ 6] =RI.PRGROMData[patternAddr+ 1];
						vt369Sprite.data[ 5] =RI.PRGROMData[patternAddr+ 2];
						vt369Sprite.data[ 4] =RI.PRGROMData[patternAddr+ 3];
						vt369Sprite.data[ 3] =RI.PRGROMData[patternAddr+ 4];
						vt369Sprite.data[ 2] =RI.PRGROMData[patternAddr+ 5];
						vt369Sprite.data[ 1] =RI.PRGROMData[patternAddr+ 6];
						vt369Sprite.data[ 0] =RI.PRGROMData[patternAddr+ 7];
					} else {
						vt369Sprite.data[ 0] =RI.PRGROMData[patternAddr+ 0];
						vt369Sprite.data[ 1] =RI.PRGROMData[patternAddr+ 1];
						vt369Sprite.data[ 2] =RI.PRGROMData[patternAddr+ 2];
						vt369Sprite.data[ 3] =RI.PRGROMData[patternAddr+ 3];
						vt369Sprite.data[ 4] =RI.PRGROMData[patternAddr+ 4];
						vt369Sprite.data[ 5] =RI.PRGROMData[patternAddr+ 5];
						vt369Sprite.data[ 6] =RI.PRGROMData[patternAddr+ 6];
						vt369Sprite.data[ 7] =RI.PRGROMData[patternAddr+ 7];
					}
				}
			}
			vt369Sprite.priority =priority;
			vt369Sprite.startX   =spriteX;
			vt369Sprite.sprite0  =!!(spriteNum ==0);
			vt369Sprites.push_back(vt369Sprite);

			if (spriteNum ==0) Spr0InLine =1;
		}
	}
}

void	PPU_VT369::Run (void) {
	if (GFX::FPSCnt < (Controllers::capsLock? 9: Settings::FSkip)) {
		if (reg2000[0x1E] ==0)
			RunSkip(reg4100[0x1C] &0x80? 1: 3);
		else
			RunSkipEnhanced(reg4100[0x1C] &0x80? 1: 3);
	} else {
		if (reg2000[0x1E] ==0)
			RunNoSkip(reg4100[0x1C] &0x80? 1: 3);
		else
		if (reg2000[0x1C] &0x04)
			RunNoSkipHiRes(reg4100[0x1C] &0x80? 1: 3);
		else
			RunNoSkipEnhanced(reg4100[0x1C] &0x80? 1: 3);
	}
}

void	PPU_VT369::RunNoSkipEnhanced (int NumTicks) {
	register unsigned char TC;
	register unsigned char *CurTileData;
	register int i;
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
			if (Clockticks &1 && IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				RenderAddr =0x2000 | VRAMAddr &0xFFF; // Fetch name table byte
				RenderData[0] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =RenderData[0]<<4 | VRAMAddr >>12 | Reg2000 <<8 &0x1000; // Set up CHR-ROM address for selected tile, don't fetch yet
				break;
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
				if (reg2000[0x1C] &0x08)
					PatAddr =(                                                                                             RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				else
					PatAddr =((CHRPointer[RenderAddr >>10][RenderAddr &0x3FF] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3) <<12 | RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				break;
			case 323:	case 331:
				if (reg2000[0x1C] &0x08)
					PatAddr =(                                                                                             RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				else
					PatAddr =((CHRPointer[RenderAddr >>10][RenderAddr &0x3FF] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3) <<12 | RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				CurTileData =&TileData[Clockticks +11];
				if (reg2000[0x1E] &0x01)
					PatAddr +=RI.vt369bgData;
				else
					PatAddr +=RI.vt369relative +(reg2000[Reg2000 &0x10? 0x12: 0x16] <<(reg2000[0x1C] &0x08? 10: 13) <<(reg2000[0x1C] &3));
				PatAddr &=RI.PRGROMSize -1;
				if ((reg2000[0x1C] &3) ==2) { // 8 bpp
					PatAddr >>=2;
					((unsigned long *)CurTileData)[0] =((unsigned long *) RI.PRGROMData)[PatAddr +0];
					((unsigned long *)CurTileData)[1] =((unsigned long *) RI.PRGROMData)[PatAddr +1];
				} else { // 4 bpp
					CurTileData[0] =RI.PRGROMData[PatAddr +0] &0x0F;
					CurTileData[1] =RI.PRGROMData[PatAddr +0] >>4;
					CurTileData[2] =RI.PRGROMData[PatAddr +1] &0x0F;
					CurTileData[3] =RI.PRGROMData[PatAddr +1] >>4;
					CurTileData[4] =RI.PRGROMData[PatAddr +2] &0x0F;
					CurTileData[5] =RI.PRGROMData[PatAddr +2] >>4;
					CurTileData[6] =RI.PRGROMData[PatAddr +3] &0x0F;
					CurTileData[7] =RI.PRGROMData[PatAddr +3] >>4;
				}
				PatAddr =0; // Prevent out-of-bounds accesses in case of switching to non-enhanced mode
				break;
			case 325:	case 333:
				CurTileData =&TileData[Clockticks -325];
				if (reg2000[0x1E] &0x01)
					PatAddr +=RI.vt369bgData;
				else
					PatAddr +=RI.vt369relative +(reg2000[Reg2000 &0x10? 0x12: 0x16] <<(reg2000[0x1C] &0x08? 10: 13) <<(reg2000[0x1C] &3));
				PatAddr &=RI.PRGROMSize -1;
				if ((reg2000[0x1C] &3) ==2) { // 8 bpp
					PatAddr >>=2;
					((unsigned long *)CurTileData)[0] =((unsigned long *) RI.PRGROMData)[PatAddr +0];
					((unsigned long *)CurTileData)[1] =((unsigned long *) RI.PRGROMData)[PatAddr +1];
				} else { // 4 bpp
					CurTileData[0] =RI.PRGROMData[PatAddr +0] &0x0F;
					CurTileData[1] =RI.PRGROMData[PatAddr +0] >>4;
					CurTileData[2] =RI.PRGROMData[PatAddr +1] &0x0F;
					CurTileData[3] =RI.PRGROMData[PatAddr +1] >>4;
					CurTileData[4] =RI.PRGROMData[PatAddr +2] &0x0F;
					CurTileData[5] =RI.PRGROMData[PatAddr +2] >>4;
					CurTileData[6] =RI.PRGROMData[PatAddr +3] &0x0F;
					CurTileData[7] =RI.PRGROMData[PatAddr +3] >>4;
				}
				PatAddr =0; // Prevent out-of-bounds accesses in case of switching to non-enhanced mode
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				ProcessSpritesEnhanced();
				break;
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
			case 337:	case 339:
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
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileData[Clockticks + IntX];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;

			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369Sprites) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &~(reg2000[0x1D] &0x2? 15: 7)) && sprite.data[spritePixel]) {
					if (sprite.sprite0) Spr0Hit =1;
					if (showOBJ && !(TC && sprite.priority)) DisplayedTC =sprite.data[spritePixel] |0x100;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		*GfxData++ =PalIndex;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) *GfxData++ =PalIndex;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

void	PPU_VT369::RunSkipEnhanced (int NumTicks) {
	register int i;
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
			if (Clockticks &1 && IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				break;
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
				break;
			case 323:	case 331:
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				break;
			case 325:	case 333:
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				ProcessSpritesEnhanced();
				break;
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
			case 337:	case 339:
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

		if (Spr0InLine && Clockticks <256 && OnScreen) {
			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369Sprites) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &~(reg2000[0x1D] &0x2? 15: 7)) && sprite.data[spritePixel]) {
					if (sprite.sprite0) Spr0Hit =1;
				}
			}
		}
		GfxData++;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) GfxData++;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

void	PPU_VT369::RunNoSkipHiRes (int NumTicks) {
	register unsigned char TC;
	register unsigned char *CurTileDataEven;
	register unsigned char *CurTileDataOdd;
	register int i;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;

		if (Clockticks ==256) {
			if (SLnum <240)	ZeroMemory(TileDataEven, sizeof(TileDataEven));
			if (SLnum <240)	ZeroMemory(TileDataOdd,  sizeof(TileDataOdd));
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
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();

		if (IsRendering) {
			if (Clockticks &1 && IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				RenderAddr =0x2000 | VRAMAddr &0xFFF; // Fetch name table byte
				RenderData[0] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =RenderData[0]<<4 | VRAMAddr >>12 | Reg2000 <<8 &0x1000; // Set up CHR-ROM address for selected tile, don't fetch yet
				break;
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
				if (reg2000[0x1C] &0x08)
					PatAddr =(                                                                                             RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				else
					PatAddr =((CHRPointer[RenderAddr >>10][RenderAddr &0x3FF] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3) <<12 | RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				break;
			case 323:	case 331:
				if (reg2000[0x1C] &0x08)
					PatAddr =(                                                                                             RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				else
					PatAddr =((CHRPointer[RenderAddr >>10][RenderAddr &0x3FF] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3) <<12 | RenderData[0]<<4 | VRAMAddr >>11 &0x0E) <<(reg2000[0x1C] &0x3);
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				CurTileDataEven =&TileDataEven[(Clockticks +11)*2];
				CurTileDataOdd  =&TileDataOdd [(Clockticks +11)*2];
				PatAddr <<=2;
				if (reg2000[0x1E] &0x01)
					PatAddr +=RI.vt369bgData;
				else
					PatAddr +=RI.vt369relative +(reg2000[Reg2000 &0x10? 0x12: 0x16] <<(reg2000[0x1C] &0x08? 10: 13) <<(reg2000[0x1C] &3));
				PatAddr &=RI.PRGROMSize -1;
				if ((reg2000[0x1C] &3) ==2) { // 8 bpp
					PatAddr >>=2;
					((unsigned long *)CurTileDataEven)[0] =((unsigned long *) RI.PRGROMData)[PatAddr +0];
					((unsigned long *)CurTileDataEven)[1] =((unsigned long *) RI.PRGROMData)[PatAddr +1];
					((unsigned long *)CurTileDataEven)[2] =((unsigned long *) RI.PRGROMData)[PatAddr +2];
					((unsigned long *)CurTileDataEven)[3] =((unsigned long *) RI.PRGROMData)[PatAddr +3];
					((unsigned long *)CurTileDataOdd) [0] =((unsigned long *) RI.PRGROMData)[PatAddr +4];
					((unsigned long *)CurTileDataOdd) [1] =((unsigned long *) RI.PRGROMData)[PatAddr +5];
					((unsigned long *)CurTileDataOdd) [2] =((unsigned long *) RI.PRGROMData)[PatAddr +6];
					((unsigned long *)CurTileDataOdd) [3] =((unsigned long *) RI.PRGROMData)[PatAddr +7];
				} else { // 4 bpp, not implemented yet
				}
				PatAddr =0; // Prevent out-of-bounds accesses in case of switching to non-enhanced mode
				break;
			case 325:	case 333:
				CurTileDataEven =&TileDataEven[(Clockticks -325)*2];
				CurTileDataOdd  =&TileDataOdd [(Clockticks -325)*2];
				PatAddr <<=2;
				if (reg2000[0x1E] &0x01)
					PatAddr +=RI.vt369bgData;
				else
					PatAddr +=RI.vt369relative +(reg2000[Reg2000 &0x10? 0x12: 0x16] <<(reg2000[0x1C] &0x08? 10: 13) <<(reg2000[0x1C] &3));
				PatAddr &=RI.PRGROMSize -1;
				if ((reg2000[0x1C] &3) ==2) { // 8 bpp
					PatAddr >>=2;
					((unsigned long *)CurTileDataEven)[0] =((unsigned long *) RI.PRGROMData)[PatAddr +0];
					((unsigned long *)CurTileDataEven)[1] =((unsigned long *) RI.PRGROMData)[PatAddr +1];
					((unsigned long *)CurTileDataEven)[2] =((unsigned long *) RI.PRGROMData)[PatAddr +2];
					((unsigned long *)CurTileDataEven)[3] =((unsigned long *) RI.PRGROMData)[PatAddr +3];
					((unsigned long *)CurTileDataOdd) [0] =((unsigned long *) RI.PRGROMData)[PatAddr +4];
					((unsigned long *)CurTileDataOdd) [1] =((unsigned long *) RI.PRGROMData)[PatAddr +5];
					((unsigned long *)CurTileDataOdd) [2] =((unsigned long *) RI.PRGROMData)[PatAddr +6];
					((unsigned long *)CurTileDataOdd) [3] =((unsigned long *) RI.PRGROMData)[PatAddr +7];
				} else { // 4 bpp, not implemented yet
				}
				PatAddr =0; // Prevent out-of-bounds accesses in case of switching to non-enhanced mode
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				ProcessSpritesHiRes();
				break;
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
			case 337:	case 339:
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
		int PalIndex =0x0F, DisplayedTC;
		
		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileDataEven[(Clockticks +IntX)*2 +0];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;

			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369SpritesHi) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &sprite.maskX) && sprite.dataEven[spritePixel *2 +0]) {
					if (sprite.sprite0) Spr0Hit =1;
					if (showOBJ && !(TC && sprite.priority)) DisplayedTC =sprite.dataEven[spritePixel *2 +0] |0x100;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		*GfxDataEven++ =PalIndex;

		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileDataEven[(Clockticks +IntX)*2 +1];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;

			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369SpritesHi) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &sprite.maskX) && sprite.dataEven[spritePixel *2 +1]) {
					if (sprite.sprite0) Spr0Hit =1;
					if (showOBJ && !(TC && sprite.priority)) DisplayedTC =sprite.dataEven[spritePixel *2 +1] |0x100;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		*GfxDataEven++ =PalIndex;
		
		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileDataOdd[(Clockticks +IntX)*2 +0];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;

			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369SpritesHi) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &sprite.maskX) && sprite.dataOdd[spritePixel *2 +0]) {
					if (sprite.sprite0) Spr0Hit =1;
					if (showOBJ && !(TC && sprite.priority)) DisplayedTC =sprite.dataOdd[spritePixel *2 +0] |0x100;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		*GfxDataOdd++ =PalIndex;
						
		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileDataOdd[(Clockticks +IntX)*2 +1];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;

			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (auto& sprite: vt369SpritesHi) {
				int spritePixel =Clockticks -sprite.startX;
				if (!(spritePixel &sprite.maskX) && sprite.dataOdd[spritePixel *2 +1]) {
					if (sprite.sprite0) Spr0Hit =1;
					if (showOBJ && !(TC && sprite.priority)) DisplayedTC =sprite.dataOdd[spritePixel *2 +1] |0x100;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		*GfxDataOdd++ =PalIndex;

		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) {
			GfxDataEven +=2;
			GfxDataOdd +=2;
		}
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

int	PPU_VT369::Save (FILE *out) {
	int clen =PPU_OneBus::Save(out);
	writeArray(Palette+0x100, 0x400 -0x100);
	writeArray(Sprite+0x100, 0x100);
	writeByte(SprAddrHigh);
	writeByte(reg2000[0x1C]);
	writeByte(reg2000[0x1D]);
	writeByte(reg2000[0x1E]);
	writeByte(reg4100[0x06]);
	return clen;
}

int	PPU_VT369::Load (FILE *in, int version_id) {
	int clen =PPU_OneBus::Load(in, version_id);
	readArray(Palette+0x100, 0x400 -0x100);
	readArray(Sprite+0x100, 0x100);
	readByte(SprAddrHigh);
	readByte(reg2000[0x1C]);
	readByte(reg2000[0x1D]);
	readByte(reg2000[0x1E]);
	readByte(reg4100[0x06]);
	return clen;
}

void	PPU_VT369::GetGFXPtr (void) {
	GfxData =DrawArray;
	GfxDataEven =DrawArrayEven;
	GfxDataOdd  =DrawArrayOdd;
}

void	PPU_VT369::ProcessSpritesHiRes (void) {
	vt369SpritesHi.clear();
	Spr0InLine =0;
	for (int spriteNum =reg2000[0x1D] &0x01? 127: 63; spriteNum !=-1; spriteNum--) {
		VT369SpriteHi vt369Sprite;

		uint8_t* oam =&oamHiRes[spriteNum <<3];
		int spriteY    = oam[0]-1;        // low-res scale
		int spriteX    = oam[7];          // low-res scale
		int spriteSL   = SLnum -spriteY;  // low-res scale
		int spriteTile = oam[1] | oam[2] <<8;
		int palette    = oam[6] <<4 &0xFF;
		
		int sizeX      = 4 <<(oam[3] >>4 &3); // low-res scale
		int sizeY      = 4 <<(oam[3] >>2 &3); // low-res scale
		bool bit8pp    = !!(oam[3] &0x80);
		bool flipX     = !!(oam[6] &0x40);
		bool flipY     = !!(oam[6] &0x80);

		if (flipY) spriteSL =(sizeY -1) -spriteSL;

		if (spriteSL >=0 && spriteSL <=(sizeY -1)) {
			if (bit8pp) {
				uint32_t patternAddr =spriteTile*sizeX*sizeY*2*2 +spriteSL *sizeX*2*2;
				patternAddr +=RI.vt369sprData;
				patternAddr &=RI.PRGROMSize -1;
				
				for (int i =0; i <sizeX*2; i++) vt369Sprite.dataEven[i] =RI.PRGROMData[patternAddr++];
				for (int i =0; i <sizeX*2; i++) vt369Sprite.dataOdd [i] =RI.PRGROMData[patternAddr++];
			} else { // 4bpp
				uint32_t patternAddr =spriteTile*sizeX*sizeY*2 +spriteSL *sizeX*2;
				patternAddr +=RI.vt369sprData;
				patternAddr &=RI.PRGROMSize -1;
				
				for (int i =0; i <sizeX; i++) {
					vt369Sprite.dataEven[i <<1 |0] =palette | RI.PRGROMData[patternAddr+ i] >>0 &0x0F;
					vt369Sprite.dataEven[i <<1 |1] =palette | RI.PRGROMData[patternAddr+ i] >>4 &0x0F;
				}
				patternAddr +=sizeX;
				for (int i =0; i <sizeX; i++) {
					vt369Sprite.dataOdd [i <<1 |0] =palette | RI.PRGROMData[patternAddr+ i] >>0 &0x0F;
					vt369Sprite.dataOdd [i <<1 |1] =palette | RI.PRGROMData[patternAddr+ i] >>4 &0x0F;
				}
				for (int i =0; i <sizeX*2; i++) {
					if (vt369Sprite.dataEven[i] ==palette) vt369Sprite.dataEven[i] =0;
					if (vt369Sprite.dataOdd [i] ==palette) vt369Sprite.dataOdd [i] =0;
				}
			}
			if (flipX) for (int i =0; i <sizeX; i++) {
				uint8_t temp =vt369Sprite.dataEven[sizeX*2 -i -1];
				vt369Sprite.dataEven[sizeX*2 -i -1] =vt369Sprite.dataEven[i];
				vt369Sprite.dataEven[i] =temp;
				
				        temp =vt369Sprite.dataOdd [sizeX*2 -i -1];
				vt369Sprite.dataOdd [sizeX*2 -i -1] =vt369Sprite.dataOdd [i];
				vt369Sprite.dataOdd [i] =temp;
			}
			if (flipY) for (int i =0; i <sizeX*2; i++) {
				uint8_t temp =vt369Sprite.dataEven[i];
				vt369Sprite.dataEven[i] =vt369Sprite.dataOdd[i];
				vt369Sprite.dataOdd [i] =temp;
			}
			vt369Sprite.priority =false;
			vt369Sprite.startX   =spriteX;
			vt369Sprite.maskX    =~(sizeX -1);
			vt369Sprite.sprite0  =!!(spriteNum ==0);
			vt369SpritesHi.push_back(vt369Sprite);

			if (spriteNum ==0) Spr0InLine =1;
		}
	}
}

} // namespace PPU