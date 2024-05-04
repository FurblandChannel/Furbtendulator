#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"
#include "PPU.h"
#include "Controllers.h"
#include "CPU.h"
#include "Settings.h"
#include "FDS.h"

namespace PlugThruDevice {
namespace Bit79 {
uint8_t		bios[20480];
uint8_t		ram [8192];
uint8_t		mode;
void		sync();

bool                 printerBusy;
std::vector<uint8_t> printerData;

void	flushPrinterData () {
	TCHAR FileName[MAX_PATH] = {0};
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	while (printerData.size()) {
		if (MessageBox(hMainWnd, _T("Data has been sent to the printer. Save it?"), _T("Bit-79"), MB_YESNO) ==IDYES) {
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("Text File (*.TXT)\0") _T("*.TXT\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Save Printer Output");
			ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = _T("TXT");
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetSaveFileName(&ofn)) {			
				FILE *handle =_tfopen(FileName, _T("wb"));
				fwrite(&printerData[0], 1, printerData.size(), handle);
				fclose(handle);
				printerData.clear(); // Saving the printer data discards it. This means that selecting "Yes" but then not entering a filename returns to the "Save It?" window.
			}
		} else
			printerData.clear(); // Selecting "No" or Closing the "Save It?" window discards the printer data.
	}
}

int	MAPINT	cpuRead_printer (int bank, int addr) {
	int result =passCPURead(bank, addr);
	if (addr ==0x17) result |=printerBusy? 0x00: 0x04;
	printerBusy =false; // Printer becomes ready after one 4017 read
	return result;
}

int	MAPINT	cpuRead_ram (int bank, int addr) {
	return ram[bank <<12 |addr];
}

void	MAPINT	cpuWrite_ram (int bank, int addr, int val) {
	ram[bank <<12 |addr] =val;
	passCPUWrite(bank, addr, val);
}

int	MAPINT	cpuRead_bios (int bank, int addr) {
	return bios[bank <<12 &0x3000 | addr];
}

void	MAPINT	cpuWrite_mode (int bank, int addr, int val) {
	switch (addr >>10) {
		case 0: // printer data
			printerData.push_back(val &0xFF);
			printerBusy =true;
			break;
		case 2:	if (~mode &2 && val &2) flushPrinterData();
			mode =val;
			sync();
			break;
		case 3: // printer control
			break;

	}
}
	
int	MAPINT	readCHR_bios (int bank, int addr) {
	return bios[bank <<10 &0xC00 | addr | 0x4000];
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

void	sync() {
	if (mode &2) {
		for (int bank =0x0; bank <0xF; bank++) {
			SetPPUReadHandler(bank, passPPURead);
			SetPPUReadHandlerDebug(bank, passPPUReadDebug);
			SetPPUWriteHandler(bank, passPPUWrite);
		}
		for (int bank =0x0; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, passCPURead);
			SetCPUReadHandlerDebug(bank, passCPUReadDebug);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		SetCPUWriteHandler(0x3, passCPUWrite);
		SetCPUReadHandler(0x4, passCPURead);
		Controllers::SetDefaultInput(RI.InputType);
		if (RI.ROMType ==ROM_FDS && Settings::FastLoad) FDS::initTrap();
		MaskKeyboard = FALSE;
	} else {
		for (int bank =0x0; bank <0x8; bank++) {
			SetPPUReadHandler(bank, readCHR_bios);
			SetPPUReadHandlerDebug(bank, readCHR_bios);
		}
		for (int bank =0x0; bank <0x2; bank++) {
			SetCPUReadHandler(bank, cpuRead_ram);
			SetCPUReadHandlerDebug(bank, cpuRead_ram);
			SetCPUWriteHandler(bank, cpuWrite_ram);
		}
		for (int bank =0x8; bank<=0xB; bank++) {
			SetCPUReadHandler(bank, passCPURead);
			SetCPUReadHandlerDebug(bank, passCPUReadDebug);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		for (int bank =0xC; bank<=0xF; bank++) {
			SetCPUReadHandler(bank, cpuRead_bios);
			SetCPUReadHandlerDebug(bank, cpuRead_bios);
			SetCPUWriteHandler(bank, passCPUWrite);
		}
		for (int bank =0x8; bank<=0xF; bank++) {
			SetPPUReadHandler     (bank, mode &1? readNT_H:  readNT_V);
			SetPPUReadHandlerDebug(bank, mode &1? readNT_H:  readNT_V);
			SetPPUWriteHandler    (bank, mode &1? writeNT_H: writeNT_V);
		}
		SetCPUWriteHandler(0x3, cpuWrite_mode);
		SetCPUReadHandler(0x4, cpuRead_printer);
		Controllers::SetDefaultInput(INPUT_BIT_79);
		CPU::callbacks.clear();
	}
}

BOOL	MAPINT	load (void) {
	loadBIOS (_T("BIOS\\BIT79.BIN"), bios, sizeof(bios));
	Description =_T("Bit Corp. Bit-79");
	
	if (adMI->Load) adMI->Load();	
	if (adMI->CPUCycle) MI->CPUCycle =adMI->CPUCycle;
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	
	printerData.clear();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	adMI->Reset(resetType);
	if (resetType ==RESET_HARD) mode =0;
	sync();
}

void	MAPINT	unload (void) {
	flushPrinterData();
	if (adMI->Unload) adMI->Unload();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	if (mode &2) for (auto& c: ram) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t deviceNum =ID_PLUG_BIT79;
} // namespace Bit79

MapperInfo plugThruDevice_Bit79 ={
	&Bit79::deviceNum,
	_T("Bit Corp. Bit-79"),
	COMPAT_FULL,
	Bit79::load,
	Bit79::reset,
	Bit79::unload,
	NULL,
	NULL,
	Bit79::saveLoad,
	NULL,
	NULL
};
} // namespace PlugThruDevice