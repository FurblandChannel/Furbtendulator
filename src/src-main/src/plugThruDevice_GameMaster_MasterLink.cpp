#include "stdafx.h"

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "FDS.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_GameMaster.hpp"

namespace PlugThruDevice {
namespace GameMaster {
BIOS_ML		biosML;
BIOS_ML		biosFDS;
uint8_t		memory[32768];

uint8_t		parallelDataML;
uint8_t		parallelStateML;
bool		mapFDSBIOS;

void	syncML (void) {
	EI.SetPRG_Ptr4(0x12, &biosML.k4[0][0], FALSE);
	EI.SetPRG_Ptr4(0x13, &biosML.k4[1][0], FALSE);
	EI.SetPRG_Ptr4(0x16, memory+0x0000, TRUE);
	EI.SetPRG_Ptr4(0x17, memory+0x1000, TRUE);
	EI.SetPRG_Ptr4(0x18, memory+0x2000, TRUE);
	EI.SetPRG_Ptr4(0x19, memory+0x3000, TRUE);
	EI.SetPRG_Ptr4(0x1A, memory+0x4000, TRUE);
	EI.SetPRG_Ptr4(0x1B, memory+0x5000, TRUE);
	EI.SetPRG_Ptr4(0x1C, memory+0x6000, TRUE);
	EI.SetPRG_Ptr4(0x1D, memory+0x7000, TRUE);

	if (mapFDSBIOS) {
		EI.SetPRG_Ptr4(0x1E, &biosFDS.k4[0][0], FALSE);
		EI.SetPRG_Ptr4(0x1F, &biosFDS.k4[1][0], FALSE);
	} else {
		EI.SetPRG_Ptr4(0x1E, &biosML.k4[0][0], FALSE);
		EI.SetPRG_Ptr4(0x1F, &biosML.k4[1][0], FALSE);
	}
	FDS::checkIRQ();
}

int	MAPINT	readMLDebug (int bank, int addr) {
	uint8_t result;

	switch (addr) {
		case 0x183:	result =transferByte;
				break;
		case 0x184:	result =(parallelStateGM ==0xCE? 0x00: 0x08) | (parallelStateGM ==0xCB? 0x00: 0x02);
				break;
		default:	result =FDS::readRegDebug(bank, addr);
				break;
	}
	return result;	
}
int	MAPINT	readML (int bank, int addr) {
	uint8_t result;
	switch (addr) {
		case 0x183:	result =transferByte;
				break;
		case 0x184:	result =(parallelStateGM ==0xCE? 0x00: 0x08) | (parallelStateGM ==0xCB? 0x00: 0x02);
				break;
		default:	result =FDS::readReg(bank, addr);
				break;
	}
	return result;
}
void	MAPINT	writeML (int bank, int addr, int val) {
	switch (addr) {
		case 0x180:	mapFDSBIOS =!!(val &1);
				syncML();
				break;
		case 0x183:	parallelDataML =val;
				break;
		case 0x184:	parallelStateML =val;
				if (parallelStateML ==0xCB) transferByte =parallelDataML;
				break;
		default:	FDS::writeReg(bank, addr, val);
				break;
	}
}

void	ml_load (void) {
	if (loadBIOS (_T("BIOS\\DISKSYS.ROM"), biosFDS.b, sizeof(biosFDS.b)) && loadBIOS (_T("BIOS\\BML.BIN"), biosML.b, sizeof(biosML.b))) {
		FDS::load(true);
	} else
		enableMasterLink =false;
}

void	ml_reset (void) {
	EnableMenuItem(hMenu, ID_FILE_REPLACE, MF_ENABLED);
	
	FDS::reset(RESET_HARD);
	FDS::writeReg(0x14, 0x017, 0x40);
	FCPURead readCart =EI.GetCPUReadHandler(0x18);
	FCPURead readCartDebug =EI.GetCPUReadHandlerDebug(0x18);
	FCPUWrite writeCart =EI.GetCPUWriteHandler(0x18);
	EI.SetCPUReadHandler(0x12, readCart);
	EI.SetCPUReadHandler(0x13, readCart);
	EI.SetCPUReadHandlerDebug(0x12, readCartDebug);
	EI.SetCPUReadHandlerDebug(0x13, readCartDebug);
	EI.SetCPUWriteHandler(0x12, writeCart);
	EI.SetCPUWriteHandler(0x13, writeCart);
	
	EI.SetCPUReadHandler(0x14, readML);
	EI.SetCPUReadHandlerDebug(0x14, readMLDebug);
	EI.SetCPUWriteHandler(0x14, writeML);
	
	mapFDSBIOS =false;
	syncML();
}

void	ml_cpuCycle (void) {
	FDS::cpuCycle();
}

int	ml_saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =FDS::saveLoad(stateMode, offset, data);
	for (auto& c: memory) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, parallelDataML);
	SAVELOAD_BYTE(stateMode, offset, data, parallelStateML);
	SAVELOAD_BOOL(stateMode, offset, data, mapFDSBIOS);
	return offset;
}
} // namespace GameMaster
} // namespace PlugThruDevice