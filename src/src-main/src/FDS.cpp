#include "stdafx.h"
#include "Nintendulator.h"
#include "resource.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "NES.h"
#include "FDS.h"
#include "Debugger.h"
#include "Cheats.hpp"
#include "FDS_Patch.h"
#include <algorithm>

namespace FDS {
bool		secondCPU;

FCPURead	passCPURead;
FCPURead	passCPUReadDebug;
FCPUWrite	passCPUWrite;

bool		enableRegs;
bool		resetActive;
bool		horizontalMirroring;

uint16_t	timerCounter;
uint16_t	timerLatch;
bool		timerEnabled;
bool		timerRepeat;

bool		pendingTimer;
bool		pendingData;

bool		ready;
bool		startMotor;
bool		motorRunning;
bool		writeGate;
bool		endTrack;
bool		skipGap;
unsigned int	diskPosition;
unsigned int	rewindCounter;

unsigned int	dataCounter;
bool		dataIRQ;
bool		dataAvailable;
uint8_t		dataLatch;

bool		outputCRC;
bool		clearCRC;
int		crc;

unsigned int	cyclesSinceLast4032Read;
unsigned int	cyclesSinceDiskInserted;
bool		bounceStatus;

#include	"FDS_Trap.h"

void	crcAddByte (uint8_t data) {
	crc |=data <<16;
	for (int i =0; i <8; i++) {
		if (crc &1) crc ^=0x10810;
		crc >>=1;
	}
}

void	checkIRQ (void) {
	if (secondCPU)
		EI.SetIRQ2(1, (pendingTimer || pendingData)? 0: 1);
	else
		EI.SetIRQ((pendingTimer || pendingData)? 0: 1);
}

int	MAPINT	readRegDebug (int bank, int addr) {
	std::lock_guard<std::mutex> lock(NES::busy28);
	uint8_t result;

	switch (addr) {
		case 0x030:	result =(pendingTimer?  0x01: 0x00) |
				        (pendingData?   0x02: 0x00) |
				        (crc !=0x00?    0x10: 0x00) |
				        (endTrack?      0x40: 0x00) |
				        (dataAvailable? 0x80: 0x00);
				break;
		case 0x031:	result =dataLatch;
				break;
		case 0x032:	result =(RI.diskData28 ==NULL? 0x05: 0x00) |    // No disk inserted sets the "no disk" as well as the "write-protect" tab
				        (!startMotor || !ready?  0x02: 0x00) |
					(NES::writeProtected28? 0x04: 0x00) |
					0x40;
				break;
		case 0x033:	result =0x80;
				break;
		default:	result =passCPUReadDebug(bank, addr);
				break;
	}
	return result;
}

int	MAPINT	readReg (int bank, int addr) {
	uint8_t result;
	
	switch (addr) {
		case 0x030:	result =readRegDebug(bank, addr);
				pendingTimer =false;
				pendingData =false;
				break;
		case 0x031:	result =readRegDebug(bank, addr);
				dataAvailable =false;
				pendingData =false;
				break;
		case 0x032:	result =readRegDebug(bank, addr);
				/* Bounce the "disk inserted" bit for 32768 cycles after disk insertion for badly-programmed INSERT DISK routines such as "Aspic"'s.
				   "cyclesSinceDiskInserted" is initialized to 32768 by cpuCycle() after a disk has been inserted.
				   If we are are still within that interval AND fewer than 24 cycles have passed since the last 4032 read,
				   conclude that we are stuck in such a routine, and flip the "disk inserted" bit.
				   Well-programmed routines will take way more than 24 cycles until the next 4032 read, and so are not affected by this method. */
				if (cyclesSinceDiskInserted >0 && cyclesSinceLast4032Read <24) {
					result |=bounceStatus? 1: 0;
					bounceStatus =!bounceStatus;
				}
				cyclesSinceLast4032Read =0;
				break;
		case 0x033:	result =readRegDebug(bank, addr);
				break;
		default:	result =passCPURead(bank, addr);
				break;
	}
	checkIRQ();
	return result;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);

	switch (addr) {
		case 0x01F:	if (Settings::FastLoad) switchToSide(val);
				break;
		case 0x020:	timerLatch =(timerLatch &0xFF00) |val;
				break;
		case 0x021:	timerLatch =(timerLatch &0x00FF) |(val <<8);
				break;
		case 0x022:	if (enableRegs) {
					timerRepeat =!!(val &1);
					timerEnabled =!!(val &2);
					if (timerEnabled)
						timerCounter =timerLatch;
					else
						pendingTimer =false;
				}
				break;
		case 0x023:	enableRegs =!!(val &1);
				if (!enableRegs) pendingTimer =false;
				break;
		case 0x024:	pendingData =false;				
				if (enableRegs) {
					dataLatch =val;
					dataAvailable =false;
				}
				break;
		case 0x025:	pendingData =false;
				if (enableRegs) {
					// If we are writing and a CRC value is about to be written (rising edge on bit 4),
					// feed two empty bytes to the CRC routine to yield the correct value.
					if (!outputCRC && val &0x10 && ~val &0x04) {
						crcAddByte(0);
						crcAddByte(0);
					}
					resetActive =!(val &0x01);
					startMotor =!(val &0x02);
					writeGate =!(val &0x04);
					outputCRC =!!(val &0x10);
					clearCRC =!(val &0x40);
					dataIRQ =!!(val &0x80);
					dataCounter =0;					
				}
				break;
	}
	checkIRQ();
}

void	MAPINT	load (bool _secondCPU) {
	secondCPU =_secondCPU;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	int bank =secondCPU? 0x14: 0x04;
	passCPURead =EI.GetCPUReadHandler(bank);
	passCPUReadDebug =EI.GetCPUReadHandlerDebug(bank);
	passCPUWrite =EI.GetCPUWriteHandler(bank);

	EI.SetCPUReadHandler(bank, readReg);
	EI.SetCPUReadHandlerDebug(bank, readRegDebug);
	EI.SetCPUWriteHandler(bank, writeReg);

	if (resetType ==RESET_HARD) {
		enableRegs =false;
		resetActive =false;
		
		timerCounter =0;
		timerLatch =0;
		timerEnabled =false;
		timerRepeat =false;
		
		pendingTimer =false;
		pendingData =false;
		
		ready =false;
		startMotor =false;
		motorRunning =false;
		writeGate =false;
		endTrack =true;
		skipGap =false;
		diskPosition =0;
		rewindCounter =0;

		dataCounter =0;
		dataIRQ =false;
		dataAvailable =false;
		dataLatch =0;
		
		outputCRC =false;
		clearCRC =false;
		crc =0;
		
		cyclesSinceLast4032Read =0;
		cyclesSinceDiskInserted =0;
		bounceStatus =false;
		if (!secondCPU && Settings::FastLoad) initTrap();
	}
	checkIRQ();
}

void	MAPINT	cpuCycle (void) {
	if (enableRegs && timerEnabled) {
		if (timerCounter ==0) {
			pendingTimer =true;
			if (timerRepeat)
				timerCounter =timerLatch;
			else
				timerEnabled =false;
		} else
			--timerCounter;
	}
	if (cyclesSinceDiskInserted) {		// Count down the CPU cycles since disk insertion
		if (!--cyclesSinceDiskInserted)
			cyclesSinceLast4032Read =0;
		else				// Increase the CPU cycle counter since the last 4032 read
			cyclesSinceLast4032Read++;
	}
	// If a disk has been newly inserted (the disk status has "changed" and the disk data pointer points to disk data), start the "bouncing" routine.
	if (RI.changedDisk28 && RI.diskData28 !=NULL) {
		std::lock_guard<std::mutex> lock(NES::busy28);
		RI.changedDisk28 =false;
		cyclesSinceDiskInserted =32768;	// Bounce for 32768 CPU cycles after insertion
		bounceStatus =false;
		
		//if (diskPosition >=RI.diskData28->size()) {
			diskPosition =0;
			endTrack =true;
			ready =false;
		//}
	}

	if (resetActive) {
		ready =false;
		startMotor =false;
	}
	if (startMotor && !motorRunning) motorRunning =true;

	dataCounter +=3;
	while (dataCounter >=448) {
		std::lock_guard<std::mutex> lock(NES::busy28);
		dataCounter -=448;
		dataAvailable =false;
		if (motorRunning) {
			if (endTrack) {
				if (!startMotor) {
					motorRunning =false;
					EI.StatusOut(_T(""));
				}
				if (rewindCounter ==0)
					rewindCounter =1800;
				else
				if (rewindCounter && !--rewindCounter) {
					endTrack =false;
					if (!writeGate) ready =true; // Signal "disk is full" if end track was reached during writing
					diskPosition =0;
				}
			} else
			if (RI.diskData28) {
				if (writeGate) { // writing
					while (diskPosition >=RI.diskData28->size() && diskPosition <75500) RI.diskData28->push_back(0x00);
					if (diskPosition <RI.diskData28->size()) {
						if (clearCRC) {
							(*RI.diskData28)[diskPosition] =0x00;
							crc =0;
						} else {
							if (outputCRC) {
								(*RI.diskData28)[diskPosition] =crc &0xFF;
								crc >>=8;
							} else {
								(*RI.diskData28)[diskPosition] =dataLatch;
								crcAddByte(dataLatch);
							}
							RI.modifiedSide28 =true;
							dataAvailable =true;
							if (dataIRQ) pendingData =true;
						}
					}
				} else { // reading
					if (diskPosition <RI.diskData28->size()) {
						// Clearing the CRC field means that data separation only starts at the next gap end mark
						if (clearCRC) skipGap =true;	
						if (skipGap) {
							if ((*RI.diskData28)[diskPosition] ==0x80) {
								skipGap =false;
								crc =0;
								crcAddByte(0x80);
							}
						} else {
							dataLatch =(*RI.diskData28)[diskPosition];
							crcAddByte(dataLatch);
							dataAvailable =true;
							if (dataIRQ) pendingData =true;
						}
					}
				}
			}
			diskPosition++;
			if (diskPosition >=75500) {
				endTrack =true;
				ready =false;
			}			
			if ((diskPosition &0xFF) ==0) EI.StatusOut(_T("%s at %i percent of disk%s"), clearCRC? _T("Skipping"): writeGate? _T("Writing"): _T("Reading"), 101*diskPosition /75500, ready? _T(" (ready)"): _T(""));
		} else
			if (dataIRQ) pendingData =true; // Magic Card games use the data IRQ without the disk spinning for frame timing
	}
	checkIRQ();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, enableRegs);
	SAVELOAD_BOOL(stateMode, offset, data, resetActive);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);

	SAVELOAD_WORD(stateMode, offset, data, timerCounter);
	SAVELOAD_WORD(stateMode, offset, data, timerLatch);
	SAVELOAD_BOOL(stateMode, offset, data, timerEnabled);
	SAVELOAD_BOOL(stateMode, offset, data, timerRepeat);

	SAVELOAD_BOOL(stateMode, offset, data, pendingTimer);
	SAVELOAD_BOOL(stateMode, offset, data, pendingData);

	SAVELOAD_BOOL(stateMode, offset, data, ready);
	SAVELOAD_BOOL(stateMode, offset, data, startMotor);
	SAVELOAD_BOOL(stateMode, offset, data, motorRunning);
	SAVELOAD_BOOL(stateMode, offset, data, writeGate);
	SAVELOAD_BOOL(stateMode, offset, data, endTrack);
	SAVELOAD_BOOL(stateMode, offset, data, skipGap);
	SAVELOAD_LONG(stateMode, offset, data, diskPosition);
	SAVELOAD_LONG(stateMode, offset, data, rewindCounter);

	SAVELOAD_LONG(stateMode, offset, data, dataCounter);
	SAVELOAD_BOOL(stateMode, offset, data, dataIRQ);
	SAVELOAD_BOOL(stateMode, offset, data, dataAvailable);
	SAVELOAD_BYTE(stateMode, offset, data, dataLatch);

	SAVELOAD_BOOL(stateMode, offset, data, outputCRC);
	SAVELOAD_BOOL(stateMode, offset, data, clearCRC);
	SAVELOAD_LONG(stateMode, offset, data, crc);

	SAVELOAD_LONG(stateMode, offset, data, cyclesSinceLast4032Read);
	SAVELOAD_LONG(stateMode, offset, data, cyclesSinceDiskInserted);
	SAVELOAD_BOOL(stateMode, offset, data, bounceStatus);
	
	if (stateMode ==STATE_LOAD) checkIRQ();
	return offset;
}

} // namespace FDS