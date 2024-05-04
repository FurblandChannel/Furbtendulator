#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_DoctorPCJr.hpp"
#include "PPU.h"

#include "CPU.h"
#include "NES.h"
#include "FDS.h"
#include "simplefdc.hpp"
#include <queue>
#include <mutex>

#define enableSRAM  !!(regs[0x00] &0x80)
#define biosBank      (regs[0x00] &7)
#define machineMode   (regs[0x01] >>4 &3)
#define newPRGSize    (regs[0x01] >>2 &3)
#define newCHRSize    (regs[0x01] >>0 &3)
#define prgMask       (regs[0x03] >>4 &15)
#define chrMask       (regs[0x03] >>0 &15)
#define MACHINE_OLD  0
#define MACHINE_NEW  1
#define MACHINE_MMC1 2
#define MACHINE_MMC3 3

namespace PlugThruDevice {
namespace DoctorPCJr {
int	chineseBank =0;
union {
	uint8_t	k4 [8][4096];
	uint8_t	b  [32*1024];
} bios;
union {
	uint8_t k8 [64][2][4096];
	uint8_t k16[32][4][4096];
	uint8_t k32[16][8][4096];
	uint8_t	b  [512*1024];
} prgram;
union {
	uint8_t k1 [512][1024];
	uint8_t k2 [256][2][1024];
	uint8_t k4 [128][4][1024];
	uint8_t k8 [64][8][1024];
	uint8_t b  [512*1024];
} chrram;
union {
	uint8_t k4 [2][4096];
	uint8_t	b  [8*1024];
} sram;

uint8_t		exram[0x1000];

bool		loadMode;
uint8_t		regs[0x20];
uint8_t		autoSwitchBank;
uint8_t		exramBank;

uint8_t		mirroring; // via 42FC-42FF
FDC		fdc;


#define KBD_STATE_IDLE              0	// data and clock high
#define KBD_STATE_RECV_REQUEST      1	// clock pulled low
#define KBD_STATE_RECV_START_BIT    2	// data pulled low, clock released: expect start bit
#define KBD_STATE_RECV_DATA_BIT_7   3
#define KBD_STATE_RECV_DATA_BIT_6   4
#define KBD_STATE_RECV_DATA_BIT_5   5
#define KBD_STATE_RECV_DATA_BIT_4   6
#define KBD_STATE_RECV_DATA_BIT_3   7
#define KBD_STATE_RECV_DATA_BIT_2   8
#define KBD_STATE_RECV_DATA_BIT_1   9
#define KBD_STATE_RECV_DATA_BIT_0   10
#define KBD_STATE_RECV_PARITY_BIT   11
#define KBD_STATE_RECV_STOP_BIT     12
#define KBD_STATE_RECV_ACK          13
#define KBD_STATE_SEND_START_BIT    14
#define KBD_STATE_SEND_DATA_BIT_7   15
#define KBD_STATE_SEND_DATA_BIT_6   16
#define KBD_STATE_SEND_DATA_BIT_5   17
#define KBD_STATE_SEND_DATA_BIT_4   18
#define KBD_STATE_SEND_DATA_BIT_3   19
#define KBD_STATE_SEND_DATA_BIT_2   20
#define KBD_STATE_SEND_DATA_BIT_1   21
#define KBD_STATE_SEND_DATA_BIT_0   22
#define KBD_STATE_SEND_PARITY_BIT   23
#define KBD_STATE_SEND_STOP_BIT     24

int			kbdClockCount;
bool			kbdClock;
bool			kbdData;
int			kbdLatch;
int			kbdState;
int			kbdParity;
bool			kbdRaiseIRQ;
std::queue<uint8_t>	kbdReceiveFromHost;
std::queue<uint8_t>	kbdSendToHost;
std::mutex		kbdSendToHostBusy;

void	sync();

int	MAPINT	cpuReadE (int bank, int addr) {
	return bios.k4[biosBank][addr];
}

int	MAPINT	cpuReadF (int bank, int addr) {
	return bios.k4[7][addr];
}

int	MAPINT	cpuRead4 (int bank, int addr) {
	switch(addr) {
		case 0x188:	// Floppy Disk Controller: Master Status Register
			return fdc.readIO(4);
		case 0x189:	// Floppy Disk Controller: Data Register
			return fdc.readIO(5);
		case 0x18B:	// Floppy Disk Controller: Digital Output Register
			return fdc.readIO(2);
		case 0x18E: {	// PS/2 Keyboard Interface {
			kbdRaiseIRQ =false;
			std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
			return regs[0x0E] &~0xC3 | 0xC0 |(kbdClock? 0x02: 0x00) |(kbdData? 0x01: 0x00);
		}
		case 0x18F:
			kbdRaiseIRQ =false;
			return 0x00;
		case 0x1AC:	// Speech chip
			return 0x40;
		default:
			return passCPURead(bank, addr);
	}
}
int	MAPINT	cpuRead4Debug (int bank, int addr) {
	switch(addr) {
		case 0x188:	// Floppy Disk Controller: Master Status Register
			return fdc.readIODebug(4);
		case 0x189:	// Floppy Disk Controller: Data Register
			return fdc.readIODebug(5);
		default:
			return passCPURead(bank, addr);
	}
}

void	MAPINT	cpuWrite4 (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	if (addr >=0x180 && addr <=0x19F) {
		regs[addr &0x1F] =val;
		sync();
	}
	switch(addr) {
		case 0x188:	// Floppy Disk Controller: Data Rate Select Register
			fdc.writeIO(4, val);
			break;
		case 0x189:	// Floppy Disk Controller: Data Register
			fdc.writeIO(5, val);
			break;
		case 0x18B:	// Floppy Disk Controller: Digital Output Register
			fdc.writeIO(2, val);
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF: // Enter game mode, set mirroring
			mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);	
			loadMode =false;
			sync();
			break;
	}
}

int	MAPINT	cpuReadSRAM (int bank, int addr) {
	return sram.k4[bank &1][addr];
}
void	MAPINT	cpuWriteSRAM (int bank, int addr, int val) {
	sram.k4[bank &1][addr] =val;
}

int	MAPINT	cpuReadPRG8k (int bank, int addr) {
	return prgram.k8 [regs[0x10 | bank >>1 &3] &(prgMask <<2 |3)][bank &1][addr];
}
int	MAPINT	cpuReadPRG16k (int bank, int addr) {
	return prgram.k16[regs[0x10 | bank >>1 &2] &(prgMask <<1 |1)][bank &3][addr];
}
int	MAPINT	cpuReadPRG32k (int bank, int addr) {
	return prgram.k32[regs[0x10]               & prgMask        ][bank &7][addr];
}
int	MAPINT	cpuReadPRGLoad (int bank, int addr) {
	return prgram.k32[regs[0x10]               & 0x0F           ][bank &7][addr];
}
void	MAPINT	cpuWritePRG8k (int bank, int addr, int val) {
	prgram.k8 [regs[0x10 | bank >>1 &3] &(prgMask <<2 |3)][bank &1][addr] =val;
}
void	MAPINT	cpuWritePRG16k (int bank, int addr, int val) {
	prgram.k16[regs[0x10 | bank >>1 &2] &(prgMask <<1 |1)][bank &3][addr] =val;
}
void	MAPINT	cpuWritePRG32k (int bank, int addr, int val) {
	prgram.k32[regs[0x10]               & prgMask        ][bank &7][addr] =val;
}
void	MAPINT	cpuWritePRGLoad (int bank, int addr, int val) {
	prgram.k32[regs[0x11]               & 0x0F           ][bank &7][addr] =val;
}

int	MAPINT	ppuReadCHR1k (int bank, int addr) {
	return chrram.k1[regs[0x18 |bank &7] &(chrMask <<5 |31)]         [addr];
}
int	MAPINT	ppuReadCHR2k (int bank, int addr) {
	return chrram.k2[regs[0x18 |bank &6] &(chrMask <<4 |15)][bank &1][addr];
}
int	MAPINT	ppuReadCHR4k (int bank, int addr) {
	return chrram.k4[regs[0x18 |bank &4] &(chrMask <<3 | 7)][bank &3][addr];
}
int	MAPINT	ppuReadCHR8k (int bank, int addr) {
	return chrram.k8[regs[0x18         ] &(chrMask <<2 | 3)][bank &7][addr];
}
int	MAPINT	ppuReadCHRLoad (int bank, int addr) {
	return chrram.k8[regs[0x18         ] &0x3F             ][bank &7][addr];
}
int	MAPINT	ppuAutoSwitchCHR (int bank, int addr) {
	if ((bank &4) ==(PPU::PPU[0]->Reg2000 >>1 &4))
		return ppuReadCHR8k(bank, addr);
	else
		return chrram.k4[((regs[0x18] <<1) |autoSwitchBank) &(chrMask <<3 | 7)][bank &3][addr];
}
int	MAPINT	ppuAutoSwitchCHR2 (int bank, int addr) {
	if ((bank &4) ==(PPU::PPU[0]->Reg2000 >>1 &4))
		return ppuReadCHR8k(bank, addr);
	else {
		int tile =(bank <<10 |addr) >>5 &0x7F;
		int scanline =addr &7 | addr >>1 &8;
		uint8_t result =chrram.k2[exramBank][tile >>6][tile <<4 &0x3FF |scanline];
		switch (autoSwitchBank >>(addr &8? 2: 0) &3) {
			case 0: result =0x00; break;
			case 1: result =0xFF; break;
			case 2: break;
			case 3: result^=0xFF; break;
		}
		return result;
	}
}
int	MAPINT	ppuAutoSwitchNT (int bank, int addr) {
	if (addr <0x3C0) {
		autoSwitchBank =chrram.k8[0x3F][bank &1 |4 |(regs[0x18] <<1 &2)][addr];
		exramBank =exram[bank <<10 &0x400 | addr &0x3FF];
	}
	return passPPURead(bank, addr);
}
void	MAPINT	ppuAutoSwitchNTWrite (int bank, int addr, int val) {
	exram[bank <<10 &0x400 | addr &0x3FF] =regs[0x14];
	passPPUWrite(bank, addr, val);
}

void	MAPINT	ppuWriteCHR1k (int bank, int addr, int val) {
	chrram.k1[regs[0x18]           &(chrMask <<5 |31)]          [addr] =val;
}
void	MAPINT	ppuWriteCHR2k (int bank, int addr, int val) {
	chrram.k2[regs[0x18 |bank &~1] &(chrMask <<4 |15)][bank &1][addr] =val;
}
void	MAPINT	ppuWriteCHR4k (int bank, int addr, int val) {
	chrram.k4[regs[0x18 |bank &~3] &(chrMask <<3 | 7)][bank &3][addr] =val;
}
void	MAPINT	ppuWriteCHR8k (int bank, int addr, int val) {
	chrram.k8[regs[0x18          ] &(chrMask <<2 | 3)][bank &7][addr] =val;
}
void	MAPINT	ppuWriteCHRLoad (int bank, int addr, int val) {
	chrram.k8[regs[0x18          ] &0x3F             ][bank &7][addr] =val;
}

int	MAPINT	readNT_V (int bank, int addr) {
	return PPU::PPU[0]->VRAM[bank &1][addr &0x3FF];
}
int	MAPINT	readNT_H (int bank, int addr) {
	return PPU::PPU[0]->VRAM[(bank >>1) &1][addr &0x3FF];
}
void	MAPINT	writeNT_V (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[bank &1][addr &0x3FF] =val;
}
void	MAPINT	writeNT_H (int bank, int addr, int val) {
	if (bank !=0xF || addr <0xF00) PPU::PPU[0]->VRAM[(bank >>1) &1][addr &0x3FF] =val;
}

// Common
void	sync() {
	// bank 4: registers
	SetCPUReadHandler     (0x4, cpuRead4);
	SetCPUReadHandlerDebug(0x4, cpuRead4Debug);
	SetCPUWriteHandler    (0x4, cpuWrite4);
	
	if (loadMode) {
		// bank 6-D: DRAM
		for (int bank =0x6; bank<=0xD; bank++) {
			SetCPUReadHandler     (bank, cpuReadPRGLoad);
			SetCPUReadHandlerDebug(bank, cpuReadPRGLoad);
			SetCPUWriteHandler    (bank, cpuWritePRGLoad);
		}
		
		// bank E: Switchable BIOS bank
		SetCPUReadHandler     (0xE, cpuReadE);
		SetCPUReadHandlerDebug(0xE, cpuReadE);
		SetCPUWriteHandler    (0xE, passCPUWrite);
		// bank F: Fixed BIOS bank
		SetCPUReadHandler     (0xF, cpuReadF);
		SetCPUReadHandlerDebug(0xF, cpuReadF);
		SetCPUWriteHandler    (0xF, passCPUWrite);
	
		// CHR
		for (int bank =0x0; bank<=0x7; bank++) {
			SetPPUReadHandler     (bank, ppuReadCHRLoad);
			SetPPUReadHandlerDebug(bank, ppuReadCHRLoad);
			SetPPUWriteHandler    (bank, ppuWriteCHRLoad);
		}
		if (enableSRAM) for (int bank =0x6; bank<=0x7; bank++) {
			SetCPUReadHandler     (bank, cpuReadSRAM);
			SetCPUReadHandlerDebug(bank, cpuReadSRAM);
			SetCPUWriteHandler    (bank, cpuWriteSRAM);
		}
		EI.Mirror_V();
	} else
	switch(machineMode) {
		case MACHINE_OLD:
			EI.DbgOut(L"Old Machine Mode not implemented!");
			NES::DoStop |=STOPMODE_NOW | STOPMODE_BREAK;
			break;
		case MACHINE_NEW: {
			FCPURead  readPRGs[4]  ={ cpuReadPRG8k,  cpuReadPRG16k,  cpuReadPRG32k,  cpuReadPRG32k  };
			FCPUWrite writePRGs[4] ={ cpuWritePRG8k, cpuWritePRG16k, cpuWritePRG32k, cpuWritePRG32k };
			for (int bank =0x8; bank<=0xF; bank++) {
				SetCPUReadHandler     (bank, readPRGs [newPRGSize]);
				SetCPUReadHandlerDebug(bank, readPRGs [newPRGSize]);
				SetCPUWriteHandler    (bank, writePRGs[newPRGSize]);
			}
			// Always enable SRAM
			for (int bank =0x6; bank<=0x7; bank++) {
				SetCPUReadHandler     (bank, cpuReadSRAM);
				SetCPUReadHandlerDebug(bank, cpuReadSRAM);
				SetCPUWriteHandler    (bank, cpuWriteSRAM);
			}
			FPPURead  readCHRs[4]  ={ ppuReadCHR1k,  ppuReadCHR2k,  ppuReadCHR4k,  ppuReadCHR8k  };
			FPPUWrite writeCHRs[4] ={ ppuWriteCHR1k, ppuWriteCHR2k, ppuWriteCHR4k, ppuWriteCHR8k };
			
			
			for (int bank =0x0; bank<=0x7; bank++) {
				switch(regs[0x0D] &3) {
					case 2: SetPPUReadHandler(bank, ppuAutoSwitchCHR ); break;
					case 3: SetPPUReadHandler(bank, ppuAutoSwitchCHR2); break;
					default:SetPPUReadHandler(bank, readCHRs[newCHRSize]); break;
				}
				SetPPUReadHandlerDebug(bank, readCHRs [newCHRSize]);
				SetPPUWriteHandler    (bank, writeCHRs[newCHRSize]);
			}
			switch(regs[0x02] >>4 &7) {
				case 0:	EI.Mirror_S0(); break;
				case 1:	EI.Mirror_S1(); break;
				case 2:	EI.Mirror_V (); break;
				case 3:	EI.Mirror_H (); break;
				case 4: if (mirroring &1)
						EI.Mirror_S1();
					else
						EI.Mirror_S0();
					break;
				case 5: if (mirroring &1)
						EI.Mirror_H();
					else
						EI.Mirror_V();
					break;
				case 6:
				case 7: switch (mirroring &3) {
						case 0: EI.Mirror_S0(); break;
						case 1: EI.Mirror_S1(); break;
						case 2: EI.Mirror_V (); break;
						case 3: EI.Mirror_H (); break;
					}
					break;
			}
			for (int bank =0x8; bank<=0xF; bank++) {
				SetPPUReadHandler     (bank, regs[0x0D] &0x02? ppuAutoSwitchNT: passPPURead);
				SetPPUReadHandlerDebug(bank, passPPUReadDebug);
				SetPPUWriteHandler    (bank, regs[0x0D] &0x02? ppuAutoSwitchNTWrite: passPPUWrite);
			}
			break;
		}
		case MACHINE_MMC1:
			EI.DbgOut(L"MMC1 Machine Mode not implemented!");
			NES::DoStop |=STOPMODE_NOW | STOPMODE_BREAK;
			break;
		case MACHINE_MMC3:
			EI.DbgOut(L"MMC1 Machine Mode not implemented!");
			NES::DoStop |=STOPMODE_NOW | STOPMODE_BREAK;
			break;
		default:
			break;
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\DRPCJR.BIN"), bios.b, sizeof(bios.b));
	Description =_T("Bung Doctor PC Jr.");
		
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) {
		for (auto& c: regs) c =0x00;
		for (auto& c: prgram.b) c =0x00;
		for (auto& c: chrram.b) c =0x00;
		for (auto& c: sram.b) c =0x00;
		fdc.reset();
	}
	loadMode =true;
	regs[0x0E] =0x03; // data and clock high
	sync();
	
	kbdClock =true;
	kbdClockCount =0;
	kbdLatch =0;
	kbdState =0;
	kbdRaiseIRQ =false;
}

void	MAPINT	cpuCycle (void) {
	if (RI.changedDisk35) {
		if (RI.diskData35)
			fdc.insertDisk(0, &(*RI.diskData35)[0], RI.diskData35->size(), false);
		else
			fdc.ejectDisk(0);
		RI.changedDisk35 =false;
	}
	fdc.run();
	
	//int oldState =kbdState;
	if (regs[0x0E] &0x01 && ~regs[0x0E] &0x02) kbdState =KBD_STATE_IDLE;
	
	switch(kbdState) {
		case KBD_STATE_IDLE:
			if (!kbdClock && !(kbdClockCount++ &0x3F)) kbdClock =true;
			
			if (regs[0x0E] &0x01 && ~regs[0x0E] &0x02) kbdState++;
			if (regs[0x0E] &0x01 && regs[0x0E] &0x02 && !kbdSendToHost.empty()) { // Ready to send
				kbdState =KBD_STATE_SEND_START_BIT;
				kbdClockCount =0;
				kbdClock =false; // Pull clock low so the following code can pull it high again to count as start bit				
			}
			break;
		case KBD_STATE_RECV_REQUEST:
			kbdClock =true;
			if (~regs[0x0E] &0x01 && regs[0x0E] &0x02) { // Receive Request
				kbdState++;
				kbdClockCount =0;
				kbdClock =false; // Pull clock low so the following code can pull it high again to count as start bit
			}
			if (regs[0x0E] &0x01 && regs[0x0E] &0x02 && !kbdSendToHost.empty()) { // Ready to send
				kbdState =KBD_STATE_SEND_START_BIT;
				kbdClockCount =0;
				kbdClock =false; // Pull clock low so the following code can pull it high again to count as start bit				
			}
			break;
	}
	if (kbdState >=KBD_STATE_RECV_START_BIT) {
		if ((kbdClockCount++ &0x3F) ==0) { // 64 cpu cycles ^= 35 µs between clock state changes
			kbdClock =!kbdClock;
			if (kbdClock) switch(kbdState) { // Rising edge
				case KBD_STATE_RECV_START_BIT:
					if (regs[0x0E] &0x01) {
						EI.DbgOut(L"Kbd: Start bit not 0");
						kbdState =KBD_STATE_IDLE;
					} else {
						kbdState++;
						kbdParity =0;
					}
					break;
				case KBD_STATE_RECV_DATA_BIT_7:
				case KBD_STATE_RECV_DATA_BIT_6:
				case KBD_STATE_RECV_DATA_BIT_5:
				case KBD_STATE_RECV_DATA_BIT_4:
				case KBD_STATE_RECV_DATA_BIT_3:
				case KBD_STATE_RECV_DATA_BIT_2:
				case KBD_STATE_RECV_DATA_BIT_1:
				case KBD_STATE_RECV_DATA_BIT_0:
					kbdLatch >>=1;
					if (regs[0x0E] &0x01) kbdLatch |=0x80;
					kbdParity +=regs[0x0E] &0x01;
					kbdState++;
					break;
				case KBD_STATE_RECV_PARITY_BIT:
					kbdParity +=regs[0x0E] &0x01;
					kbdState++;
					break;
				case KBD_STATE_RECV_STOP_BIT:
					if (~regs[0x0E] &0x01) {
						EI.DbgOut(L"Kbd: Stop bit not 1");
						kbdState =KBD_STATE_IDLE;
					} else
					if (~kbdParity &1) {
						EI.DbgOut(L"Kbd: Parity error");
						kbdState =KBD_STATE_IDLE;
					} else {						
						kbdReceiveFromHost.push(kbdLatch);
						kbdState++;
					}
					break;
				case KBD_STATE_RECV_ACK: {
					std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
					kbdData =false; // pull data line low one released by host. 
					kbdState =kbdSendToHost.empty()? KBD_STATE_IDLE: KBD_STATE_SEND_START_BIT;
					break;
				}
			} else switch(kbdState) { // Falling edge*/
				case KBD_STATE_SEND_START_BIT: {
					//EI.DbgOut(L"send start bit");
					std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
					kbdData =false;
					kbdLatch =kbdSendToHost.front();
					kbdSendToHost.pop();
					kbdParity =0;
					kbdState++;
					break;
				}
				case KBD_STATE_SEND_DATA_BIT_7:
				case KBD_STATE_SEND_DATA_BIT_6:
				case KBD_STATE_SEND_DATA_BIT_5:
				case KBD_STATE_SEND_DATA_BIT_4:
				case KBD_STATE_SEND_DATA_BIT_3:
				case KBD_STATE_SEND_DATA_BIT_2:
				case KBD_STATE_SEND_DATA_BIT_1:
				case KBD_STATE_SEND_DATA_BIT_0:
					kbdData =!!(kbdLatch &0x01);
					kbdParity +=kbdLatch &0x01;
					kbdLatch >>=1;
					kbdState++;
					break;
				case KBD_STATE_SEND_PARITY_BIT:
					kbdData =!(kbdParity &0x01);
					kbdState++;
					break;
				case KBD_STATE_SEND_STOP_BIT: {
					std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
					kbdData =true;
					kbdState =kbdSendToHost.empty()? KBD_STATE_IDLE: KBD_STATE_SEND_START_BIT;
					break;
				}
			}
		}
	}
	
	if (!kbdReceiveFromHost.empty()) {
		std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
		switch(kbdReceiveFromHost.front()) {
		case 0xFF: // Reset
			kbdReceiveFromHost.pop();
			while (!kbdSendToHost.empty()) kbdSendToHost.pop();
			kbdSendToHost.push(0xFA);
			kbdSendToHost.push(0xAA);
			break;
		case 0xEE: // Echo
			kbdReceiveFromHost.pop();
			kbdSendToHost.push(0xEE);
			break;
		case 0xF3: // Typematic
			if (kbdReceiveFromHost.size() >=2) {
				kbdReceiveFromHost.pop();
				kbdReceiveFromHost.pop();
				kbdSendToHost.push(0xFA);
			}
			break;
		default:
			kbdReceiveFromHost.pop();
			break;
		}
	}
	MaskKeyboard =TRUE;
	pdSetIRQ(kbdRaiseIRQ? 0: 1);
	
	//if (oldState !=kbdState) EI.DbgOut(L"Kbd state: %d", kbdState);
	if (adMI->CPUCycle) adMI->CPUCycle();	
}

static const uint8_t scancodeTranslation [256] = {
0xff, 0x43, 0x41, 0x3f, 0x3d, 0x3b, 0x3c, 0x58, 0x64, 0x44, 0x42, 0x40, 0x3e, 0x0f, 0x29, 0x59,
0x65, 0x38, 0x2a, 0x70, 0x1d, 0x10, 0x02, 0x5a, 0x66, 0x71, 0x2c, 0x1f, 0x1e, 0x11, 0x03, 0x5b,
0x67, 0x2e, 0x2d, 0x20, 0x12, 0x05, 0x04, 0x5c, 0x68, 0x39, 0x2f, 0x21, 0x14, 0x13, 0x06, 0x5d,
0x69, 0x31, 0x30, 0x23, 0x22, 0x15, 0x07, 0x5e, 0x6a, 0x72, 0x32, 0x24, 0x16, 0x08, 0x09, 0x5f,
0x6b, 0x33, 0x25, 0x17, 0x18, 0x0b, 0x0a, 0x60, 0x6c, 0x34, 0x35, 0x26, 0x27, 0x19, 0x0c, 0x61,
0x6d, 0x73, 0x28, 0x74, 0x1a, 0x0d, 0x62, 0x6e, 0x3a, 0x36, 0x1c, 0x1b, 0x75, 0x2b, 0x63, 0x76,
0x55, 0x56, 0x77, 0x78, 0x79, 0x7a, 0x0e, 0x7b, 0x7c, 0x4f, 0x7d, 0x4b, 0x47, 0x7e, 0x7f, 0x6f,
0x52, 0x53, 0x50, 0x4c, 0x4d, 0x48, 0x01, 0x45, 0x57, 0x4e, 0x51, 0x4a, 0x37, 0x49, 0x46, 0x54,
0x80, 0x81, 0x82, 0x41, 0x54, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

void windowsMessage(UINT message, WPARAM wParam) {	
	if (message ==WM_KEYUP && wParam ==VK_ADD) {
		chineseBank++;
		EI.DbgOut(L"%02X", chineseBank);
		return;
	} else
	if (message ==WM_KEYUP && wParam ==VK_SUBTRACT) {
		chineseBank--;
		EI.DbgOut(L"%02X", chineseBank);
		return;
	}
	UINT scancode =MapVirtualKey(wParam, MAPVK_VK_TO_VSC);
	
	bool found =false;
	for (int i =0; i <256; i++) {
		if (scancodeTranslation[i] ==scancode) { 
			scancode =i;
			found =true;
			break;
		}
	}
	if (found) {
		std::lock_guard<std::mutex> lock(kbdSendToHostBusy);
		if (message ==WM_KEYUP) kbdSendToHost.push(0xF0);
		kbdSendToHost.push(scancode);
		kbdRaiseIRQ =true;
	}	
}


int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	//if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	for (auto& c: regs) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: prgram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chrram.b) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: exram) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, loadMode);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_DRPCJR;
} // namespace DoctorPCJr

MapperInfo plugThruDevice_DoctorPCJr ={
	&DoctorPCJr::deviceNum,
	_T("Bung Doctor PC Jr."),
	COMPAT_FULL,
	DoctorPCJr::load,
	DoctorPCJr::reset,
	NULL,
	DoctorPCJr::cpuCycle,
	NULL,
	DoctorPCJr::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice
/*
	DOS banking:
*/
