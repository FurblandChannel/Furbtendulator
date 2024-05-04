#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "FDS.h"
#include "OneBus.h"
#include "Sound.h"
#include "Debugger.h"
#include "Controllers.h"
#include "Cheats.hpp"

#define DMC_DEBUG 0

#define CalcAddr rCalcAddr.Full
#define CalcAddrL rCalcAddr.Segment[0]
#define CalcAddrH rCalcAddr.Segment[1]

#define TmpAddr rTmpAddr.Full
#define TmpAddrL rTmpAddr.Segment[0]
#define TmpAddrH rTmpAddr.Segment[1]

namespace CPU {
std::vector<Callback> callbacks;
	
CPU_RP2A03::CPU_RP2A03 (int _which, int _RAMSize, uint8_t* _RAM):
	which(_which),
	RAMSize(_RAMSize),
	RAMMask(_RAMSize -1),
	EnableDMA(0),
	DMAStateOAM(0),
	DMAStateDMC(0),	
	CycleCount(0),
	scheduleDMC(0),
	RAM(_RAM) {
}

CPU_RP2A03::CPU_RP2A03 (int _which, uint8_t* _RAM):
	CPU_RP2A03(_which, 2048, _RAM) {
}

CPU_RP2A03::CPU_RP2A03 (uint8_t* _RAM):
	CPU_RP2A03(0, 2048, _RAM) {
}

void CPU_RP2A03::RunCycle (void) {
	++CycleCount;
	
	LastNMI = WantNMI;
	LastIRQ = WantIRQ && !FI;
	if (scheduleDMC && !--scheduleDMC) EnableDMA |=DMA_PCM;
	if (RI.ROMType ==ROM_FDS) FDS::cpuCycle();
	CPUCycle();
	
	if (PPU::PPU[which]) PPU::PPU[which]->Run();
	if (APU::APU[which]) APU::APU[which]->Run();
	if (which ==NES::WhichScreenToShow) Sound::Run();
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

static const TCHAR *DMCStates[4] = { _T("None"), _T("Halt"), _T("Dummy"), _T("Fetch") };
static const TCHAR *OAMStates[4] = { _T("None"), _T("Halt"), _T("Fetch"), _T("Store") };

unsigned char	CPU_RP2A03::MemGet (unsigned int Addr) {
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
	RunCycle();
	LastRead =ReadHandler[Addr >>12 &0xF](which <<4 | Addr >>12 &0xF, Addr &0xFFF);
	return Cheats::readCPUMemory(Addr, LastRead);
}

unsigned char	CPU_RP2A03::MemGetA (unsigned int Addr) {
	RunCycle();
	LastRead =ReadHandler[Addr >>12 &0xF](which <<4 | Addr >>12 &0xF, Addr &0xFFF);
	return LastRead;
}

void	CPU_RP2A03::MemSet (unsigned int Addr, unsigned char Val) {
	if (EnableDMA &DMA_PCM) { /* The beginning of the DMC Halt Cycle is aligned, although it can be delayed to an unaligned cycle */
		bool isGetCycle =!!(~CycleCount &1);
		bool isLoadDMA  =!!(EnableDMA &DMA_PCM_LOAD);
		if (isLoadDMA && isGetCycle || !isLoadDMA && !isGetCycle) {
			EnableDMA &=~DMA_PCM;
			DMAStateDMC =STATE_DMC_HALT;
			#if DMC_DEBUG
			EI.DbgOut(L"%08lld: DMC (%s) delayed by write", CycleCount, isLoadDMA? L"Load": L"Reload");
			#endif
			if (EnableDMA &DMA_HALT) {
				EnableDMA &=~DMA_PCM;
				EnableDMA &=~DMA_HALT;
				DMAStateDMC =0;
				#if DMC_DEBUG
				EI.DbgOut(L"%08lld: DMA aborted before Halt", CycleCount);
				#endif
			}
		}
	}

	#if DMC_DEBUG
	if (DMAStateDMC || EnableDMA &DMA_PCM) EI.DbgOut(L"%08lld: Write to %04X", CycleCount, Addr);
	#endif
	RunCycle();
	WriteHandler[Addr >>12 &0xF](which <<4 | Addr >>12 &0xF, Addr &0xFFF, Val);
}

__forceinline	void	CPU_RP2A03::Push (unsigned char Val)
{
	MemSet(0x100 | SP--, Val);
}
__forceinline	unsigned char	CPU_RP2A03::Pull (void)
{
	return MemGet(0x100 | ++SP);
}
void	CPU_RP2A03::JoinFlags (void)
{
	P = 0x20;
	if (FC) P |= 0x01;
	if (FZ) P |= 0x02;
	if (FI) P |= 0x04;
	if (FD) P |= 0x08;
	if (FV) P |= 0x40;
	if (FN) P |= 0x80;
}
void	CPU_RP2A03::SplitFlags (void)
{
	FC = (P >> 0) & 1;
	FZ = (P >> 1) & 1;
	FI = (P >> 2) & 1;
	FD = (P >> 3) & 1;
	FV = (P >> 6) & 1;
	FN = (P >> 7) & 1;
}
void	CPU_RP2A03::DoNMI (void)
{
	MemGet(PC);
	MemGet(PC);
	Push(PCH);
	Push(PCL);
	JoinFlags();
	Push(P);
	FI = 1;

	PCL = MemGet(0xFFFA);
	PCH = MemGet(0xFFFB);
#ifdef	ENABLE_DEBUGGER
	GotInterrupt = INTERRUPT_NMI;
#endif	/* ENABLE_DEBUGGER */
}

void	CPU_RP2A03::DoIRQ (void)
{
	MemGet(PC);
	MemGet(PC);
	Push(PCH);
	Push(PCL);
	JoinFlags();
	Push(P);
	FI = 1;
	if (WantNMI) {
		WantNMI = FALSE;
		PCL = MemGet(0xFFFA);
		PCH = MemGet(0xFFFB);
	}
	else
	{
		PCL = MemGet(0xFFFE);
		PCH = MemGet(0xFFFF);
	}
#ifdef	ENABLE_DEBUGGER
	GotInterrupt = INTERRUPT_IRQ;
#endif	/* ENABLE_DEBUGGER */
}

void	CPU_RP2A03::GetHandlers (void) {
	if (which ==0 && MI && MI->CPUCycle)
		CPUCycle = MI->CPUCycle;
	else	
		CPUCycle = NoCPUCycle;
}

void	CPU_RP2A03::PowerOn (void) {
	CycleCount =0;
	CPU::RandomMemory(RAM, RAMSize);
	A = 0;
	X = 0;
	Y = 0;
	PC = 0;
	SP = 0;
	P = 0x20;
	CalcAddr = 0;
	SplitFlags();
	GetHandlers();
}

void	CPU_RP2A03::Reset (void) {
	CycleCount =0;
	DMAMiddleAddr =0;
	DMALength =0x100;
	DMATarget =0x2004;
	MemGet(PC);
	MemGet(PC);
	MemGet(0x100 | SP--);
	MemGet(0x100 | SP--);
	MemGet(0x100 | SP--);
	FI = 1;
	PCL = MemGet(0xFFFC);
	PCH = MemGet(0xFFFD);
#ifdef	ENABLE_DEBUGGER
	GotInterrupt = INTERRUPT_RST;
#endif	/* ENABLE_DEBUGGER */
	WantIRQ =WantNMI =0;
	EnableDMA =0;
	DMAStateDMC =0;
	DMAStateOAM =0;
	scheduleDMC =0;
}

void	CPU_RP2A03::Unload (void) {
}

int	CPU_RP2A03::Save (FILE *out) {
	int clen = 0;
		//Data
	writeByte(PCH);		//	PCL	uint8		Program Counter, low byte
	writeByte(PCL);		//	PCH	uint8		Program Counter, high byte
	writeByte(A);		//	A	uint8		Accumulator
	writeByte(X);		//	X	uint8		X register
	writeByte(Y);		//	Y	uint8		Y register
	writeByte(SP);		//	SP	uint8		Stack pointer
	JoinFlags();
	writeByte(P);		//	P	uint8		Processor status register
	writeByte(LastRead);	//	BUS	uint8		Last contents of data bus
	writeByte(WantNMI);	//	NMI	uint8		TRUE if falling edge detected on /NMI
	writeByte(WantIRQ);	//	IRQ	uint8		Flags 1=FRAME, 2=DPCM, 4=EXTERNAL
	writeArray(RAM, RAMSize);	//	RAM	uint8[0x800]	2KB work RAM
	write64(CycleCount);
	return clen;
}

int	CPU_RP2A03::Load (FILE *in, int version_id) {
	int clen = 0;
	readByte(PCH);		//	PCL	uint8		Program Counter, low byte
	readByte(PCL);		//	PCH	uint8		Program Counter, high byte
	readByte(A);		//	A	uint8		Accumulator
	readByte(X);		//	X	uint8		X register
	readByte(Y);		//	Y	uint8		Y register
	readByte(SP);		//	SP	uint8		Stack pointer
	readByte(P);		//	P	uint8		Processor status register
	SplitFlags();
	readByte(LastRead);	//	BUS	uint8		Last contents of data bus
	readByte(WantNMI);	//	NMI	uint8		TRUE if falling edge detected on /NMI
	readByte(WantIRQ);	//	IRQ	uint8		Flags 1=FRAME, 2=DPCM, 4=EXTERNAL
	readArray(RAM, RAMSize);	//	RAM	uint8[0x800]	2KB work RAM
	read64(CycleCount);
	return clen;
}

__forceinline	void	CPU_RP2A03::AM_IMP (void)
{
	MemGet(PC);
}
__forceinline	void	CPU_RP2A03::AM_IMM (void)
{
	CalcAddr = PC++;
}
__forceinline	void	CPU_RP2A03::AM_ABS (void)
{
	CalcAddrL = MemGet(PC++);
	CalcAddrH = MemGet(PC++);
}
__forceinline	void	CPU_RP2A03::AM_REL (void)
{
	BranchOffset = MemGet(PC++);
}
__forceinline	void	CPU_RP2A03::AM_ABX (void)
{
	CalcAddrL = MemGet(PC++);
	CalcAddrH = MemGet(PC++);
	bool inc = (CalcAddrL + X) >= 0x100;
	CalcAddrL += X;
	if (inc) {
		MemGet(CalcAddr);
		CalcAddrH++;
	}
}
__forceinline	void	CPU_RP2A03::AM_ABXW (void)
{
	CalcAddrL = MemGet(PC++);
	CalcAddrH = MemGet(PC++);
	bool inc = (CalcAddrL + X) >= 0x100;
	CalcAddrL += X;
	MemGet(CalcAddr);
	if (inc)
		CalcAddrH++;
}
__forceinline	void	CPU_RP2A03::AM_ABY (void)
{
	CalcAddrL = MemGet(PC++);
	CalcAddrH = MemGet(PC++);
	bool inc = (CalcAddrL + Y) >= 0x100;
	CalcAddrL += Y;
	if (inc) {
		MemGet(CalcAddr);
		CalcAddrH++;
	}
}
__forceinline	void	CPU_RP2A03::AM_ABYW (void)
{
	CalcAddrL = MemGet(PC++);
	CalcAddrH = MemGet(PC++);
	bool inc = (CalcAddrL + Y) >= 0x100;
	CalcAddrL += Y;
	MemGet(CalcAddr);
	if (inc)
		CalcAddrH++;
}
__forceinline	void	CPU_RP2A03::AM_ZPG (void)
{
	CalcAddr = MemGet(PC++);
}
__forceinline	void	CPU_RP2A03::AM_ZPX (void)
{
	CalcAddr = MemGet(PC++);
	MemGet(CalcAddr);
	CalcAddrL = CalcAddrL + X;
}
__forceinline	void	CPU_RP2A03::AM_ZPY (void)
{
	CalcAddr = MemGet(PC++);
	MemGet(CalcAddr);
	CalcAddrL = CalcAddrL + Y;
}
__forceinline	void	CPU_RP2A03::AM_INX (void)
{
	TmpAddr = MemGet(PC++);
	MemGet(TmpAddr);
	TmpAddrL = TmpAddrL + X;
	CalcAddrL = MemGet(TmpAddr);
	TmpAddrL++;
	CalcAddrH = MemGet(TmpAddr);
}
__forceinline	void	CPU_RP2A03::AM_INY (void)
{
	TmpAddr = MemGet(PC++);
	CalcAddrL = MemGet(TmpAddr);
	TmpAddrL++;
	CalcAddrH = MemGet(TmpAddr);
	bool inc = (CalcAddrL + Y) >= 0x100;
	CalcAddrL += Y;
	if (inc) {
		MemGet(CalcAddr);
		CalcAddrH++;
	}
}
__forceinline	void	CPU_RP2A03::AM_INYW (void)
{
	TmpAddr = MemGet(PC++);
	CalcAddrL = MemGet(TmpAddr);
	TmpAddrL++;
	CalcAddrH = MemGet(TmpAddr);
	bool inc = (CalcAddrL + Y) >= 0x100;
	CalcAddrL += Y;
	MemGet(CalcAddr);
	if (inc)
		CalcAddrH++;
}
__forceinline	void	CPU_RP2A03::AM_NON (void)
{
	// placeholder for instructions that don't use any of the above addressing modes
}

void	CPU_RP2A03::IN_ADC (void) {
	unsigned char Val = MemGet(CalcAddr);
	int result = A + Val + FC;
	FV = !!(~(A ^ Val) & (A ^ result) & 0x80);
	FC = !!(result & 0x100);
	A = result & 0xFF;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}

__forceinline	void	CPU_RP2A03::IN_AND (void)
{
	A &= MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_ASL (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	FC = (TmpData >> 7) & 1;
	TmpData <<= 1;
	FZ = (TmpData == 0);
	FN = (TmpData >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_ASLA (void)
{
	FC = (A >> 7) & 1;
	A <<= 1;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
void	CPU_RP2A03::IN_BRANCH (bool Condition)
{
	if (Condition)
	{
		// If an interrupt occurs during the final cycle of a non-pagecrossing branch
		// then it will be ignored until the next instruction completes
		BOOL SkipNMI = FALSE;
		BOOL SkipIRQ = FALSE;
		if (WantNMI && !LastNMI)
			SkipNMI = TRUE;
		if (WantIRQ && !LastIRQ)
			SkipIRQ = TRUE;
		MemGet(PC);
		if (SkipNMI)
			LastNMI = FALSE;
		if (SkipIRQ)
			LastIRQ = FALSE;
		bool inc = (PCL + BranchOffset) >= 0x100;
		PCL += BranchOffset;
		if (BranchOffset & 0x80)
		{
			if (!inc)
			{
				MemGet(PC);
				PCH--;
			}
		}
		else
		{
			if (inc)
			{
				MemGet(PC);
				PCH++;
			}
		}
	}
}
__forceinline	void	CPU_RP2A03::IN_BCS (void) {	IN_BRANCH( FC);	}
__forceinline	void	CPU_RP2A03::IN_BCC (void) {	IN_BRANCH(!FC);	}
__forceinline	void	CPU_RP2A03::IN_BEQ (void) {	IN_BRANCH( FZ);	}
__forceinline	void	CPU_RP2A03::IN_BNE (void) {	IN_BRANCH(!FZ);	}
__forceinline	void	CPU_RP2A03::IN_BMI (void) {	IN_BRANCH( FN);	}
__forceinline	void	CPU_RP2A03::IN_BPL (void) {	IN_BRANCH(!FN);	}
__forceinline	void	CPU_RP2A03::IN_BVS (void) {	IN_BRANCH( FV);	}
__forceinline	void	CPU_RP2A03::IN_BVC (void) {	IN_BRANCH(!FV);	}
__forceinline	void	CPU_RP2A03::IN_BIT (void)
{
	unsigned char Val = MemGet(CalcAddr);
	FV = (Val >> 6) & 1;
	FN = (Val >> 7) & 1;
	FZ = (Val & A) == 0;
}
void	CPU_RP2A03::IN_BRK (void)
{
	MemGet(CalcAddr);
	Push(PCH);
	Push(PCL);
	JoinFlags();
	Push(P | 0x10);
	FI = 1;
	if (WantNMI)
	{
		WantNMI = FALSE;
		PCL = MemGet(0xFFFA);
		PCH = MemGet(0xFFFB);
	}
	else
	{
		PCL = MemGet(0xFFFE);
		PCH = MemGet(0xFFFF);
	}
#ifdef	ENABLE_DEBUGGER
	GotInterrupt = INTERRUPT_BRK;
#endif	/* ENABLE_DEBUGGER */
}
__forceinline	void	CPU_RP2A03::IN_CLC (void) { FC = 0; }
__forceinline	void	CPU_RP2A03::IN_CLD (void) { FD = 0; }
__forceinline	void	CPU_RP2A03::IN_CLI (void) { FI = 0; }
__forceinline	void	CPU_RP2A03::IN_CLV (void) { FV = 0; }
__forceinline	void	CPU_RP2A03::IN_CMP (void)
{
	int result = A - MemGet(CalcAddr);
	FC = (result >= 0);
	FZ = (result == 0);
	FN = (result >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_CPX (void)
{
	int result = X - MemGet(CalcAddr);
	FC = (result >= 0);
	FZ = (result == 0);
	FN = (result >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_CPY (void)
{
	int result = Y - MemGet(CalcAddr);
	FC = (result >= 0);
	FZ = (result == 0);
	FN = (result >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_DEC (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	TmpData--;
	FZ = (TmpData == 0);
	FN = (TmpData >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_DEX (void)
{
	X--;
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_DEY (void)
{
	Y--;
	FZ = (Y == 0);
	FN = (Y >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_EOR (void)
{
	A ^= MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_INC (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	TmpData++;
	FZ = (TmpData == 0);
	FN = (TmpData >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_INX (void)
{
	X++;
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_INY (void)
{
	Y++;
	FZ = (Y == 0);
	FN = (Y >> 7) & 1;
}

void	CPU_RP2A03::IN_JMP (void) {
	PC =CalcAddr;
}

__forceinline	void	CPU_RP2A03::IN_JMPI (void) {
	PCL = MemGet(CalcAddr);
	CalcAddrL++;
	PCH = MemGet(CalcAddr);
}

__forceinline	void	CPU_RP2A03::IN_JSR (void)
{
	TmpAddrL = MemGet(PC++);
	MemGet(0x100 | SP);
	Push(PCH);
	Push(PCL);
	PCH = MemGet(PC);
	PCL = TmpAddrL;
}
__forceinline	void	CPU_RP2A03::IN_LDA (void)
{
	A = MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_LDX (void)
{
	X = MemGet(CalcAddr);
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_LDY (void)
{
	Y = MemGet(CalcAddr);
	FZ = (Y == 0);
	FN = (Y >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_LSR (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	FC = (TmpData & 0x01);
	TmpData >>= 1;
	FZ = (TmpData == 0);
	FN = 0;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_LSRA (void)
{
	FC = (A & 0x01);
	A >>= 1;
	FZ = (A == 0);
	FN = 0;
}
__forceinline	void	CPU_RP2A03::IN_NOP (void)
{
}
__forceinline	void	CPU_RP2A03::IN_ORA (void)
{
	A |= MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_PHA (void)
{
	Push(A);
}
__forceinline	void	CPU_RP2A03::IN_PHP (void)
{
	JoinFlags();
	Push(P | 0x10);
}
__forceinline	void	CPU_RP2A03::IN_PLA (void)
{
	MemGet(0x100 | SP);
	A = Pull();
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_PLP (void)
{
	MemGet(0x100 | SP);
	P = Pull();
	SplitFlags();
}
__forceinline	void	CPU_RP2A03::IN_ROL (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	unsigned char carry = FC;
	FC = (TmpData >> 7) & 1;
	TmpData = (TmpData << 1) | carry;
	FZ = (TmpData == 0);
	FN = (TmpData >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_ROLA (void)
{
	unsigned char carry = FC;
	FC = (A >> 7) & 1;
	A = (A << 1) | carry;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_ROR (void)
{
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	unsigned char carry = FC;
	FC = (TmpData & 0x01);
	TmpData = (TmpData >> 1) | (carry << 7);
	FZ = (TmpData == 0);
	FN = !!carry;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IN_RORA (void)
{
	unsigned char carry = FC;
	FC = (A & 0x01);
	A = (A >> 1) | (carry << 7);
	FZ = (A == 0);
	FN = !!carry;
}
__forceinline	void	CPU_RP2A03::IN_RTI (void)
{
	MemGet(0x100 | SP);
	P = Pull();
	SplitFlags();
	PCL = Pull();
	PCH = Pull();
}
__forceinline	void	CPU_RP2A03::IN_RTS (void)
{
	MemGet(0x100 | SP);
	PCL = Pull();
	PCH = Pull();
	MemGet(PC++);
}

void	CPU_RP2A03::IN_SBC (void) {
	unsigned char Val = MemGet(CalcAddr);
	int result = A + ~Val + FC;
	FV = !!((A ^ Val) & (A ^ result) & 0x80);
	FC = !(result & 0x100);
	A = result & 0xFF;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
/*
std::vector<uint32_t> loggedOffsets;
void	writeLog(uint16_t from, uint16_t address, uint8_t value) {
	if (from &0x8000 && address &0x8000) {
		uint8_t* bankStart =EI.GetPRG_Ptr4(from >>12);
		uint32_t romOffset =bankStart -RI.PRGROMData +(from &0x0FFF);
		bool found =false;
		for (auto& l: loggedOffsets) if (l ==romOffset) { found =true; break; }
		if (!found) {
			loggedOffsets.push_back(romOffset);
			address =address &0xE001;
			EI.DbgOut(L"chgFile file.nes at %05X with %02x %02X", romOffset +0x11, address &0xFF, address >>8);
		}
	}
}
*/
__forceinline	void	CPU_RP2A03::IN_SEC (void) { FC = 1; }
__forceinline	void	CPU_RP2A03::IN_SED (void) { FD = 1; }
__forceinline	void	CPU_RP2A03::IN_SEI (void) { FI = 1; }
__forceinline	void	CPU_RP2A03::IN_STA (void)
{
	//if (RI.INES_MapperNum ==263) writeLog(PC -3, CalcAddr, A);
	MemSet(CalcAddr, A);
}
__forceinline	void	CPU_RP2A03::IN_STX (void)
{
	//if (RI.INES_MapperNum ==263) writeLog(PC -3, CalcAddr, X);
	MemSet(CalcAddr, X);
}
__forceinline	void	CPU_RP2A03::IN_STY (void)
{
	//if (RI.INES_MapperNum ==263) writeLog(PC -3, CalcAddr, Y);
	MemSet(CalcAddr, Y);
}
__forceinline	void	CPU_RP2A03::IN_TAX (void)
{
	X = A;
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_TAY (void)
{
	Y = A;
	FZ = (Y == 0);
	FN = (Y >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_TSX (void)
{
	X = SP;
	FZ = (X == 0);
	FN = (X >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_TXA (void)
{
	A = X;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IN_TXS (void)
{
	SP = X;
}
__forceinline	void	CPU_RP2A03::IN_TYA (void)
{
	A = Y;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}

__forceinline	void	CPU_RP2A03::IV_XAA (void) {
	A = (A | 0xFF) & X & MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}

__forceinline	void	CPU_RP2A03::IV_UNK (void)
{
	// This isn't affected by the "Log Invalid Ops" toggle
	//EI.DbgOut(_T("Unsupported opcode $%02X (???) encountered at $%04X"), Opcode, OpAddr);
}

__forceinline	void	CPU_RP2A03::IV_HLT (void)
{
	// And neither is this, considering it's going to be followed immediately by a message box
	//EI.DbgOut(_T("HLT opcode $%02X encountered at $%04X; CPU locked"), Opcode, OpAddr);
	//MessageBox(hMainWnd, _T("Bad opcode, CPU locked"), _T("Nintendulator"), MB_OK);
	//NES::DoStop = STOPMODE_NOW | STOPMODE_BREAK;
}
__forceinline	void	CPU_RP2A03::IV_NOP (void)
{
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (NOP) encountered at $%04X"), Opcode, OpAddr);*/
	MemGet(CalcAddr);
}
__forceinline	void	CPU_RP2A03::IV_NOP2 (void)
{
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (NOP) encountered at $%04X"), Opcode, OpAddr);*/
}
__forceinline	void	CPU_RP2A03::IV_SLO (void)
{	// ASL + ORA
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (SLO) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	FC = (TmpData >> 7) & 1;
	TmpData <<= 1;
	A |= TmpData;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_RLA (void)
{	// ROL + AND
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (RLA) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	unsigned char carry = FC;
	FC = (TmpData >> 7) & 1;
	TmpData = (TmpData << 1) | carry;
	A &= TmpData;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_SRE (void)
{	// LSR + EOR
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (SRE) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	FC = (TmpData & 0x01);
	TmpData >>= 1;
	A ^= TmpData;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_RRA (void)
{	// ROR + ADC
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (RRA) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	unsigned char carry = FC << 7;
	FC = (TmpData & 0x01);
	TmpData = (TmpData >> 1) | carry;
	int result = A + TmpData + FC;
	FV = !!(~(A ^ TmpData) & (A ^ result) & 0x80);
	FC = !!(result & 0x100);
	A = result & 0xFF;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_SAX (void)
{	// STA + STX
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (SAX) encountered at $%04X"), Opcode, OpAddr);*/
	MemSet(CalcAddr, A & X);
}
__forceinline	void	CPU_RP2A03::IV_LAX (void)
{	// LDA + LDX
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (LAX) encountered at $%04X"), Opcode, OpAddr);*/
	X = A = MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IV_DCP (void)
{	// DEC + CMP
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (DCP) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	TmpData--;
	int result = A - TmpData;
	FC = (result >= 0);
	FZ = (result == 0);
	FN = (result >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_ISB (void)
{	// INC + SBC
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (ISB) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char TmpData = MemGet(CalcAddr);
	MemSet(CalcAddr, TmpData);
	TmpData++;
	int result = A + ~TmpData + FC;
	FV = !!((A ^ TmpData) & (A ^ result) & 0x80);
	FC = !(result & 0x100);
	A = result & 0xFF;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	MemSet(CalcAddr, TmpData);
}
__forceinline	void	CPU_RP2A03::IV_SBC (void)
{	// NOP + SBC
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (SBC) encountered at $%04X"), Opcode, OpAddr);*/
	unsigned char Val = MemGet(CalcAddr);
	int result = A + ~Val + FC;
	FV = !!((A ^ Val) & (A ^ result) & 0x80);
	FC = !(result & 0x100);
	A = result & 0xFF;
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}

__forceinline	void	CPU_RP2A03::IV_AAC (void)
{	// ASL A+ORA and ROL A+AND, behaves strangely
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (AAC) encountered at $%04X"), Opcode, OpAddr);*/
	A &= MemGet(CalcAddr);
	FZ = (A == 0);
	FC = FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IV_ASR (void)
{	// LSR A+EOR, behaves strangely
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (ASR) encountered at $%04X"), Opcode, OpAddr);*/
	A &= MemGet(CalcAddr);
	FC = (A & 0x01);
	A >>= 1;
	FZ = (A == 0);
	FN = 0;
}
__forceinline	void	CPU_RP2A03::IV_ARR (void)
{	// ROR A+AND, behaves strangely
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (ARR) encountered at $%04X"), Opcode, OpAddr);*/
	A = ((A & MemGet(CalcAddr)) >> 1) | (FC << 7);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
	FC = (A >> 6) & 1;
	FV = FC ^ ((A >> 5) & 1);
}
__forceinline	void	CPU_RP2A03::IV_ATX (void)
{	// LDA+TAX, behaves strangely
	// documented as ANDing accumulator with data, but seemingly behaves exactly the same as LAX
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (ATX) encountered at $%04X"), Opcode, OpAddr);*/
	X = A = MemGet(CalcAddr);
	FZ = (A == 0);
	FN = (A >> 7) & 1;
}
__forceinline	void	CPU_RP2A03::IV_AXS (void)
{	// CMP+DEX, behaves strangely
	/*if (Settings::LogBadOps)
		EI.DbgOut(_T("Invalid opcode $%02X (AXS) encountered at $%04X"), Opcode, OpAddr);*/
	int result = (A & X) - MemGet(CalcAddr);
	FC = (result >= 0);
	FZ = (result == 0);
	X = result & 0xFF;
	FN = (result >> 7) & 1;
}

uint8_t	CPU_RP2A03::GetOpcode() {
	return MemGet(OpAddr = PC++);
}

void	CPU_RP2A03::ExecOp (void) {
	for (auto& cb: callbacks) if (cb.which ==which && cb.pc ==PC) cb.handler();

	Opcode =GetOpcode();
	
	switch (Opcode)	{
#define OP(op,adr,ins) case 0x##op: AM_##adr(); ins(); break;

OP(00, IMM, IN_BRK) OP(10, REL, IN_BPL) OP(08, IMP, IN_PHP) OP(18, IMP, IN_CLC) OP(04, ZPG, IV_NOP) OP(14, ZPX, IV_NOP) OP(0C, ABS, IV_NOP) OP(1C, ABX, IV_NOP)
OP(20, NON, IN_JSR) OP(30, REL, IN_BMI) OP(28, IMP, IN_PLP) OP(38, IMP, IN_SEC) OP(24, ZPG, IN_BIT) OP(34, ZPX, IV_NOP) OP(2C, ABS, IN_BIT) OP(3C, ABX, IV_NOP)
OP(40, IMP, IN_RTI) OP(50, REL, IN_BVC) OP(48, IMP, IN_PHA) OP(58, IMP, IN_CLI) OP(44, ZPG, IV_NOP) OP(54, ZPX, IV_NOP) OP(4C, ABS, IN_JMP) OP(5C, ABX, IV_NOP)
OP(60, IMP, IN_RTS) OP(70, REL, IN_BVS) OP(68, IMP, IN_PLA) OP(78, IMP, IN_SEI) OP(64, ZPG, IV_NOP) OP(74, ZPX, IV_NOP) OP(6C, ABS,IN_JMPI) OP(7C, ABX, IV_NOP)
OP(80, IMM, IV_NOP) OP(90, REL, IN_BCC) OP(88, IMP, IN_DEY) OP(98, IMP, IN_TYA) OP(84, ZPG, IN_STY) OP(94, ZPX, IN_STY) OP(8C, ABS, IN_STY) OP(9C,ABXW, IV_NOP)
OP(A0, IMM, IN_LDY) OP(B0, REL, IN_BCS) OP(A8, IMP, IN_TAY) OP(B8, IMP, IN_CLV) OP(A4, ZPG, IN_LDY) OP(B4, ZPX, IN_LDY) OP(AC, ABS, IN_LDY) OP(BC, ABX, IN_LDY)
OP(C0, IMM, IN_CPY) OP(D0, REL, IN_BNE) OP(C8, IMP, IN_INY) OP(D8, IMP, IN_CLD) OP(C4, ZPG, IN_CPY) OP(D4, ZPX, IV_NOP) OP(CC, ABS, IN_CPY) OP(DC, ABX, IV_NOP)
OP(E0, IMM, IN_CPX) OP(F0, REL, IN_BEQ) OP(E8, IMP, IN_INX) OP(F8, IMP, IN_SED) OP(E4, ZPG, IN_CPX) OP(F4, ZPX, IV_NOP) OP(EC, ABS, IN_CPX) OP(FC, ABX, IV_NOP)

OP(02, NON, IV_HLT) OP(12, NON, IV_HLT) OP(0A, IMP,IN_ASLA) OP(1A, IMP,IV_NOP2) OP(06, ZPG, IN_ASL) OP(16, ZPX, IN_ASL) OP(0E, ABS, IN_ASL) OP(1E,ABXW, IN_ASL)
OP(22, NON, IV_HLT) OP(32, NON, IV_HLT) OP(2A, IMP,IN_ROLA) OP(3A, IMP,IV_NOP2) OP(26, ZPG, IN_ROL) OP(36, ZPX, IN_ROL) OP(2E, ABS, IN_ROL) OP(3E,ABXW, IN_ROL)
OP(42, NON, IV_HLT) OP(52, NON, IV_HLT) OP(4A, IMP,IN_LSRA) OP(5A, IMP,IV_NOP2) OP(46, ZPG, IN_LSR) OP(56, ZPX, IN_LSR) OP(4E, ABS, IN_LSR) OP(5E,ABXW, IN_LSR)
OP(62, NON, IV_HLT) OP(72, NON, IV_HLT) OP(6A, IMP,IN_RORA) OP(7A, IMP,IV_NOP2) OP(66, ZPG, IN_ROR) OP(76, ZPX, IN_ROR) OP(6E, ABS, IN_ROR) OP(7E,ABXW, IN_ROR)
OP(82, IMM, IV_NOP) OP(92, NON, IV_HLT) OP(8A, IMP, IN_TXA) OP(9A, IMP, IN_TXS) OP(86, ZPG, IN_STX) OP(96, ZPY, IN_STX) OP(8E, ABS, IN_STX) OP(9E,ABYW, IV_NOP)
OP(A2, IMM, IN_LDX) OP(B2, NON, IV_HLT) OP(AA, IMP, IN_TAX) OP(BA, IMP, IN_TSX) OP(A6, ZPG, IN_LDX) OP(B6, ZPY, IN_LDX) OP(AE, ABS, IN_LDX) OP(BE, ABY, IN_LDX)
OP(C2, IMM, IV_NOP) OP(D2, NON, IV_HLT) OP(CA, IMP, IN_DEX) OP(DA, IMP,IV_NOP2) OP(C6, ZPG, IN_DEC) OP(D6, ZPX, IN_DEC) OP(CE, ABS, IN_DEC) OP(DE,ABXW, IN_DEC)
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
OP(A3, INX, IV_LAX) OP(B3, INY, IV_LAX) OP(AB, IMM, IV_ATX) OP(BB, ABY, IV_UNK) OP(A7, ZPG, IV_LAX) OP(B7, ZPY, IV_LAX) OP(AF, ABS, IV_LAX) OP(BF, ABY, IV_LAX)
OP(C3, INX, IV_DCP) OP(D3,INYW, IV_DCP) OP(CB, IMM, IV_AXS) OP(DB,ABYW, IV_DCP) OP(C7, ZPG, IV_DCP) OP(D7, ZPX, IV_DCP) OP(CF, ABS, IV_DCP) OP(DF,ABXW, IV_DCP)
OP(E3, INX, IV_ISB) OP(F3,INYW, IV_ISB) OP(EB, IMM, IV_SBC) OP(FB,ABYW, IV_ISB) OP(E7, ZPG, IV_ISB) OP(F7, ZPX, IV_ISB) OP(EF, ABS, IV_ISB) OP(FF,ABXW, IV_ISB)

#undef OP
	}

	if (LastNMI) {
		DoNMI();
		WantNMI = FALSE;
	}
	else
	if (LastIRQ)
		DoIRQ();
#ifdef	ENABLE_DEBUGGER
	else	
		GotInterrupt = 0;
#endif	/* ENABLE_DEBUGGER */
}

int	CPU_RP2A03::ReadRAM (int Bank, int Addr) {
	return RAM[((Bank <<12) |Addr) &RAMMask];
}

void	CPU_RP2A03::WriteRAM (int Bank, int Addr, int Val) {
	RAM[((Bank <<12) |Addr) &RAMMask] = (unsigned char)Val;
}

CPU_RP2A03_Decimal::CPU_RP2A03_Decimal(uint8_t* _RAM):
	CPU_RP2A03(_RAM) {
}

CPU_RP2A03_Decimal::CPU_RP2A03_Decimal(int _which, int _ramSize, uint8_t* _RAM):
	CPU_RP2A03(_which, _ramSize, _RAM) {
}

void	CPU_RP2A03_Decimal::IN_ADC (void) {
	unsigned char Val = MemGet(CalcAddr);
	if (FD) {
		int low = (A &0x0F) + (Val & 0x0F) + FC;
		int high= (A &0xF0) + (Val & 0xF0);
		if (low >=0x0A) { low -=0x0A; high +=0x10; }
		if (high>=0xA0) { high-=0xA0; }
		int result =high | (low &0x0F);
		FN = (A >> 7) & 1;
		FV = !(((A ^ Val) & 0x80) && ((A ^ result) & 0x80));
		FZ = !result;
		FC = !!(high & 0x80);
		A = result & 0xFF;
	} else {
		int result = A + Val + FC;
		FV = !!(~(A ^ Val) & (A ^ result) & 0x80);
		FC = !!(result & 0x100);
		A = result & 0xFF;
		FZ = (A == 0);
		FN = (A >> 7) & 1;
	}
}

void	CPU_RP2A03_Decimal::IN_SBC (void) {
	unsigned char Val = MemGet(CalcAddr);
	if (FD) {
		Val = 0x99 -Val;
		int low = (A &0x0F) + (Val & 0x0F) + FC;
		int high= (A &0xF0) + (Val & 0xF0);
		if (low >=0x0A) { low -=0x0A; high +=0x10; }
		if (high>=0xA0) { high-=0xA0; }
		int result =high | (low &0x0F);
		FN = (A >> 7) & 1;
		FV = !(((A ^ Val) & 0x80) && ((A ^ result) & 0x80));
		FZ = !result;
		FC = !!(high & 0x80);
		A = result & 0xFF;
	} else {
		int result = A + ~Val + FC;
		FV = !!((A ^ Val) & (A ^ result) & 0x80);
		FC = !(result & 0x100);
		A = result & 0xFF;
		FZ = (A == 0);
		FN = (A >> 7) & 1;
	}
}

CPU_RP2A03* CPU[2];

int	MAPINT	ReadUnsafe (int Bank, int Addr) {
	return 0xFF;
}

int	MAPINT	ReadRAM (int Bank, int Addr) {
	return CPU[Bank >>4]->ReadRAM(Bank, Addr);
}

void	MAPINT	WriteRAM (int Bank, int Addr, int Val) {
	CPU[Bank >>4]->WriteRAM(Bank, Addr, Val);
}

int	MAPINT	ReadPRG (int Bank, int Addr) {
	if (CPU::CPU[Bank >>4]->Readable[Bank &0xF])
		return CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF][Addr];
	else	
		//return CPU::CPU[Bank >>4]->LastRead; //-1;
		return Bank <<4 | Addr >>8;
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	if (CPU::CPU[Bank >>4]->Writable[Bank &0xF])
		CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF][Addr] = (unsigned char)Val;
}

void	PowerOn (void) {
	if (CPU[0]) CPU[0]->PowerOn();
	if (CPU[1]) CPU[1]->PowerOn();
}

void	Reset (void) {
	if (CPU[0]) CPU[0]->Reset();
	if (CPU[1]) CPU[1]->Reset();
}

void	Unload (void) {
	if (CPU[0]) CPU[0]->Unload();
	if (CPU[1]) CPU[1]->Unload();
}

void	RandomMemory(void* Destination, size_t Length) {
	unsigned char *out =(unsigned char*) Destination;
	int Value =Settings::RAMInitialization;
	switch (Value) {
		case 0x00:	for (size_t i =0; i<Length; i++) out[i] =0x00; break;
		case 0xFF:	for (size_t i =0; i<Length; i++) out[i] =0xFF; break;
		default:	for (size_t i =0; i<Length; i++) out[i] =rand() &0xFF; break;
	}
}
} // namespace CPU