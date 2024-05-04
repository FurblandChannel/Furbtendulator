#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "NES.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "GFX.h"
#include "Debugger.h"
#include "Controllers.h"
#include "plugThruDevice.hpp"

EmulatorInterface EI;
ROMInfo RI;

DLLInfo *DI;
MapperInfo *MI;
MapperInfo *MI2;

namespace MapperInterface {
void	MAPINT	noCartInserted_reset (RESET_TYPE ResetType) {
}

MapperInfo noCartInserted ={
	_T(""),
	_T("No cart inserted"),
	COMPAT_FULL,
	NULL,
	noCartInserted_reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

struct	MapperDLL
{
	HINSTANCE	dInst;
	TCHAR		filename[MAX_PATH];
	FLoadMapperDLL	LoadDLL;
	FUnloadMapperDLL	UnloadDLL;
	DLLInfo		*DI;
	MapperDLL	*Next;
} *MapperDLLs = NULL;

/******************************************************************************/

void	MAPINT	SetCPUReadHandlerDebug (int Page, FCPURead New) {	
	CPU::CPU[Page >>4]->ReadHandlerDebug[Page &0xF] = New;
}

FCPURead	MAPINT	GetCPUReadHandlerDebug (int Page) {
	return CPU::CPU[Page >>4]->ReadHandlerDebug[Page &0xF];
}

void	MAPINT	SetCPUReadHandler (int Page, FCPURead New) {
	CPU::CPU[Page >>4]->ReadHandler[Page &0xF] = New;
	if (New == CPU::ReadRAM || New == CPU::ReadPRG) {
		if (New == CPU::ReadRAM || New == CPU::ReadPRG)
			SetCPUReadHandlerDebug(Page, New);
		else	
			SetCPUReadHandlerDebug(Page, CPU::ReadUnsafe);
	}
}

FCPURead	MAPINT	GetCPUReadHandler (int Page) {
	return CPU::CPU[Page >>4]->ReadHandler[Page &0xF];
}

void	MAPINT	SetCPUWriteHandler (int Page, FCPUWrite New) {
	CPU::CPU[Page >>4]->WriteHandler[Page &0xF] = New;
}

FCPUWrite	MAPINT	GetCPUWriteHandler (int Page) {
	return CPU::CPU[Page >>4]->WriteHandler[Page &0xF];
}

void	MAPINT	SetPPUReadHandlerDebug (int Page, FPPURead New) {
	PPU::PPU[Page >>6]->ReadHandlerDebug[Page &0x3F] = New;
}

FPPURead	MAPINT	GetPPUReadHandlerDebug (int Page) {
	return PPU::PPU[Page >>6]->ReadHandlerDebug[Page &0x3F];
}

void	MAPINT	SetPPUReadHandler (int Page, FPPURead New) {
	PPU::PPU[Page >>6]->ReadHandler[Page &0x3F] = New;
	if (New == PPU::BusRead)
		SetPPUReadHandlerDebug(Page, New);
	else	
		SetPPUReadHandlerDebug(Page, PPU::ReadUnsafe);
}
FPPURead	MAPINT	GetPPUReadHandler (int Page)
{
	return PPU::PPU[Page >>6]->ReadHandler[Page &0x3F];
}

void	MAPINT	SetPPUWriteHandler (int Page, FPPUWrite New)
{
	PPU::PPU[Page >>6]->WriteHandler[Page &0x3F] = New;
}
FPPUWrite	MAPINT	GetPPUWriteHandler (int Page)
{
	return PPU::PPU[Page >>6]->WriteHandler[Page &0x3F];
}

/******************************************************************************/
void	MAPINT	SetPRGROMSize (int newSize) {
	RI.PRGROMSize =newSize;
	NES::PRGSizeROM =newSize /0x1000;
	NES::PRGMaskROM = NES::getMask(NES::PRGSizeROM - 1) & MAX_PRGROM_MASK;
}

void	MAPINT	SetPRGRAMSize (int newSize) {
	RI.PRGRAMSize =newSize;
	NES::PRGSizeRAM =newSize /0x1000;
	NES::PRGMaskRAM = NES::getMask(NES::PRGSizeRAM - 1) & MAX_PRGRAM_MASK;
}

__inline void	MAPINT	SetPRG_ROM4 (int Bank, int Val) {
	CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] = NES::PRG_ROM[Val &NES::PRGMaskROM];
	if (!NES::PRGSizeROM)
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = FALSE;
	else
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = TRUE;
	CPU::CPU[Bank >>4]->Writable[Bank &0xF] = FALSE;
}
void	MAPINT	SetPRG_ROM8 (int Bank, int Val)
{
	Val <<= 1;
	SetPRG_ROM4(Bank+0, Val+0);
	SetPRG_ROM4(Bank+1, Val+1);
}
void	MAPINT	SetPRG_ROM16 (int Bank, int Val)
{
	Val <<= 2;
	SetPRG_ROM4(Bank+0, Val+0);
	SetPRG_ROM4(Bank+1, Val+1);
	SetPRG_ROM4(Bank+2, Val+2);
	SetPRG_ROM4(Bank+3, Val+3);
}
void	MAPINT	SetPRG_ROM32 (int Bank, int Val)
{
	Val <<= 3;
	SetPRG_ROM4(Bank+0, Val+0);
	SetPRG_ROM4(Bank+1, Val+1);
	SetPRG_ROM4(Bank+2, Val+2);
	SetPRG_ROM4(Bank+3, Val+3);
	SetPRG_ROM4(Bank+4, Val+4);
	SetPRG_ROM4(Bank+5, Val+5);
	SetPRG_ROM4(Bank+6, Val+6);
	SetPRG_ROM4(Bank+7, Val+7);
}
int	MAPINT	GetPRG_ROM4 (int Bank)	// -1 if no ROM mapped
{
	int tpi = (int)((CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] - NES::PRG_ROM[0]) >> 12);
	if ((tpi >= 0) && (tpi < NES::PRGSizeROM))
		return tpi;
	else	return -1;
}

/******************************************************************************/

__inline void	MAPINT	SetPRG_RAM4 (int Bank, int Val)
{
	CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] = NES::PRG_RAM[Val & NES::PRGMaskRAM];
	if (!NES::PRGSizeRAM) {
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = FALSE;
		CPU::CPU[Bank >>4]->Writable[Bank &0xF] = FALSE;
	} else {
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = TRUE;
		CPU::CPU[Bank >>4]->Writable[Bank &0xF] = TRUE;
	}
}
void	MAPINT	SetPRG_RAM8 (int Bank, int Val)
{
	Val <<= 1;
	SetPRG_RAM4(Bank+0, Val+0);
	SetPRG_RAM4(Bank+1, Val+1);
}
void	MAPINT	SetPRG_RAM16 (int Bank, int Val)
{
	Val <<= 2;
	SetPRG_RAM4(Bank+0, Val+0);
	SetPRG_RAM4(Bank+1, Val+1);
	SetPRG_RAM4(Bank+2, Val+2);
	SetPRG_RAM4(Bank+3, Val+3);
}
void	MAPINT	SetPRG_RAM32 (int Bank, int Val)
{
	Val <<= 3;
	SetPRG_RAM4(Bank+0, Val+0);
	SetPRG_RAM4(Bank+1, Val+1);
	SetPRG_RAM4(Bank+2, Val+2);
	SetPRG_RAM4(Bank+3, Val+3);
	SetPRG_RAM4(Bank+4, Val+4);
	SetPRG_RAM4(Bank+5, Val+5);
	SetPRG_RAM4(Bank+6, Val+6);
	SetPRG_RAM4(Bank+7, Val+7);
}
int	MAPINT	GetPRG_RAM4 (int Bank)	// -1 if no RAM mapped
{
	int tpi = (int)((CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] - NES::PRG_RAM[0]) >> 12);
	if ((tpi >= 0) && (tpi < NES::PRGSizeRAM))
		return tpi;
	else	return -1;
}

/******************************************************************************/

unsigned char *	MAPINT	GetPRG_Ptr4 (int Bank)
{
	return CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF];
}
void	MAPINT	SetPRG_Ptr4 (int Bank, unsigned char *Data, BOOL Writable) {
	CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] = Data;
	if (Data >= &NES::PRG_RAM[0][0] && Data <&NES::PRG_RAM[0][0] +sizeof(NES::PRG_RAM) && NES::PRGSizeRAM ==0) {
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = FALSE;
		CPU::CPU[Bank >>4]->Writable[Bank &0xF] = FALSE;
	} else {
		CPU::CPU[Bank >>4]->Readable[Bank &0xF] = TRUE;
		CPU::CPU[Bank >>4]->Writable[Bank &0xF] = Writable;
	}
}
void	MAPINT	SetPRG_OB4 (int Bank)	// Open bus
{
	CPU::CPU[Bank >>4]->PRGPointer[Bank &0xF] = NES::PRG_RAM[0];
	CPU::CPU[Bank >>4]->Readable[Bank &0xF] = FALSE;
	CPU::CPU[Bank >>4]->Writable[Bank &0xF] = FALSE;
}

/******************************************************************************/

void	MAPINT	SetCHRROMSize (int newSize) {
	RI.CHRROMSize =newSize;
	NES::CHRSizeROM =newSize /0x0400;
	NES::CHRMaskROM = NES::getMask(NES::CHRSizeROM - 1) & MAX_CHRROM_MASK;
}

void	MAPINT	SetCHRRAMSize (int newSize) {
	RI.CHRRAMSize =newSize;
	NES::CHRSizeRAM =newSize /0x0400;
	NES::CHRMaskRAM = NES::getMask(NES::CHRSizeRAM - 1) & MAX_CHRRAM_MASK;
}

__inline void	MAPINT	SetCHR_ROM1 (int Bank, int Val)
{
	if (!NES::CHRSizeROM)
		return;
	PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] = NES::CHR_ROM[Val & NES::CHRMaskROM];
	PPU::PPU[Bank >>6]->Writable[Bank &0x3F] = FALSE;
#ifdef	ENABLE_DEBUGGER
	if (Bank & 8)
		Debugger::NTabChanged = TRUE;
	else	
		Debugger::PatChanged = TRUE;
#endif	/* ENABLE_DEBUGGER */
}
void	MAPINT	SetCHR_ROM2 (int Bank, int Val)
{
	Val <<= 1;
	SetCHR_ROM1(Bank+0, Val+0);
	SetCHR_ROM1(Bank+1, Val+1);
}
void	MAPINT	SetCHR_ROM4 (int Bank, int Val)
{
	Val <<= 2;
	SetCHR_ROM1(Bank+0, Val+0);
	SetCHR_ROM1(Bank+1, Val+1);
	SetCHR_ROM1(Bank+2, Val+2);
	SetCHR_ROM1(Bank+3, Val+3);
}
void	MAPINT	SetCHR_ROM8 (int Bank, int Val)
{
	Val <<= 3;
	SetCHR_ROM1(Bank+0, Val+0);
	SetCHR_ROM1(Bank+1, Val+1);
	SetCHR_ROM1(Bank+2, Val+2);
	SetCHR_ROM1(Bank+3, Val+3);
	SetCHR_ROM1(Bank+4, Val+4);
	SetCHR_ROM1(Bank+5, Val+5);
	SetCHR_ROM1(Bank+6, Val+6);
	SetCHR_ROM1(Bank+7, Val+7);
}
int	MAPINT	GetCHR_ROM1 (int Bank)	// -1 if no ROM mapped
{
	int tpi = (int)((PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] - NES::CHR_ROM[0]) >> 10);
	if ((tpi >= 0) && (tpi < NES::CHRSizeROM))
		return tpi;
	else	return -1;
}

/******************************************************************************/

__inline void	MAPINT	SetCHR_RAM1 (int Bank, int Val)
{
	if (!NES::CHRSizeRAM)
		return;
	PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] = NES::CHR_RAM[Val & NES::CHRMaskRAM];
	PPU::PPU[Bank >>6]->Writable[Bank &0x3F] = TRUE;
#ifdef	ENABLE_DEBUGGER
	if (Bank & 8)
		Debugger::NTabChanged = TRUE;
	else	
		Debugger::PatChanged = TRUE;
#endif	/* ENABLE_DEBUGGER */
}
void	MAPINT	SetCHR_RAM2 (int Bank, int Val)
{
	Val <<= 1;
	SetCHR_RAM1(Bank+0, Val+0);
	SetCHR_RAM1(Bank+1, Val+1);
}
void	MAPINT	SetCHR_RAM4 (int Bank, int Val)
{
	Val <<= 2;
	SetCHR_RAM1(Bank+0, Val+0);
	SetCHR_RAM1(Bank+1, Val+1);
	SetCHR_RAM1(Bank+2, Val+2);
	SetCHR_RAM1(Bank+3, Val+3);
}
void	MAPINT	SetCHR_RAM8 (int Bank, int Val)
{
	Val <<= 3;
	SetCHR_RAM1(Bank+0, Val+0);
	SetCHR_RAM1(Bank+1, Val+1);
	SetCHR_RAM1(Bank+2, Val+2);
	SetCHR_RAM1(Bank+3, Val+3);
	SetCHR_RAM1(Bank+4, Val+4);
	SetCHR_RAM1(Bank+5, Val+5);
	SetCHR_RAM1(Bank+6, Val+6);
	SetCHR_RAM1(Bank+7, Val+7);
}
int	MAPINT	GetCHR_RAM1 (int Bank)	// -1 if no ROM mapped
{
	int tpi = (int)((PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] - NES::CHR_RAM[0]) >> 10);
	if ((tpi >= 0) && (tpi < NES::CHRSizeRAM))
		return tpi;
	else	return -1;
}

/******************************************************************************/

__inline void	MAPINT	SetCHR_NT1 (int Bank, int Val)
{
	PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] = PPU::PPU[Bank >>6]->VRAM[Val & 3];
	PPU::PPU[Bank >>6]->Writable[Bank &0x3F] = TRUE;
#ifdef	ENABLE_DEBUGGER
	if (Bank & 8)
		Debugger::NTabChanged = TRUE;
	else	
		Debugger::PatChanged = TRUE;
#endif	/* ENABLE_DEBUGGER */
}
int	MAPINT	GetCHR_NT1 (int Bank)	// -1 if no ROM mapped
{
	int tpi = (int)((PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] - PPU::PPU[Bank >>6]->VRAM[0]) >> 10);
	if ((tpi < 0) || (tpi > 4)) return -1;
	else return tpi;
}

/******************************************************************************/

unsigned char *	MAPINT	GetCHR_Ptr1 (int Bank)
{
	return PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F];
}

void	MAPINT	SetCHR_Ptr1 (int Bank, unsigned char *Data, BOOL Writable)
{
	PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] = Data;
	PPU::PPU[Bank >>6]->Writable[Bank &0x3F] = Writable;
#ifdef	ENABLE_DEBUGGER
	if (Bank & 8)
		Debugger::NTabChanged = TRUE;
	else	
		Debugger::PatChanged = TRUE;
#endif	/* ENABLE_DEBUGGER */
}

void	MAPINT	SetCHR_OB1 (int Bank)
{
	PPU::PPU[Bank >>6]->CHRPointer[Bank &0x3F] = PPU::OpenBus;
	PPU::PPU[Bank >>6]->Writable[Bank &0x3F] = FALSE;
#ifdef	ENABLE_DEBUGGER
	if (Bank & 8)
		Debugger::NTabChanged = TRUE;
	else	
		Debugger::PatChanged = TRUE;
#endif	/* ENABLE_DEBUGGER */
}

/******************************************************************************/

void	MAPINT	Mirror_H (void)
{
	SetCHR_NT1(0x8, 0);	SetCHR_NT1(0xC, 0);
	SetCHR_NT1(0x9, 0);	SetCHR_NT1(0xD, 0);
	SetCHR_NT1(0xA, 1);	SetCHR_NT1(0xE, 1);
	SetCHR_NT1(0xB, 1);	SetCHR_NT1(0xF, 1);
}
void	MAPINT	Mirror_V (void)
{
	SetCHR_NT1(0x8, 0);	SetCHR_NT1(0xC, 0);
	SetCHR_NT1(0x9, 1);	SetCHR_NT1(0xD, 1);
	SetCHR_NT1(0xA, 0);	SetCHR_NT1(0xE, 0);
	SetCHR_NT1(0xB, 1);	SetCHR_NT1(0xF, 1);
}
void	MAPINT	Mirror_4 (void)
{
	SetCHR_NT1(0x8, 0);	SetCHR_NT1(0xC, 0);
	SetCHR_NT1(0x9, 1);	SetCHR_NT1(0xD, 1);
	SetCHR_NT1(0xA, 2);	SetCHR_NT1(0xE, 2);
	SetCHR_NT1(0xB, 3);	SetCHR_NT1(0xF, 3);
	if (RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL) {
		SetCHR_NT1(0x48, 0);	SetCHR_NT1(0x4C, 0);
		SetCHR_NT1(0x49, 1);	SetCHR_NT1(0x4D, 1);
		SetCHR_NT1(0x4A, 2);	SetCHR_NT1(0x4E, 2);
		SetCHR_NT1(0x4B, 3);	SetCHR_NT1(0x4F, 3);
	}
}
void	MAPINT	Mirror_S0 (void)
{
	SetCHR_NT1(0x8, 0);	SetCHR_NT1(0xC, 0);
	SetCHR_NT1(0x9, 0);	SetCHR_NT1(0xD, 0);
	SetCHR_NT1(0xA, 0);	SetCHR_NT1(0xE, 0);
	SetCHR_NT1(0xB, 0);	SetCHR_NT1(0xF, 0);
}
void	MAPINT	Mirror_S1 (void)
{
	SetCHR_NT1(0x8, 1);	SetCHR_NT1(0xC, 1);
	SetCHR_NT1(0x9, 1);	SetCHR_NT1(0xD, 1);
	SetCHR_NT1(0xA, 1);	SetCHR_NT1(0xE, 1);
	SetCHR_NT1(0xB, 1);	SetCHR_NT1(0xF, 1);
}
void	MAPINT	Mirror_Custom (int M1, int M2, int M3, int M4)
{
	SetCHR_NT1(0x8, M1);	SetCHR_NT1(0xC, M1);
	SetCHR_NT1(0x9, M2);	SetCHR_NT1(0xD, M2);
	SetCHR_NT1(0xA, M3);	SetCHR_NT1(0xE, M3);
	SetCHR_NT1(0xB, M4);	SetCHR_NT1(0xF, M4);
}

/******************************************************************************/

void	MAPINT	Set_SRAMSize (int Size)	// Sets the size of the SRAM (in bytes) and clears PRG_RAM
{
	NES::SRAM_Size = Size;
	int SizeIn4K = (Size / 0x1000) + ((Size % 0x1000) ? 1 : 0);
	if (SizeIn4K >NES::PRGSizeRAM) {
		NES::PRGSizeRAM =SizeIn4K;
		NES::PRGMaskRAM =NES::getMask(NES::PRGSizeRAM -1) &MAX_PRGRAM_MASK;
	}
	if (Size <1024) // EEPROM always zeroed out
		ZeroMemory(NES::PRG_RAM, Size);
	else
		CPU::RandomMemory(NES::PRG_RAM, Size);
}

/******************************************************************************/

void	MAPINT	SetIRQ2 (int which, int IRQstate) {
	if (IRQstate)
		CPU::CPU[which]->WantIRQ &= ~IRQ_EXTERNAL;
	else	CPU::CPU[which]->WantIRQ |= IRQ_EXTERNAL;
}

void	MAPINT	SetIRQ (int IRQstate) {
	//if (IRQstate ==0 && ~CPU::CPU[0]->WantIRQ &IRQ_EXTERNAL) EI.DbgOut(L"IRQ at SL %d, cycle %d", PPU::PPU[0]->SLnum, PPU::PPU[0]->Clockticks);
	SetIRQ2 (0, IRQstate);
}

void	MAPINT	DbgOut (const TCHAR *text, ...)
{
	TCHAR txt[1024];
	va_list marker;
	va_start(marker, text);
	_vstprintf(txt, text, marker);
	va_end(marker);
	AddDebug(txt);
}

void	MAPINT	StatusOut (const TCHAR *text, ...)
{
	TCHAR txt[1024];
	va_list marker;
	va_start(marker, text);
	_vstprintf(txt, text, marker);
	va_end(marker);
	PrintTitlebar(txt);
}

void	MAPINT	SetPC (int NewPC) {
	NES::NewPC =NewPC;
}

void	MAPINT	ForceReset (RESET_TYPE ResetType) {
	NES::Reset(ResetType);
}


void	MAPINT RegisterWindow (HWND handle) {
	hWindows.insert(handle);
}

void	MAPINT UnregisterWindow (HWND handle) {
	hWindows.erase(handle);
}

void	MAPINT StopCPU (void) {
	NES::DoStop |=STOPMODE_NOW;
}


/******************************************************************************/

const TCHAR *CompatLevel[COMPAT_NUMTYPES] = {_T("Unsupported"), _T(" [Partially supported]"), _T(" [Mostly supported]"), _T("")};
static uint8_t DummyOpenBus =0xFF;
BOOL		multiCanSave;
uint8_t*	multiPRGStart;
uint8_t*	multiCHRStart;
size_t		multiPRGSize;
size_t		multiCHRSize;
uint16_t	multiMapper;
uint8_t		multiSubmapper;
uint8_t		multiMirroring;


void	Init (void) {
	WIN32_FIND_DATA Data;
	HANDLE Handle;
	MapperDLL *ThisDLL;
	TCHAR Filename[MAX_PATH], Path[MAX_PATH];
	_tcscpy(Path, ProgPath);
	_tcscat(Path, _T("Mappers\\"));
	_stprintf(Filename, _T("%s%s"), Path, _T("*.dll"));
	Handle = FindFirstFile(Filename, &Data);
	ThisDLL = new MapperDLL;
	if (Handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			TCHAR Tmp[MAX_PATH];
			_tcscpy(ThisDLL->filename, Data.cFileName);
			_stprintf(Tmp, _T("%s%s"), Path, ThisDLL->filename);
			ThisDLL->dInst = LoadLibrary(Tmp);
			if (!ThisDLL->dInst)
			{
				DbgOut(_T("Failed to load %s - not a valid DLL file! LastError %d"), Data.cFileName, GetLastError());
				continue;
			}
			ThisDLL->LoadDLL = (FLoadMapperDLL)GetProcAddress(ThisDLL->dInst, "LoadMapperDLL");
			ThisDLL->UnloadDLL = (FUnloadMapperDLL)GetProcAddress(ThisDLL->dInst, "UnloadMapperDLL");
			if ((ThisDLL->LoadDLL) && (ThisDLL->UnloadDLL))
			{
				ThisDLL->DI = ThisDLL->LoadDLL(hMainWnd, &EI, CurrentMapperInterface);
				if (ThisDLL->DI)
				{
					//DbgOut(_T("Added mapper pack %s: '%s' v%X.%X (%04X/%02X/%02X)"), Data.cFileName, ThisDLL->DI->Description, ThisDLL->DI->Version >> 16, ThisDLL->DI->Version & 0xFFFF, ThisDLL->DI->Date >> 16, (ThisDLL->DI->Date >> 8) & 0xFF, ThisDLL->DI->Date & 0xFF);
					ThisDLL->Next = MapperDLLs;
					MapperDLLs = ThisDLL;
					ThisDLL = new MapperDLL;
				}
				else
				{
					DbgOut(_T("Failed to load mapper pack %s - version mismatch!"), Data.cFileName);
					FreeLibrary(ThisDLL->dInst);
				}
			}
			else
			{
				DbgOut(_T("Failed to load %s - not a valid mapper pack!"), Data.cFileName);
				FreeLibrary(ThisDLL->dInst);
			}
		}	while (FindNextFile(Handle, &Data));
		FindClose(Handle);
	}
	delete ThisDLL;
	if (MapperDLLs == NULL)
		MessageBox(hMainWnd, _T("Fatal error: unable to locate any mapper DLLs!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
	ZeroMemory(&EI, sizeof(EI));
	ZeroMemory(&RI, sizeof(RI));
	EI.SetCPUReadHandler = SetCPUReadHandler;
	EI.GetCPUReadHandler = GetCPUReadHandler;
	EI.SetCPUReadHandlerDebug = SetCPUReadHandlerDebug;
	EI.GetCPUReadHandlerDebug = GetCPUReadHandlerDebug;
	EI.SetCPUWriteHandler = SetCPUWriteHandler;
	EI.GetCPUWriteHandler = GetCPUWriteHandler;
	EI.SetPPUReadHandler = SetPPUReadHandler;
	EI.GetPPUReadHandler = GetPPUReadHandler;
	EI.SetPPUReadHandlerDebug = SetPPUReadHandlerDebug;
	EI.GetPPUReadHandlerDebug = GetPPUReadHandlerDebug;
	EI.SetPPUWriteHandler = SetPPUWriteHandler;
	EI.GetPPUWriteHandler = GetPPUWriteHandler;
	EI.SetPRGROMSize = SetPRGROMSize;
	EI.SetPRGRAMSize = SetPRGRAMSize;
	EI.SetPRG_ROM4 = SetPRG_ROM4;
	EI.SetPRG_ROM8 = SetPRG_ROM8;
	EI.SetPRG_ROM16 = SetPRG_ROM16;
	EI.SetPRG_ROM32 = SetPRG_ROM32;
	EI.GetPRG_ROM4 = GetPRG_ROM4;
	EI.SetPRG_RAM4 = SetPRG_RAM4;
	EI.SetPRG_RAM8 = SetPRG_RAM8;
	EI.SetPRG_RAM16 = SetPRG_RAM16;
	EI.SetPRG_RAM32 = SetPRG_RAM32;
	EI.GetPRG_RAM4 = GetPRG_RAM4;
	EI.GetPRG_Ptr4 = GetPRG_Ptr4;
	EI.SetPRG_Ptr4 = SetPRG_Ptr4;
	EI.SetPRG_OB4 = SetPRG_OB4;
	EI.SetCHRROMSize = SetCHRROMSize;
	EI.SetCHRRAMSize = SetCHRRAMSize;
	EI.SetCHR_ROM1 = SetCHR_ROM1;
	EI.SetCHR_ROM2 = SetCHR_ROM2;
	EI.SetCHR_ROM4 = SetCHR_ROM4;
	EI.SetCHR_ROM8 = SetCHR_ROM8;
	EI.GetCHR_ROM1 = GetCHR_ROM1;
	EI.SetCHR_RAM1 = SetCHR_RAM1;
	EI.SetCHR_RAM2 = SetCHR_RAM2;
	EI.SetCHR_RAM4 = SetCHR_RAM4;
	EI.SetCHR_RAM8 = SetCHR_RAM8;
	EI.GetCHR_RAM1 = GetCHR_RAM1;
	EI.SetCHR_NT1 = SetCHR_NT1;
	EI.GetCHR_NT1 = GetCHR_NT1;
	EI.GetCHR_Ptr1 = GetCHR_Ptr1;
	EI.SetCHR_Ptr1 = SetCHR_Ptr1;
	EI.SetCHR_OB1 = SetCHR_OB1;
	EI.Mirror_H = Mirror_H;
	EI.Mirror_V = Mirror_V;
	EI.Mirror_4 = Mirror_4;
	EI.Mirror_S0 = Mirror_S0;
	EI.Mirror_S1 = Mirror_S1;
	EI.Mirror_Custom = Mirror_Custom;
	EI.SetIRQ = SetIRQ;
	EI.SetIRQ2 = SetIRQ2;
	
	EI.Set_SRAMSize = Set_SRAMSize;
	EI.DbgOut = DbgOut;
	EI.StatusOut = StatusOut;
	EI.SetPC = SetPC;
	EI.ForceReset = ForceReset;
	EI.RegisterWindow = RegisterWindow;
	EI.UnregisterWindow = UnregisterWindow;
	EI.StopCPU = StopCPU;
	EI.OpenBus = &DummyOpenBus; // Will be changed once a CPU has been loaded
	EI.BootlegExpansionAudio =&Settings::BootlegExpansionAudio;
	EI.CleanN163 =&Settings::CleanN163;
	EI.cpuPC =NULL;
	EI.cpuFI =NULL;

	EI.multiCanSave   =&multiCanSave;
	EI.multiPRGStart  =&multiPRGStart;
	EI.multiCHRStart  =&multiCHRStart;
	EI.multiPRGSize   =&multiPRGSize;
	EI.multiCHRSize   =&multiCHRSize;
	EI.multiMapper    =&multiMapper;
	EI.multiSubmapper =&multiSubmapper;
	EI.multiMirroring =&multiMirroring;
	
	MI = &noCartInserted;
	MI2 = NULL;
}

struct	MapperFindInfo
{
	TCHAR		*filename;
	DLLInfo		*DI;
	MapperInfo	*MI;
};

INT_PTR CALLBACK	DllSelect (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	MapperFindInfo *DLLs = (MapperFindInfo *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	int i;
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		DLLs = (MapperFindInfo *)lParam;
		for (i = 0; DLLs[i].DI != NULL; i++)
			SendDlgItemMessage(hDlg, IDC_DLL_LIST, LB_INSERTSTRING, i, (LPARAM)DLLs[i].filename);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DLL_LIST:
			if (HIWORD(wParam) != LBN_DBLCLK)
			{
				i = SendDlgItemMessage(hDlg, IDC_DLL_LIST, LB_GETCURSEL, 0, 0);
				if (i == LB_ERR)
				{
					SetDlgItemText(hDlg, IDC_DLL_FILEDESC, _T(""));
					SetDlgItemText(hDlg, IDC_DLL_MAPPERDESC, _T(""));
					return FALSE;
				}
				TCHAR *desc;
				desc = new TCHAR[_tcslen(DLLs[i].DI->Description) + 27];
				_stprintf(desc, _T("\"%s\" v%x.%x (%04x/%02x/%02x)"),
					DLLs[i].DI->Description,
					(DLLs[i].DI->Version >> 16) & 0xFFFF, DLLs[i].DI->Version & 0xFFFF,
					(DLLs[i].DI->Date >> 16) & 0xFFFF, (DLLs[i].DI->Date >> 8) & 0xFF, DLLs[i].DI->Date & 0xFF);

				SetDlgItemText(hDlg, IDC_DLL_FILEDESC, desc);
				delete[] desc;

				desc = new TCHAR[_tcslen(DLLs[i].MI->Description) + _tcslen(CompatLevel[DLLs[i].MI->Compatibility]) + 4];
				_stprintf(desc, _T("%s (%s)"), DLLs[i].MI->Description, CompatLevel[DLLs[i].MI->Compatibility]);
				SetDlgItemText(hDlg, IDC_DLL_MAPPERDESC, desc);
				delete[] desc;

				return TRUE;
			}
			// have double-clicks fall through
		case IDOK:
			i = SendDlgItemMessage(hDlg, IDC_DLL_LIST, LB_GETCURSEL, 0, 0);
			EndDialog(hDlg, (INT_PTR)i);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, (INT_PTR)-1);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL	LoadMapper (const ROMInfo *ROM)
{
	int num = 1;
	MapperDLL *ThisDLL = MapperDLLs;
	while (ThisDLL) 
	{	// 1 - count how many DLLs we have (add 1 for null terminator)
		num++;
		ThisDLL = ThisDLL->Next;
	}
	MapperFindInfo *DLLs = new MapperFindInfo[num];
	ZeroMemory(DLLs, sizeof(MapperFindInfo) * num);
	num = 0;
	ThisDLL = MapperDLLs;
	while (ThisDLL)
	{	// 2 - see how many DLLs support our mapper
		DLLs[num].filename = ThisDLL->filename;
		DLLs[num].DI = ThisDLL->DI;
		DLLs[num].MI = DLLs[num].DI->LoadMapper(ROM);
		if (DLLs[num].MI)
			num++;
		else
		{
			DLLs[num].filename = NULL;
			DLLs[num].DI = NULL;
		}
		ThisDLL = ThisDLL->Next;
	}
	if (num == 0)
	{	// none found
		DI = NULL;
		delete[] DLLs;
		return FALSE;
	}
	if (num == 1)
	{	// 1 found
		DI = DLLs[0].DI;
		MI = DLLs[0].MI;
		delete[] DLLs;

		PlugThruDevice::Description =NULL;
		PlugThruDevice::initPlugThruDevice();
		BOOL result =TRUE;;
		if (MI->Load) result =MI->Load();
		if (PlugThruDevice::Description) EI.DbgOut(_T("Plug-through device: %s"), PlugThruDevice::Description);
		return result;
	}
	// else more than one found - ask which one to use
	int choice = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLLSELECT), hMainWnd, DllSelect, (LPARAM)DLLs);
	// -1 indicates that no mapper was chosen
	if (choice >= 0)
	{
		DI = DLLs[choice].DI;
		MI = DLLs[choice].MI;
	}

	// unload whichever mappers weren't chosen
	for (num = 0; DLLs[num].DI != NULL; num++)
	{
		// don't unload the one that was chosen
		if (num == choice)
			continue;
		DLLs[num].DI->UnloadMapper();
	}
	delete[] DLLs;

	if (choice < 0)
		return FALSE;

	if (MI->Load)
		return MI->Load();
	else	
		return TRUE;
}

void	UnloadMapper (void) {
	if (MI) {
		if (MI->Unload)
			MI->Unload();
		MI = &noCartInserted;
	}
	if (DI) {
		if (DI->UnloadMapper)
			DI->UnloadMapper();
		DI = NULL;
	}
	PlugThruDevice::uninitPlugThruDevice();
}

void	Destroy (void)
{
	MapperDLL *ThisDLL = MapperDLLs;
	while (ThisDLL)
	{
		MapperDLLs = ThisDLL->Next;
		ThisDLL->UnloadDLL();
		FreeLibrary(ThisDLL->dInst);
		delete ThisDLL;
		ThisDLL = MapperDLLs;
	}
}
} // namespace MapperInterface
