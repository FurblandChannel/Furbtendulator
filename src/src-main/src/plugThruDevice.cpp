/*	The plug-through device sits between the console and the cartridge.
	Similarly, the PlugThruDevice sits between the emulator core and the mapper DLL.
	It substitutes the Read/WriteHandler function in the EmulatorInterface and substitutes its own, redirecting reads/writes as needed.
	The ReadCart/WriteCart handlers are not redirected. This means that the PlugThruDevice must not use them to redirect any additional memory; instead, any such redirection must be performed by the Read/Write handlers.
	The current implementation only allows for one plug-through device at a time. So, you cannot use a Game Genie with a Bung Doctor PC Junior, for example.
*/

#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "NES.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_StudyBox.hpp"
#include "plugThruDevice_SuperMagicCard_transfer.hpp"

namespace PlugThruDevice {
// Read/write handlers of attached devices
FCPURead	adCPURead[32];
FCPURead	adCPUReadDebug[32];
FCPUWrite	adCPUWrite[32];
FPPURead	adPPURead[32];
FPPURead	adPPUReadDebug[32];
FPPUWrite	adPPUWrite[32];
MapperInfo	*adMI =NULL;
TCHAR		*Description;
bool		initialized =false;
bool		button =false;
int		buttonCount =0;
uint8_t		pdIRQ, adIRQ;

FCPURead	(MAPINT	*GetCPUReadHandler	) (int);
FCPURead	(MAPINT	*GetCPUReadHandlerDebug ) (int);
FCPUWrite	(MAPINT	*GetCPUWriteHandler	) (int);
FPPURead	(MAPINT	*GetPPUReadHandler	) (int);
FPPURead	(MAPINT	*GetPPUReadHandlerDebug ) (int);
FPPUWrite	(MAPINT	*GetPPUWriteHandler	) (int);
void		(MAPINT	*SetCPUReadHandler	) (int, FCPURead);
void		(MAPINT	*SetCPUReadHandlerDebug ) (int, FCPURead);
void		(MAPINT	*SetCPUWriteHandler	) (int, FCPUWrite);
void		(MAPINT	*SetPPUReadHandler	) (int, FPPURead);
void		(MAPINT	*SetPPUReadHandlerDebug ) (int, FPPURead);
void		(MAPINT	*SetPPUWriteHandler	) (int, FPPUWrite);
void		(MAPINT *SetIRQ                 ) (int);

FCPURead	MAPINT	adGetCPUReadHandler	(int bank)	{ return adCPURead[bank];               }
FCPURead	MAPINT	adGetCPUReadHandlerDebug(int bank)	{ return adCPUReadDebug[bank];          }
FCPUWrite	MAPINT	adGetCPUWriteHandler	(int bank)	{ return adCPUWrite[bank];              }
FPPURead	MAPINT	adGetPPUReadHandler	(int bank)	{ return adPPURead[bank];               }
FPPURead	MAPINT	adGetPPUReadHandlerDebug(int bank)	{ return adPPURead[bank];               }
FPPUWrite	MAPINT	adGetPPUWriteHandler	(int bank)	{ return adPPUWrite[bank];              }
void		MAPINT	adSetIRQ		(int state)	{ adIRQ =state; SetIRQ(pdIRQ &adIRQ); }
void		MAPINT	pdSetIRQ		(int state)	{ pdIRQ =state; SetIRQ(pdIRQ &adIRQ);}
void	MAPINT	adSetCPUReadHandler	(int bank, FCPURead handler)		{ adCPURead[bank]      =handler;}
void	MAPINT	adSetCPUReadHandlerDebug(int bank, FCPURead handler)		{ adCPUReadDebug[bank] =handler;}
void	MAPINT	adSetCPUWriteHandler	(int bank, FCPUWrite handler)		{ adCPUWrite[bank]     =handler;}
void	MAPINT	adSetPPUReadHandler	(int bank, FPPURead handler)		{ adPPURead[bank]      =handler;}
void	MAPINT	adSetPPUReadHandlerDebug(int bank, FPPURead handler)		{ adPPUReadDebug[bank] =handler;}
void	MAPINT	adSetPPUWriteHandler	(int bank, FPPUWrite handler)		{ adPPUWrite[bank]     =handler;}

void	initPlugThruDevice (void) {
	if (RI.ROMType ==ROM_NSF) return;
	MapperInfo *device =NULL;
	EnableMenuItem(hMenu, ID_GAME_BUTTON, MF_GRAYED);
	switch (NES::currentPlugThruDevice) {
		case ID_PLUG_GAME_GENIE:        device =&plugThruDevice_GameGenie; break;
		case ID_PLUG_PRO_ACTION_REPLAY:	device =&plugThruDevice_ProActionReplay; break;
		case ID_PLUG_BUNG_GD1M:         device =&plugThruDevice_GameDoctor1M; break;
		case ID_PLUG_BUNG_SGD2M:        device =&plugThruDevice_SuperGameDoctor2M; break;
		case ID_PLUG_BUNG_SGD4M:        device =&plugThruDevice_SuperGameDoctor4M; break;
		case ID_PLUG_BUNG_GM19:         device =&plugThruDevice_GameMaster19; break;
		case ID_PLUG_BUNG_GM20:         device =&plugThruDevice_GameMaster20; break;
		case ID_PLUG_BUNG_GMK:          device =&plugThruDevice_GameMasterKid; break;
		case ID_PLUG_BUNG_GMB:          device =&plugThruDevice_MasterBoy; break;
		case ID_PLUG_QJ_GAR:            device =&plugThruDevice_GameActionReplay; break;
		case ID_PLUG_VENUS_MD:          device =&plugThruDevice_MiniDoctor; break;
		case ID_PLUG_VENUS_GC1M:        device =&plugThruDevice_GameConverter1M; break;
		case ID_PLUG_VENUS_GC2M:        device =&plugThruDevice_GameConverter2M; break;
		case ID_PLUG_VENUS_TGD_4P:      device =&plugThruDevice_TurboGameDoctor4P; break;
		case ID_PLUG_VENUS_TGD_6P:      device =&plugThruDevice_TurboGameDoctor6P; break;
		case ID_PLUG_VENUS_TGD_6M:      device =&plugThruDevice_TurboGameDoctor6M; break;
		case ID_PLUG_FFE_SMC:           device =&plugThruDevice_SuperMagicCard; break;
		case ID_PLUG_TC_MC2T:           device =&plugThruDevice_MagicardIITurbo; break;
		case ID_PLUG_SOUSEIKI_FAMMY:    device =&plugThruDevice_SousekiFammy; break;
		case ID_PLUG_BIT79:             device =&plugThruDevice_Bit79; break;
		case ID_PLUG_STUDYBOX:          device =&plugThruDevice_StudyBox; break;
		case ID_PLUG_DRPCJR:            device =&plugThruDevice_DoctorPCJr; break;
	}
	if (device) {
		adMI =MI;
		MI =device;
		MI->MapperId =adMI->MapperId;
		initialized =false;
		button =false;
	}
}

void	uninitPlugThruDevice (void) {
	if (adMI) MI =adMI;
	adMI =NULL;
	uninitHandlers();
}

void	initHandlers (void) {
	if (initialized ==true) uninitHandlers();
	if (adMI ==NULL) return;
	GetCPUReadHandler      =*EI.GetCPUReadHandler;
	GetCPUReadHandlerDebug =*EI.GetCPUReadHandlerDebug;
	GetCPUWriteHandler     =*EI.GetCPUWriteHandler;
	GetPPUReadHandler      =*EI.GetPPUReadHandler;
	GetPPUReadHandlerDebug =*EI.GetPPUReadHandlerDebug;
	GetPPUWriteHandler     =*EI.GetPPUWriteHandler;
	SetCPUReadHandler      =*EI.SetCPUReadHandler;
	SetCPUReadHandlerDebug =*EI.SetCPUReadHandlerDebug;
	SetCPUWriteHandler     =*EI.SetCPUWriteHandler;
	SetPPUReadHandler      =*EI.SetPPUReadHandler;
	SetPPUReadHandlerDebug =*EI.SetPPUReadHandlerDebug;
	SetPPUWriteHandler     =*EI.SetPPUWriteHandler;
	SetIRQ                 =*EI.SetIRQ;
	EI.GetCPUReadHandler      =&adGetCPUReadHandler;
	EI.GetCPUReadHandlerDebug =&adGetCPUReadHandlerDebug;
	EI.GetCPUWriteHandler     =&adGetCPUWriteHandler;
	EI.GetPPUReadHandler      =&adGetPPUReadHandler;
	EI.GetPPUReadHandlerDebug =&adGetPPUReadHandlerDebug;
	EI.GetPPUWriteHandler     =&adGetPPUWriteHandler;
	EI.SetCPUReadHandler      =&adSetCPUReadHandler;
	EI.SetCPUReadHandlerDebug =&adSetCPUReadHandlerDebug;
	EI.SetCPUWriteHandler     =&adSetCPUWriteHandler;
	EI.SetPPUReadHandler      =&adSetPPUReadHandler;
	EI.SetPPUReadHandlerDebug =&adSetPPUReadHandlerDebug;
	EI.SetPPUWriteHandler     =&adSetPPUWriteHandler;
	EI.SetIRQ                 =&adSetIRQ;
	for (int bank =0x00; bank <0x20; bank++) {
		if (bank >=0x10 && CPU::CPU[1] ==NULL) break;
		int ppuBank =bank &0xF | bank <<2 &0x40;
		adCPURead[bank]      =GetCPUReadHandler(bank);
		adCPUReadDebug[bank] =GetCPUReadHandlerDebug(bank);
		adCPUWrite[bank]     =GetCPUWriteHandler(bank);
		adPPURead[bank]      =GetPPUReadHandler(ppuBank);
		adPPUReadDebug[bank] =GetPPUReadHandlerDebug(ppuBank);
		adPPUWrite[bank]     =GetPPUWriteHandler(ppuBank);
		SetCPUReadHandler     (bank, passCPURead);
		SetCPUReadHandlerDebug(bank, passCPUReadDebug);
		SetCPUWriteHandler    (bank, passCPUWrite);
		SetPPUReadHandler     (ppuBank, passPPURead);
		SetPPUReadHandlerDebug(ppuBank, passPPUReadDebug);
		SetPPUWriteHandler    (ppuBank, passPPUWrite);
	}
	initialized =true;
	pdIRQ =adIRQ =1;
}

void	uninitHandlers (void) {
	if (initialized) {
		EI.GetCPUReadHandler      =*GetCPUReadHandler;
		EI.GetCPUReadHandlerDebug =*GetCPUReadHandlerDebug;
		EI.GetCPUWriteHandler     =*GetCPUWriteHandler;
		EI.GetPPUReadHandler      =*GetPPUReadHandler;
		EI.GetPPUReadHandlerDebug =*GetPPUReadHandlerDebug;
		EI.GetPPUWriteHandler     =*GetPPUWriteHandler;
		EI.SetCPUReadHandler      =*SetCPUReadHandler;
		EI.SetCPUReadHandlerDebug =*SetCPUReadHandlerDebug;
		EI.SetCPUWriteHandler     =*SetCPUWriteHandler;
		EI.SetPPUReadHandler      =*SetPPUReadHandler;
		EI.SetPPUReadHandlerDebug =*SetPPUReadHandlerDebug;
		EI.SetPPUWriteHandler     =*SetPPUWriteHandler;
		EI.SetIRQ                 =*SetIRQ;
		for (int bank =0; bank <32; bank++) {
			if (bank >=16 && CPU::CPU[1] ==NULL) break;
			int ppuBank =bank &0xF | bank <<2 &0x40;
			EI.SetCPUReadHandler     (bank, adCPURead[bank]);
			EI.SetCPUReadHandlerDebug(bank, adCPUReadDebug[bank]);
			EI.SetCPUWriteHandler    (bank, adCPUWrite[bank]);
			EI.SetPPUReadHandler     (ppuBank, adPPURead[bank]);
			EI.SetPPUReadHandlerDebug(ppuBank, adPPUReadDebug[bank]);
			EI.SetPPUWriteHandler    (ppuBank, adPPUWrite[bank]);
		}
		initialized =false;
	}
}

int	MAPINT	passCPURead (int bank, int addr) {
	return adCPURead[bank](bank, addr);
}
int	MAPINT	passCPUReadDebug (int bank, int addr) {
	return adCPUReadDebug[bank](bank, addr);
}
void	MAPINT	passCPUWrite (int bank, int addr, int val) {
	adCPUWrite[bank](bank, addr, val);
}
void	MAPINT	ignoreWrite (int bank, int addr, int val) {
}
int	MAPINT	passPPURead (int bank, int addr) {
	return adPPURead[bank &0xF | bank >>2 &0x10](bank, addr);
}
int	MAPINT	passPPUReadDebug (int bank, int addr) {
	return adPPUReadDebug[bank &0xF | bank >>2 &0x10](bank, addr);
}
void	MAPINT	passPPUWrite (int bank, int addr, int val) {
	if (bank <7 || addr <0xF00)
		adPPUWrite[bank &0xF | bank >>2 &0x10](bank, addr, val);
}

bool	loadBIOS (TCHAR *name, uint8_t *data, size_t size) {
	bool result =false;
		TCHAR tmp[MAX_PATH+64];
	
	TCHAR buf[MAX_PATH];
	int i =GetModuleFileName(NULL, buf, MAX_PATH);
	if (i) {
		while (i > 0)
			if (buf[--i] =='\\')
				break;
		buf[i++] ='\\';
		buf[i++] =0;
		_tcscat(buf, name);
		
		FILE *Handle =_tfopen(buf, _T("rb"));
		if (Handle) {
			if (fread(data, 1, size, Handle) ==size)
				result =true;
			else {
				_stprintf(tmp, _T("File %s does not have size %i."), buf, size);
				MessageBox(hMainWnd, tmp, _T("Plug-through device"), MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL);
			}
			fclose(Handle);
		} else {
			_stprintf(tmp, _T("File %s not found."), buf);
			MessageBox(hMainWnd, tmp, _T("Plug-through device"), MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL);
		}
	} else 
		MessageBox(hMainWnd, _T("Cannot determine BIOS path."), _T("Plug-through device"), MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL);
	return result;
}

int	save (FILE *out) {
	int clen =0;
	writeLong(NES::currentPlugThruDevice);
	return clen;
}
int	load (FILE *in, int version_id) {
	int clen =0;
	int newDevice;
	readLong(newDevice);
	if (newDevice !=NES::currentPlugThruDevice) {
		uninitPlugThruDevice();
		// Create/destroy second CPU for Bung Master Link
		if (needSecondCPU(NES::currentPlugThruDevice) && !needSecondCPU(newDevice)) {
			if (CPU::CPU[1]) delete CPU::CPU[1];
			if (PPU::PPU[1]) delete PPU::PPU[1];
			CPU::CPU[1] =NULL;
			PPU::PPU[1] =NULL;
		} else
		if (!needSecondCPU(NES::currentPlugThruDevice) && needSecondCPU(newDevice)) {
			CPU::CPU[1] =new CPU::CPU_RP2A03(1, 2048, NES::CPU_RAM +2048);
			PPU::PPU[1] =new PPU::PPU_Dummy(1);
		}
		NES::currentPlugThruDevice =newDevice;
		initPlugThruDevice();
		Settings::ApplySettingsToMenu();		
		
		Description =_T("None");
		if (MI->Load) MI->Load();
		NES::Reset(RESET_HARD);
		TCHAR tmp[256];
		_stprintf(tmp, _T("Loading this state changed the plug-through device to:\n%s"), Description);
		MessageBox(hMainWnd, tmp, _T("Nintendulator"), MB_OK | MB_ICONEXCLAMATION);
	}
	
	return clen;
}

void	pressButton (void) {
	button =true;
	buttonCount =1789000;
	EI.DbgOut(L"Button pressed");
}
void	releaseButton (void) {
	button =false;
	EI.DbgOut(L"Button released");
}
void	checkButton (void) {
	if (buttonCount && !--buttonCount) releaseButton();
}

bool	needSecondCPU (int device) {
	if ((device ==ID_PLUG_BUNG_GM19 || device ==ID_PLUG_BUNG_GM20) && RI.ROMType !=ROM_FDS && RI.INES_MapperNum !=761 && !PlugThruDevice::SuperMagicCard::haveFileToSend)
		return true;
	else
		return false;
}
}; // namespace