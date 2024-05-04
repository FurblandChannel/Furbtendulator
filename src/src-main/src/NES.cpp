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
#include "OneBus.h"
#include "OneBus_VT32.h"
#include "OneBus_VT369.h"
#include "UM6578.h"
#include "Sound.h"
#include "AVI.h"
#include "Debugger.h"
#include "States.h"
#include "Movie.h"
#include "Controllers.h"
#include "HeaderEdit.h"
#include <shellapi.h>
#include <algorithm>
#include "Hash.h"
#include "FDSFile/FDSFile.hpp"
#include "FDSFile/FDSFile_BungMFC.hpp"
#include "FDSFile/FDSFile_FDSStick_Binary.hpp"
#include "FDSFile/FDSFile_FDSStick_Raw.hpp"
#include "FDSFile/FDSFile_FDSStick_Raw03.hpp"
#include "FDSFile/FDSFile_FFE.hpp"
#include "FDSFile/FDSFile_FQD.hpp"
#include "FDSFile/FDSFile_fwNES.hpp"
#include "FDSFile/FDSFile_NintendoQD.hpp"
#include "FDSFile/FDSFile_QDC_Raw.hpp"
#include "FDSFile/FDSFile_Venus.hpp"
#include "WXN.h"
#include "DIPSwitch.h"
#include "Cheats.hpp"
#include "FDS.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_StudyBox.hpp"
#include "plugThruDevice_SuperMagicCard_transfer.hpp"
#include <vector>
#include <math.h>
#include "ADPCM_VT1682.hpp"

#pragma comment(lib, "shlwapi.lib")

namespace NES {
int SRAM_Size;

int PRGSizeROM, PRGSizeRAM, CHRSizeROM, CHRSizeRAM;
int PRGMaskROM, PRGMaskRAM, CHRMaskROM, CHRMaskRAM;

BOOL ROMLoaded;
BOOL Scanline;
volatile BOOL Running;
volatile int DoStop;
BOOL FrameStep, GotStep;
BOOL HasMenu;
Settings::Region CurRegion =Settings::REGION_NONE;
BOOL GenericMulticart;
int WhichScreenToShow =0;
int WhichRGBPalette =0;

unsigned char CPU_RAM[8192]; // 2 KiB for normal NES, 2x2 KiB for Vs. Dual, 4 KiB for VT09/VT32/VT369, 8 KiB for UM6578
unsigned char PRG_ROM[MAX_PRGROM_SIZE][0x1000];
unsigned char PRG_RAM[MAX_PRGRAM_SIZE][0x1000];
unsigned char CHR_ROM[MAX_CHRROM_SIZE][0x400];
unsigned char CHR_RAM[MAX_CHRRAM_SIZE][0x400];
unsigned char coin1, coin2, mic;
uint32_t coinDelay1, coinDelay2, micDelay;
DIP::Game dipswitchDefinition;

std::vector<std::vector<uint8_t>> disk28;
TCHAR		name28[MAX_PATH];
TCHAR		name28Full[MAX_PATH];
TCHAR		name28AppData[MAX_PATH];
unsigned int	side28;
bool		modified28;
bool		writeProtected28;
std::mutex	busy28;
int		fqdDevice;
int		fqdFusemap;
int		fqdExpansion;
int		fdsFormat;
char		fdsGameID [5];
int		currentPlugThruDevice;

std::vector<uint8_t> disk35;
TCHAR	name35[MAX_PATH];
std::mutex busy35;

void	Init (void) {
	CPU::CPU[0] =NULL;
	CPU::CPU[1] =NULL;
	APU::APU[0] =NULL;
	APU::APU[1] =NULL;
	PPU::PPU[0] =NULL;
	PPU::PPU[1] =NULL;

	SetupDataPath();
	MapperInterface::Init();
	Controllers::Init();
	Sound::Init();
	GFX::Init();
	AVI::Init();
#ifdef	ENABLE_DEBUGGER
	Debugger::Init();
#endif	/* ENABLE_DEBUGGER */
	States::Init();
#ifdef	ENABLE_DEBUGGER
	Debugger::SetMode(0);
#endif	/* ENABLE_DEBUGGER */
	Settings::LoadSettings();

	GFX::Start();

	CloseFile();

	Running = FALSE;
	DoStop = 0;
	ROMLoaded = FALSE;
	ZeroMemory(&RI, sizeof(RI));

	EnableMenuItem(hMenu, ID_FILE_REPLACE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_DIPSWITCHES, MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_INSERT_COIN1, MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_INSERT_COIN2, MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_BUTTON, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FDS_EJECT, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FDS_INSERT, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FDS_NEXT, MF_GRAYED);
	UpdateTitlebar();
}

void	Destroy (void) {
	unload28();
	unload35();
	if (ROMLoaded) CloseFile();
	GFX::Stop();
	Settings::SaveSettings();
#ifdef	ENABLE_DEBUGGER
	Debugger::Destroy();
#endif	/* ENABLE_DEBUGGER */
	GFX::Destroy();
	Sound::Destroy();
	Controllers::Destroy();
	MapperInterface::Destroy();

	DestroyWindow(hMainWnd);
}

std::wstring getMakerName(uint8_t code) {
	switch(code) {
		case 0x01: return L"Nintendo";
		case 0x07: return L"Enix";
		case 0x08: return L"Capcom";
		case 0x09: return L"Hot-B";
		case 0x0A: return L"Jaleco";
		case 0x0B: return L"Coconuts";
		case 0x18: return L"Hudson";
		case 0x1B: return L"SNK";
		case 0x35: return L"Hector";
		case 0x49: return L"Irem";
		case 0x4A: return L"学研";
		case 0x5F: return L"九娯";
		case 0x8B: return L"BPS";
		case 0x8C: return L"Vic 東海";
		case 0x99: return L"Pack-in Video";
		case 0x9A: return L"日本物産";
		case 0x9B: return L"Tecmo";
		case 0x9C: return L"Imagineer";
		case 0xA2: return L"Scorpion Soft";
		case 0xA4: return L"Konami";
		case 0xA6: return L"Kawada";
		case 0xA7: return L"Takara";
		case 0xA8: return L"Royal";
		case 0xAB: return L"Nexoft";
		case 0xAC: return L"東映";
		case 0xAF: return L"Namco";
		case 0xB1: return L"ASCII";
		case 0xB2: return L"Bandai";
		case 0xB3: return L"Soft-Pro";
		case 0xB6: return L"HAL Laboratory";
		case 0xB9: return L"Pony Canyon";
		case 0xBA: return L"Culture Brain";
		case 0xBB: return L"Sunsoft";
		case 0xBC: return L"Toshiba";
		case 0xBF: return L"Sammy";
		case 0xC0: return L"Taito";
		case 0xC3: return L"Square";
		case 0xC4: return L"徳間書店";
		case 0xC5: return L"Data East";
		case 0xC6: return L"東京書籍";
		case 0xC7: return L"East Cube";
		case 0xCA: return L"Ultra";
		case 0xCB: return L"Vap";
		case 0xCC: return L"Use";
		case 0xCE: return L"FCI";
		case 0xD1: return L"Sofel";
		case 0xD2: return L"Bothtec";
		case 0xDA: return L"Tomy";
		case 0xDB: return L"Hiro";
		case 0xDD: return L"NCS";
		case 0xE1: return L"東和 Chiki";
		case 0xE2: return L"Bandai";
		case 0xE3: return L"Varie";
		case 0xE7: return L"Athena";
		case 0xE8: return L"Asmik";
		case 0xE9: return L"Natsume";
		case 0xEB: return L"Atlus";
		case 0xEE: return L"IGS";
		default: return L"Unknown";
	}	
}

void	FillROMInfo (bool DataRead) {
	if (RI.PRGROMData ==NULL)  {
		RI.PRGROMData =&PRG_ROM[0][0];
		RI.PRGRAMData =&PRG_RAM[0][0];
		RI.CHRROMData =&CHR_ROM[0][0];
		RI.CHRRAMData =&CHR_RAM[0][0];
		RI.PRGROMSize =PRGSizeROM *0x1000;
		RI.PRGRAMSize =PRGSizeRAM *0x1000;
		RI.CHRROMSize =CHRSizeROM *0x0400;
		RI.CHRRAMSize =CHRSizeRAM *0x0400;
	}
	if (DataRead && RI.ROMType ==ROM_INES) {
		RI.PRGROMCRC32 =Hash::CRC32C(0, PRG_ROM, RI.PRGROMSize);
		RI.CHRROMCRC32 =Hash::CRC32C(0, CHR_ROM, RI.CHRROMSize);
		RI.OverallCRC32 =Hash::CRC32C(RI.PRGROMCRC32, CHR_ROM, RI.CHRROMSize);
		EI.DbgOut(_T("PRG-ROM CRC32:\t%08X"), RI.PRGROMCRC32);
		if (RI.PRGROMCRC32 !=RI.OverallCRC32) {
			EI.DbgOut(_T("CHR-ROM CRC32:\t%08X"), RI.CHRROMCRC32);
			EI.DbgOut(_T("Overall CRC32:\t%08X"), RI.OverallCRC32);
		}
		uint16_t prgSum =0;
		for (uint32_t i =0; i <RI.PRGROMSize; i++) prgSum +=RI.PRGROMData[i];
		EI.DbgOut(_T("PRG-ROM sum:\t%04X"), prgSum);
		
		if (RI.CHRROMSize >0) {
			uint16_t chrSum =0;
			for (uint32_t i =0; i <RI.CHRROMSize; i++) chrSum +=RI.CHRROMData[i];
			EI.DbgOut(_T("CHR-ROM sum:\t%04X"), chrSum);
		}
	}
	GenericMulticart =RI.InputType ==INPUT_GENERIC_MULTICART;

	// Parse DIP switch definitions
	EnableMenuItem(hMenu, ID_GAME_DIPSWITCHES, MF_GRAYED);
	RI.dipValueSet =false;
	RI.dipValue =0;
	TCHAR dipFileName[MAX_PATH];
	unsigned int i =GetModuleFileName(NULL, dipFileName, MAX_PATH);
	if (i) {
		while (i) if (dipFileName[--i] =='\\') break;
		dipFileName[i++] ='\\';
		dipFileName[i++] =0;
		_tcscat(dipFileName, L"dip.cfg");
		FILE *handle =_tfopen(dipFileName, _T("rt,ccs=UTF-16LE"));
		if (handle) {
			std::vector<wchar_t> text;
			wint_t c;
			while (1) {
				c =fgetwc(handle);
				if (c ==WEOF) break;
				text.push_back(c);
			};
			std::vector<DIP::Game> dipDefinitions =DIP::parseDIP(text);
			for (auto& game: dipDefinitions) if (game.hasCRC(RI.PRGROMCRC32)) {
				RI.dipValue =game.getDefaultValue();
				dipswitchDefinition =game;
				if (game.settings.size() >1 || game.settings.size() ==1 && game.settings[0].choices.size() >1) {
					DIP::windowDIP(hMainWnd, dipswitchDefinition, RI.dipValue);
					EnableMenuItem(hMenu, ID_GAME_DIPSWITCHES, MF_ENABLED);
				}
				RI.dipValueSet =true;
				break;
			}
			fclose(handle);
		} else
			EI.DbgOut(L"Could not open dip.cfg in emulator directory.");
	}
	Cheats::findCheats();
	
	// Print Cartridge Footer Data
	if (RI.PRGROMSize >=32) {		
		const uint8_t* footer =RI.PRGROMData +RI.PRGROMSize -32;
		int footerChecksum =footer[0x12] +footer[0x13] +footer[0x14] +footer[0x15] +footer[0x16] +footer[0x17] +footer[0x18] +footer[0x19];
		uint8_t prgSizeCode =footer[0x14] >>4;
		uint8_t chrSizeCode =footer[0x14]  &7;
		if (prgSizeCode >2) prgSizeCode++;
		if (chrSizeCode >2) chrSizeCode++;
		uint32_t prgSize =8 <<prgSizeCode <<10;
		uint32_t chrSize =8 <<chrSizeCode <<10;

		if (footerChecksum >0 && prgSize ==RI.PRGROMSize && (footer[0x15] &0x7F) <=4) {
			EI.DbgOut(L"-");
			EI.DbgOut(L"Footer Data%s:", footerChecksum &0xFF? L" (bad checksum)": L"");
			EI.DbgOut(L"Maker Code: %02X (%s)", footer[0x18], getMakerName(footer[0x18]).c_str());

			if (footer[0x17]) {
				for (i =0; i <16; i++) {
					uint8_t c =footer[i];
					if (c !=0xFF && c !=0x20 && c !=0x00) break;
				}
				unsigned int first =i;
	
				for (i =15; i >0; i--) {
					uint8_t c =footer[i];
					if (c !=0xFF && c !=0x20 && c !=0x00) break;
				}
				unsigned int last =i +1;
				std::string title;
				for (i =first; i <last; i++) title +=footer[i];
				EI.DbgOut(L"Title: %S", title.c_str());
			}
			
			const wchar_t *cartridgeTypes[5] ={ L"NROM", L"CNROM", L"UNROM", L"GNROM", L"MMC" };
			EI.DbgOut(L"Cartridge type: %s", cartridgeTypes[footer[0x15] &0x7F]);
			if ((footer[0x15] &0x7F) <4) EI.DbgOut(L"Nametable mirroring: %s", footer[0x15] &0x80? L"Horizonal": L"Vertical");
			
			EI.DbgOut(L"PRG-ROM: %d KiB, CHR-R%cM: %d KiB", prgSize >>10, footer[0x14] &8? 'A': 'O', chrSize >>10);
			
			uint16_t prgSumWhole =0;
			for (i =0; i <RI.PRGROMSize; i++) if ((i &~1) !=RI.PRGROMSize -16) prgSumWhole +=RI.PRGROMData[i];
			uint16_t prgLastBank =0;
			for (i =RI.PRGROMSize -16384; i <RI.PRGROMSize; i++) if (i >=0 && (i &~1) !=RI.PRGROMSize -16) prgLastBank +=RI.PRGROMData[i];
			EI.DbgOut(_T("PRG-ROM sum: %04X (actual: %04X whole/%04X last)"), footer[0x10] <<8 | footer[0x11], prgSumWhole, prgLastBank);

			uint16_t chrSum =0;
			for (i =0; i <RI.CHRROMSize; i++) chrSum +=RI.CHRROMData[i];
			EI.DbgOut(_T("CHR-ROM sum: %04X (actual: %04X)"), footer[0x12] <<8 | footer[0x13], chrSum);			
			EI.DbgOut(L"-");
		}
	}
	
	bool romNeedsVerticalFlip =RI.PRGROMCRC32 ==0xFDA18BAC;
	if (GFX::verticalMode !=romNeedsVerticalFlip) {
		GFX::verticalMode =romNeedsVerticalFlip;
		GFX::verticalFlip =romNeedsVerticalFlip;
		GFX::apertureChanged =true;
		NES::DoStop |=STOPMODE_NOW;
	}
}

const TCHAR *LoadErrorMessage;
TCHAR CurrentFilename[MAX_PATH];

void	OpenFile (TCHAR *filename) {
	FILE *data;
	if (ROMLoaded) CloseFile();

	_tcscpy(CurrentFilename, filename);
	EI.DbgOut(_T("Loading file '%s'..."), filename);
	data = _tfopen(filename, _T("rb"));

	if (!data) {
		MessageBox(hMainWnd, _T("Unable to open file!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		CloseFile();
		return;
	}
	SetRegion(Settings::DefaultRegion);
	WhichScreenToShow =0;

	GFX::g_bSan2 =FALSE;
	LoadErrorMessage =NULL;
	RI.ConsoleType =CONSOLE_HVC;
	RI.InputType =INPUT_UNSPECIFIED;
	RI.ReloadUponHardReset =FALSE;
	currentPlugThruDevice =Settings::plugThruDevice;
	EnableMenuItem(hMenu, ID_PPU_PALETTE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_PPU_MODE_NTSC,  MF_ENABLED);
	EnableMenuItem(hMenu, ID_PPU_MODE_PAL,   MF_ENABLED);
	EnableMenuItem(hMenu, ID_PPU_MODE_DENDY, MF_ENABLED);
	WhichRGBPalette =0;
	GFX::LoadPalette(Settings::Palette[NES::CurRegion]);
	PlugThruDevice::SuperMagicCard::haveFileToSend =false;
	PlugThruDevice::SuperMagicCard::fileData.clear();

	if (!OpenFileiNES(data) &&
		!OpenFileSTBX(data) &&
		!OpenFileNSF(data) &&
		!OpenFileUNIF(data) &&
		!OpenFileMFC(data) &&
		!OpenFileTNES(data) &&
		!OpenFileSMC(data) &&
		!OpenFileFDS(data, filename) &&
		!OpenFileBIN(data) && LoadErrorMessage == NULL)
		LoadErrorMessage =_T("File type not recognized!");

	fclose(data);

	if (LoadErrorMessage) {
		MessageBox(hMainWnd, LoadErrorMessage, _T("Nintendulator"), MB_OK | MB_ICONERROR);
		CloseFile();
		return;
	}

	FillROMInfo(true);

	// initialize bank masks based on returned bank sizes
	PRGMaskROM = getMask(PRGSizeROM - 1) & MAX_PRGROM_MASK;
	PRGMaskRAM = getMask(PRGSizeRAM - 1) & MAX_PRGRAM_MASK;
	CHRMaskROM = getMask(CHRSizeROM - 1) & MAX_CHRROM_MASK;
	CHRMaskRAM = getMask(CHRSizeRAM - 1) & MAX_CHRRAM_MASK;

	// if the ROM loaded without errors, drop the filename into ROMInfo
	RI.Filename = _tcsdup(filename);
	ROMLoaded = TRUE;
	States::SetFilename(filename);

	HasMenu = FALSE;
	if (MI->Config) {
		if (MI->Config(CFG_WINDOW, FALSE)) HasMenu = TRUE;
		EnableMenuItem(hMenu, ID_GAME, MF_ENABLED);
	} else
		EnableMenuItem(hMenu, ID_GAME, MF_GRAYED);

	if (RI.ConsoleType ==CONSOLE_VS) {
		EnableMenuItem(hMenu, ID_GAME_INSERT_COIN1, MF_ENABLED);
		if (RI.INES2_VSFlags ==VS_DUAL || RI.INES2_VSFlags ==VS_BUNGELING)
			EnableMenuItem(hMenu, ID_GAME_INSERT_COIN2, MF_ENABLED);
		else
			EnableMenuItem(hMenu, ID_GAME_INSERT_COIN2, MF_GRAYED);
	} else {
		EnableMenuItem(hMenu, ID_GAME_INSERT_COIN1, MF_GRAYED);
		EnableMenuItem(hMenu, ID_GAME_INSERT_COIN2, MF_GRAYED);
	}
	LoadSRAM();

	if (RI.INES_MapperNum ==365 || RI.INES_MapperNum ==531) RI.InputType =INPUT_SUBOR_KEYBOARD;
	Controllers::SetDefaultInput(RI.InputType);
	EnableMenuItem(hMenu, ID_CPU_SAVESTATE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_LOADSTATE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_PREVSTATE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_NEXTSTATE, MF_ENABLED);

	EnableMenuItem(hMenu, ID_MISC_PLAYMOVIE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_MISC_RECORDMOVIE, MF_ENABLED);

	EnableMenuItem(hMenu, ID_MISC_STARTAVICAPTURE, MF_ENABLED);

	EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_RUN, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_STEP, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_STOP, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_SOFTRESET, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CPU_HARDRESET, MF_ENABLED);
	EnableMenuItem(hMenu, ID_FILE_REPLACE, (RI.ROMType == ROM_FDS)? MF_ENABLED: MF_GRAYED);
	DrawMenuBar(hMainWnd);

	// When switching from Vs. Dual to normal, adjust window size
	GFX::Stop();
	GFX::Start();
	UpdateInterface();

	Reset(RESET_HARD);
	if (Settings::AutoRun || RI.ROMType == ROM_NSF) Start(FALSE);
}

void	SaveSRAM (void) {
	if (Settings::SkipSRAMSave) return;
	TCHAR Filename[MAX_PATH];
	if (RI.ROMType != ROM_FDS) {
		TCHAR* Extension =_T("sav");
		if (RI.ChipRAMSize && (RI.INES_MapperNum ==157 || RI.INES_Flags &2)) {
			bool valid =false;
			switch (RI.INES_MapperNum) {
				case 1:	// 2ME
					if (RI.INES2_SubMapper ==6) {
						_stprintf(Filename, _T("%s\\SRAM\\%s.epr"), DataPath, States::BaseFilename);
						valid =true;
					}
					break;
				case 5:
					_stprintf(Filename, _T("%s\\SRAM\\%s.mmc"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 19:
					_stprintf(Filename, _T("%s\\SRAM\\%s.nmc"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 30:
				case 111:
				case 406:
				case 446:
				case 451:
					_stprintf(Filename, _T("%s\\SRAM\\%s.prg"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 157:
					_stprintf(Filename, _T("%s\\SRAM\\DatachMainUnit.epr"), DataPath);
					valid =true;
					break;
			}
			if (valid) {
				FILE *ChipRAMFile = _tfopen(Filename, _T("wb"));
				if (ChipRAMFile) {
					fwrite(RI.ChipRAMData, 1, RI.ChipRAMSize, ChipRAMFile);
					EI.DbgOut(_T("Saved chip RAM."));
					fclose(ChipRAMFile);
				}
			}
		}
		if (!SRAM_Size) return;
		switch (RI.INES_MapperNum) {
			case 16:	// Normal FCG with 256 byte EPROM
			case 529:	// YY0807
				if (SRAM_Size ==8192) SRAM_Size =256;
				Extension =_T("epr");
				break;
			case 157: 	// Datach Battle Rush (main Datach unit is saved as ChipRAMSize)
				if (SRAM_Size ==8192) SRAM_Size =128;
				Extension =_T("epr");
				break;
			case 159:	// Normal FCG with 128 byte EPROM
				if (SRAM_Size ==8192) SRAM_Size =128;
				Extension =_T("epr");
				break;
			case 427:	// VT369 with EEPROM
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			case 405:	// UMC UM6578 with EEPROM
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			case 164:	// Yancheng with 512 byte EEPROM
			case 558:
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			default:
				Extension =_T("sav");
				break;
		}
		_stprintf(Filename, _T("%s\\SRAM\\%s.%s"), DataPath, States::BaseFilename, Extension);
		FILE *SRAMFile = _tfopen(Filename, _T("wb"));
		if (!SRAMFile) {
			EI.DbgOut(_T("Failed to save SRAM!"));
			return;
		}
		if (RI.ROMType ==ROM_INES && RI.INES_Version >=2 && (RI.INES2_PRGRAM &0xF0 || RI.INES2_CHRRAM &0xF0)) {
			size_t PRGSaveSize =0, CHRSaveSize =0;
			if (RI.INES2_PRGRAM &0xF0) PRGSaveSize =64 << ((RI.INES2_PRGRAM & 0xF0) >> 4);
			if (RI.INES2_CHRRAM &0xF0) CHRSaveSize =64 << ((RI.INES2_CHRRAM & 0xF0) >> 4);
			if (PRGSaveSize) fwrite(PRG_RAM, 1, PRGSaveSize, SRAMFile);
			if (CHRSaveSize) fwrite(CHR_RAM, 1, CHRSaveSize, SRAMFile);
		} else
			fwrite(PRG_RAM, 1, SRAM_Size, SRAMFile);
		EI.DbgOut(_T("Saved SRAM."));
		fclose(SRAMFile);
	}
}
void	LoadSRAM (void) {
	TCHAR Filename[MAX_PATH];
	int len;
	if (RI.ROMType !=ROM_FDS) {
		TCHAR* Extension =_T("sav");
		if (RI.ChipRAMSize && (RI.INES_MapperNum ==157 || RI.INES_Flags &2)) {
			bool valid =false;
			switch (RI.INES_MapperNum) {
				case 1:	// 2ME
					if (RI.INES2_SubMapper ==6) {
						_stprintf(Filename, _T("%s\\SRAM\\%s.epr"), DataPath, States::BaseFilename);
						valid =true;
					}
					break;
				case 5:
					_stprintf(Filename, _T("%s\\SRAM\\%s.mmc"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 19:
					_stprintf(Filename, _T("%s\\SRAM\\%s.nmc"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 30:
				case 111:
				case 406:
				case 446:
				case 451:
					_stprintf(Filename, _T("%s\\SRAM\\%s.prg"), DataPath, States::BaseFilename);
					valid =true;
					break;
				case 157:
					_stprintf(Filename, _T("%s\\SRAM\\DatachMainUnit.epr"), DataPath);
					valid =true;
					break;
			}
			FILE *ChipRAMFile = _tfopen(Filename, _T("rb"));
			if (ChipRAMFile) {
				fseek(ChipRAMFile, 0, SEEK_END);
				len = ftell(ChipRAMFile);
				fseek(ChipRAMFile, 0, SEEK_SET);
				fread(RI.ChipRAMData, 1, RI.ChipRAMSize, ChipRAMFile);
				if ((unsigned) len ==RI.ChipRAMSize)
					EI.DbgOut(_T("Loaded chip RAM (%i bytes)."), RI.ChipRAMSize);
				else
					EI.DbgOut(_T("File length mismatch while loading chip RAM!"));
				fclose(ChipRAMFile);
			}
		}
		if (!SRAM_Size) return;
		switch (RI.INES_MapperNum) {
			case 16:	// Normal FCG with 256 byte EPROM
			case 529:	// YY0807
				if (SRAM_Size ==8192) SRAM_Size =256;
				Extension =_T("epr");
				break;
			case 157: 	// Datach Battle Rush (main Datach unit is saved as ChipRAMSize)
				if (SRAM_Size ==8192) SRAM_Size =128;
				Extension =_T("epr");
				break;
			case 159:	// Normal FCG with 128 byte EPROM
				if (SRAM_Size ==8192) SRAM_Size =128;
				Extension =_T("epr");
				break;
			case 427:	// VT369 with EEPROM
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			case 405:	// UMC UM6578 with EEPROM
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			case 164:	// Yancheng with 512 byte EEPROM
			case 558:
				if (SRAM_Size <=512) Extension =_T("epr");
				break;
			default:
				Extension =_T("sav");
				break;
		}
		_stprintf(Filename, _T("%s\\SRAM\\%s.%s"), DataPath, States::BaseFilename, Extension);
		FILE *SRAMFile = _tfopen(Filename, _T("rb"));
		if (!SRAMFile) return;
		fseek(SRAMFile, 0, SEEK_END);
		len = ftell(SRAMFile);
		fseek(SRAMFile, 0, SEEK_SET);

		if (RI.ROMType ==ROM_INES && RI.INES_Version >=2 && (RI.INES2_PRGRAM &0xF0 || RI.INES2_CHRRAM &0xF0)) {
			size_t PRGSaveSize =0, CHRSaveSize =0, PRGLoadedSize =0, CHRLoadedSize =0;
			if (RI.INES2_PRGRAM &0xF0) PRGSaveSize =64 << ((RI.INES2_PRGRAM & 0xF0) >> 4);
			if (RI.INES2_CHRRAM &0xF0) CHRSaveSize =64 << ((RI.INES2_CHRRAM & 0xF0) >> 4);
			if (PRGSaveSize) {
				CPU::RandomMemory(PRG_RAM, PRGSaveSize);
				PRGLoadedSize =fread(PRG_RAM, 1, PRGSaveSize, SRAMFile);
			}
			if (CHRSaveSize) {
				CPU::RandomMemory(CHR_RAM, CHRSaveSize);
				CHRLoadedSize =fread(CHR_RAM, 1, CHRSaveSize, SRAMFile);
			}
			if (PRGLoadedSize ==PRGSaveSize && CHRLoadedSize ==CHRSaveSize)
				EI.DbgOut(_T("Loaded SRAM (%i bytes PRG and %i bytes CHR)."), PRGSaveSize, CHRSaveSize);
			else
				EI.DbgOut(_T("File length mismatch while loading SRAM!"));
		} else {
			CPU::RandomMemory(PRG_RAM, SRAM_Size);
			fread(PRG_RAM, 1, SRAM_Size, SRAMFile);
			if (len == SRAM_Size)
				EI.DbgOut(_T("Loaded SRAM (%i bytes)."), SRAM_Size);
			else
				EI.DbgOut(_T("File length mismatch while loading SRAM!"));
		}
		fclose(SRAMFile);
	}
}

void	CloseFile (void) {
	SaveSRAM();
	if (ROMLoaded) 	{
		MapperInterface::UnloadMapper();
		ROMLoaded = FALSE;
		EI.DbgOut(_T("ROM unloaded."));
	}
	if (RI.MiscROMData) {
		delete[] RI.MiscROMData;
		RI.MiscROMData =NULL;
		RI.MiscROMSize =0;
	}
	CPU::Unload();

	if (RI.ROMType) {
		if (RI.ROMType ==ROM_FDS)
			unload28();
		else
		if (RI.ROMType == ROM_UNIF)
			delete[] RI.UNIF_BoardName;
		else
		if (RI.ROMType == ROM_NSF) {
			delete[] RI.NSF_Title;
			delete[] RI.NSF_Artist;
			delete[] RI.NSF_Copyright;
		}
		// Allocated using _tcsdup()
		free(RI.Filename);
		ZeroMemory(&RI, sizeof(RI));
	}

	if (AVI::handle)
		AVI::End();
	if (Movie::Mode)
		Movie::Stop();

	EnableMenuItem(hMenu, ID_GAME, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_SAVESTATE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_LOADSTATE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_PREVSTATE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_NEXTSTATE, MF_GRAYED);

	EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FILE_REPLACE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_RUN, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_STEP, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_STOP, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_SOFTRESET, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CPU_HARDRESET, MF_GRAYED);

	EnableMenuItem(hMenu, ID_MISC_PLAYMOVIE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_MISC_RECORDMOVIE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_MISC_STARTAVICAPTURE, MF_GRAYED);

	SRAM_Size = 0;
	DestroyMachine();
}

#define MKID(a) ((unsigned long) \
		(((a) >> 24) & 0x000000FF) | \
		(((a) >>  8) & 0x0000FF00) | \
		(((a) <<  8) & 0x00FF0000) | \
		(((a) << 24) & 0xFF000000))

// Generates a bit mask sufficient to fit the specified value
DWORD	getMask (unsigned int maxval)
{
	DWORD result = 0;
	while (maxval > 0)
	{
		result = (result << 1) | 1;
		maxval >>= 1;
	}
	return result;
}

#include "GameDB_Headers.h"

BOOL	OpenFileSTBX (FILE *in) {
	fseek(in, 0, SEEK_SET);
	try {
		PlugThruDevice::StudyBox::cassette =new PlugThruDevice::StudyBox::StudyBoxFile(in);
	} catch(...) {
		return false;
	}
	currentPlugThruDevice =ID_PLUG_STUDYBOX;
	PlugThruDevice::loadBIOS (_T("BIOS\\STUDYBOX.BIN"), &PRG_ROM[0][0], 262144);
	RI.ROMType =ROM_INES;
	PRGSizeROM =256*1024 /4096;
	CHRSizeROM =0;
	PRGSizeRAM =65536/4096;
	CHRSizeRAM =8192 /1024;
	FillROMInfo(false);
	CreateMachine();
	MapperInterface::LoadMapper(&RI);
	SetRegion(Settings::REGION_NTSC);
	return true;
}

BOOL	OpenFileiNES (FILE *in) {
	unsigned char Header[16];
	bool encrypted =false;

	fseek(in, 0, SEEK_END);
	size_t fileSize =ftell(in);

	fseek(in, 0, SEEK_SET);
	fread(Header, 1, 16, in);
	if (memcmp(Header, "NES\x1A", 4) !=0) {
		fseek(in, 0x30, SEEK_SET);
		fread(Header, 1, 16, in);
		fileSize -=0x30;
		if (memcmp(Header, "NES", 3) !=0) return false;
	}
	if (memcmp(Header+7, "DiskDude!", 9) ==0) memset(Header+7, 0, 9);

	RI.ROMType = ROM_INES;
	RI.INES_Version =((Header[7] & 0x0C) ==0x08)? 2: 1;
	RI.INES_PRGSize = Header[4];
	RI.INES_CHRSize = Header[5];
	RI.INES_MapperNum = ((Header[6] & 0xF0) >> 4) | (Header[7] & 0xF0);
	RI.INES_Flags = (Header[6] & 0x0F) | ((Header[7] & 0x0F) << 4);
	RI.ConsoleType =Header[7] &3;

	// PRGBytes/CHRBytes contain the number of bytes to be read.
	size_t PRGBytes =RI.INES_PRGSize *16384;
	size_t CHRBytes =RI.INES_CHRSize *8192;

	if (RI.INES_Version >=2) {
		RI.INES_MapperNum |= (Header[8] & 0x0F) << 8;
		RI.INES2_SubMapper = (Header[8] & 0xF0) >> 4;

		int PRGSizeHigh =Header[9] &0x0F;
		if (PRGSizeHigh <0xF) {
			RI.INES_PRGSize |= PRGSizeHigh <<8;
			if (RI.INES_PRGSize ==0) RI.INES_PRGSize =4096;
			PRGBytes =RI.INES_PRGSize *16384;
		} else {
			PRGBytes =pow(2, Header[4] >>2) *((Header[4] &3)*2 +1);
			RI.INES_PRGSize =(WORD) (PRGBytes /16384) + ((PRGBytes %16384)? 1: 0);
		}

		int CHRSizeHigh =Header[9] >>4;
		if (CHRSizeHigh <0xF) {
			RI.INES_CHRSize |= CHRSizeHigh <<8;
			CHRBytes =RI.INES_CHRSize *8192;
		} else {
			CHRBytes =pow(2, Header[5] >>2) *((Header[5] &3)*2 +1);
			RI.INES_CHRSize =(WORD) (CHRBytes /8192) + ((CHRBytes %8192)? 1: 0);
		}

		RI.INES2_PRGRAM = Header[10];
		RI.INES2_CHRRAM = Header[11];
		RI.INES2_TVMode = Header[12];
		if (RI.ConsoleType ==3)
			RI.ConsoleType =Header[13];
		else
		if (RI.ConsoleType ==CONSOLE_VS) {
			RI.INES2_VSPalette = Header[13] &0xF;
			RI.INES2_VSFlags = Header[13] >>4;
		}
		else
		if (RI.ConsoleType ==CONSOLE_HVC && Settings::AutoCorrect) {
			if (RI.INES_MapperNum ==256) RI.ConsoleType =CONSOLE_VT09; else
			if (RI.INES_MapperNum ==270) RI.ConsoleType =CONSOLE_VT09; else
			if (RI.INES_MapperNum ==296) RI.ConsoleType =CONSOLE_VT32;
			if (RI.ConsoleType !=CONSOLE_HVC) EI.DbgOut(_T("Console type auto-adjusted based on mapper number!"));
		}
		RI.InputType =Header[15];

		PRGSizeRAM =0;
		if (RI.INES2_PRGRAM &0x0F) PRGSizeRAM += 64 << (RI.INES2_PRGRAM & 0x0F);
		if (RI.INES2_PRGRAM &0xF0) PRGSizeRAM += 64 <<((RI.INES2_PRGRAM & 0xF0) >>4);
		PRGSizeRAM =(PRGSizeRAM / 0x1000) + ((PRGSizeRAM % 0x1000) ? 1 : 0);

		CHRSizeRAM =0;
		if (RI.INES2_CHRRAM &0x0F) CHRSizeRAM += 64 << (RI.INES2_CHRRAM & 0x0F);
		if (RI.INES2_CHRRAM &0xF0) CHRSizeRAM += 64 <<((RI.INES2_CHRRAM & 0xF0) >>4);
		CHRSizeRAM = (CHRSizeRAM / 0x400) + ((CHRSizeRAM % 0x400) ? 1 : 0);
	} else {
		if (RI.INES_PRGSize ==0) RI.INES_PRGSize =256;
		if (RI.ConsoleType ==CONSOLE_VS) RI.InputType =INPUT_VS_NORMAL;
		PRGSizeRAM = 0x02;	// default to 8KB
		CHRSizeRAM = RI.INES_CHRSize? 0x00: 0x08;	// default to 8KB in the absence of CHR-ROM
		if (Header[11] ==1) {	// WXN ROMs with iNES header but encrypted data
			encrypted =true;
			if (RI.INES_MapperNum ==4) RI.INES_MapperNum =256; // for Five Kids
		}
	}
	if (RI.ConsoleType !=CONSOLE_HVC) {
		if (RI.ConsoleType ==CONSOLE_VS)
			EI.DbgOut(_T("Console type: %s (%s)"), HeaderEdit::ConsoleTypes[RI.ConsoleType], HeaderEdit::VSFlags[RI.INES2_VSFlags]);
		else
			EI.DbgOut(_T("Console type: %s"), HeaderEdit::ConsoleTypes[RI.ConsoleType]);
	}
	if (RI.InputType !=INPUT_UNSPECIFIED) EI.DbgOut(_T("Input type: %s"), HeaderEdit::ExpansionDevices[RI.InputType]);

	// A 512-byte trainer may appear between the header and the PRG-ROM data. We treat it as a "Misc ROM".
	if (RI.INES_Flags &0x04) {
		if (RI.INES_Version >=2 && (Header[14] &3) !=0) EI.DbgOut(_T("Cannot have trainer plus Misc ROM! Using only the trainer."));
		RI.MiscROMSize =512;
		RI.MiscROMData =new unsigned char[RI.MiscROMSize];
		fread(RI.MiscROMData, 1, RI.MiscROMSize, in);
	}
	
	// Special treatment for ROMs with BIOS in them. Put the BIOS into the Misc ROM area.
	if (RI.INES_MapperNum ==53) { // Supervision 16-in-1
		RI.INES_PRGSize -=2;
		PRGBytes -=32768;
		RI.MiscROMSize =32768;
		RI.MiscROMData =new unsigned char[RI.MiscROMSize];
		fread(RI.MiscROMData, 1, RI.MiscROMSize, in);
	}

	PRGSizeROM =RI.INES_PRGSize <<2; // from 16 KiB to MapperInterface's 4 KiB
	CHRSizeROM =RI.INES_CHRSize <<3; // From 8 KiB to MapperInterface's 1 KiB
	if (PRGBytes ==8192) PRGSizeROM =2; // For 8 KiB PRG-ROM
	if (PRGBytes ==4096) PRGSizeROM =1; // For 4 KiB PRG-ROM
	if (CHRBytes ==4096) CHRSizeROM =4; // For 4 KiB CHR-ROM
	FillROMInfo(false);

	/* Read PRG-ROM and CHR-ROM */
	size_t PRGRead, CHRRead; // Monitor actual number of bytes read to complain if ROM file is too short
	RI.PRGROMSize =PRGBytes;
	RI.CHRROMSize =CHRBytes;
	PRGRead =fread(PRG_ROM, 1, PRGBytes, in);
	CHRRead =fread(CHR_ROM, 1, CHRBytes, in);
	if (PRGRead !=PRGBytes || CHRRead !=CHRBytes)
		MessageBox(NULL, _T("iNES ROM image is shorter than specified in its header!"), _T("Nintendulator"), MB_OK | MB_ICONWARNING);

	/* Read Misc ROM */
	if (RI.INES_Version >=2 && Header[14] &3 && ~RI.INES_Flags &0x04) {
		RI.MiscROMSize =fileSize -16 -PRGBytes -CHRBytes;
		size_t MiscROMSize_4K =RI.MiscROMSize;
		if (MiscROMSize_4K &0x0FFF) MiscROMSize_4K =(MiscROMSize_4K &~0xFFF) +0x1000;
		RI.MiscROMData =new unsigned char[MiscROMSize_4K];
		fread(RI.MiscROMData, 1, RI.MiscROMSize, in);
	}
	if (RI.MiscROMSize ==0 && RI.ConsoleType ==CONSOLE_VT369) {
		RI.MiscROMSize =4096;
		RI.MiscROMData =new unsigned char[RI.MiscROMSize];
		if (!PlugThruDevice::loadBIOS (_T("BIOS\\VT369-00.BIN"), RI.MiscROMData, 4096)) {
			delete[] RI.MiscROMData;
			RI.MiscROMData =NULL;
			RI.MiscROMSize =0;
		}
	}

	if (encrypted) DecryptWXN();
	CorrectHeader(in);
	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) {
		static TCHAR err[256];
		_stprintf(err, _T("Mapper %i not supported!"), RI.INES_MapperNum);
		LoadErrorMessage =err;
		DestroyMachine();
		return false;
	}

	/* Set up RAM strings */
	size_t PRGRAM, PRGNVRAM, CHRRAM, CHRNVRAM;
	if (RI.INES_Version >=2) {
		PRGRAM   =(RI.INES2_PRGRAM &0x0F)? (64 << (RI.INES2_PRGRAM & 0x0F)): 0;
		PRGNVRAM =(RI.INES2_PRGRAM &0xF0)? (64 <<((RI.INES2_PRGRAM & 0xF0) >>4)): 0;
		CHRRAM   =(RI.INES2_CHRRAM &0x0F)? (64 << (RI.INES2_CHRRAM & 0x0F)): 0;
		CHRNVRAM =(RI.INES2_CHRRAM &0xF0)? (64 <<((RI.INES2_CHRRAM & 0xF0) >>4)): 0;
		PRGRAM +=PRGNVRAM;
		CHRRAM +=CHRNVRAM;
	} else {
		PRGRAM =PRGSizeRAM *4096;
		CHRRAM =CHRSizeRAM *1024;
		PRGNVRAM =(RI.INES_Flags &0x02)? 8192: 0;
		CHRNVRAM =0;
	}
	TCHAR sPRGRAM [16], sPRGNVRAM[16], sCHRRAM [16], sCHRNVRAM [16];
	_stprintf(sPRGRAM, (PRGRAM &0x3FF)? _T("%i bytes"): _T("%i KiB"), (PRGRAM &0x3FF)? PRGRAM: (PRGRAM /1024));
	_stprintf(sPRGNVRAM, (PRGRAM &0x3FF)? _T("%i bytes"): _T("%i KiB"), (PRGRAM &0x3FF)? PRGNVRAM: (PRGNVRAM /1024)); // If PRG-RAM is in bytes, print PRG-NVRAM in bytes as well
	_stprintf(sCHRRAM, (CHRRAM &0x3FF)? _T("%i bytes"): _T("%i KiB"), (CHRRAM &0x3FF)? CHRRAM: (CHRRAM /1024));
	_stprintf(sCHRNVRAM, (CHRRAM &0x3FF)? _T("%i bytes"): _T("%i KiB"), (CHRRAM &0x3FF)? CHRNVRAM: (CHRNVRAM /1024)); // If CHR-RAM is in bytes, print CHR-NVRAM in bytes as well
	EI.DbgOut(_T("iNES Mapper %i.%i: %s%s"), RI.INES_MapperNum, RI.INES2_SubMapper, MI->Description, MapperInterface::CompatLevel[MI->Compatibility]);
	EI.DbgOut(_T("ROM: PRG %i KiB, CHR %i KiB"), PRGBytes /1024, CHRBytes /1024);
	if (PRGRAM || CHRRAM) { // Print nothing about RAM if there isn't any
		TCHAR sMsgPRG [64], sMsgCHR [64];
		sMsgPRG[0] =0; sMsgCHR[0] =0;
		if (PRGNVRAM ==0)
			_stprintf(sMsgPRG, _T("PRG %s"), sPRGRAM);
		else if (PRGNVRAM ==PRGRAM)
			_stprintf(sMsgPRG, _T("PRG %s battery-backed"), sPRGNVRAM);
		else
			_stprintf(sMsgPRG, _T("PRG %s (%s battery-backed)"), sPRGRAM, sPRGNVRAM);
		if (CHRNVRAM ==0)
			_stprintf(sMsgCHR, _T("CHR %s"), sCHRRAM);
		else if (CHRNVRAM ==CHRRAM)
			_stprintf(sMsgCHR, _T("CHR %s battery-backed"), sCHRNVRAM);
		else
			_stprintf(sMsgCHR, _T("CHR %s (%s battery-backed)"), sCHRRAM, sCHRNVRAM);
		EI.DbgOut(_T("RAM: %s%s%s"), PRGRAM? sMsgPRG: _T(""), (PRGRAM && CHRRAM)? _T(", "): _T(""), CHRRAM? sMsgCHR: _T(""));
	}
	EI.DbgOut(_T("Flags: %s%s"), RI.INES_Flags & 0x02 ? _T("Battery, ") : _T(""), RI.INES_Flags & 0x08 ? _T("Four-screen VRAM") : (RI.INES_Flags & 0x01 ? _T("Vertical mirroring") : _T("Horizontal mirroring")));

	EnableMenuItem(hMenu, ID_PPU_MODE_NTSC,  MF_ENABLED);
	EnableMenuItem(hMenu, ID_PPU_MODE_PAL,   MF_ENABLED);
	EnableMenuItem(hMenu, ID_PPU_MODE_DENDY, MF_ENABLED);

	if (RI.ConsoleType ==CONSOLE_VS) {
		if (VSDUAL) {
			if (Settings::VSDualScreens ==1) WhichScreenToShow =1;
			if (RI.INES2_VSFlags ==VS_BUNGELING) WhichScreenToShow =0;
		}
		const uint8_t pals[16] = {
			0, // RP2C03B
			0, // RP2C03G - no dump available, assuming identical to RP2C03B
			1, // RP2C04-0001
			2, // RP2C04-0002
			3, // RP2C04-0003
			4, // RP2C04-0004
			5,  // RC2C03B
			0,  // RC2C03C
			0,  // RC2C05C-01 - no dump available, assuming identical to RC2C05C-03
			0,  // RC2C05C-02 - no dump available, assuming identical to RC2C05C-03
			0,  // RC2C05C-03
			0,  // RC2C05C-04 - no dump available, assuming identical to RC2C05C-03
			0,  // RC2C05C-05 - no dump available, assuming identical to RC2C05C-03
			0,  // unused
			0,  // unused
			0   // unused
		};
		WhichRGBPalette =pals[RI.INES2_VSPalette];
		EnableMenuItem(hMenu, ID_PPU_MODE_NTSC,  MF_DISABLED);
		EnableMenuItem(hMenu, ID_PPU_MODE_PAL,   MF_DISABLED);
		EnableMenuItem(hMenu, ID_PPU_MODE_DENDY, MF_DISABLED);
		SetRegion(Settings::REGION_NTSC);
	} else if (RI.ConsoleType ==CONSOLE_PC10) {
		EnableMenuItem(hMenu, ID_PPU_MODE_NTSC,  MF_DISABLED);
		EnableMenuItem(hMenu, ID_PPU_MODE_PAL,   MF_DISABLED);
		EnableMenuItem(hMenu, ID_PPU_MODE_DENDY, MF_DISABLED);
		SetRegion(Settings::REGION_NTSC);
	} else {
		if (RI.INES_Version >=2) switch(RI.INES2_TVMode &3) {
			case 0:	SetRegion(Settings::REGION_NTSC); break;
			case 1:	SetRegion(Settings::REGION_PAL); break;
			case 2:	break; // Both
			case 3:	SetRegion(Settings::REGION_DENDY); break;
		}
	}
	return true;
}

BOOL	OpenFileUNIF (FILE *in) {
	unsigned long Signature, BlockLen;
	unsigned char *tPRG[0x10], *tCHR[0x10];
	unsigned char *PRGPoint, *CHRPoint;
	unsigned long PRGsize = 0, CHRsize = 0;
	const TCHAR *error = NULL;
	int i;
	unsigned char tp;

	fseek(in, 0, SEEK_SET);
	fread(&Signature, 4, 1, in);
	if (Signature != MKID('UNIF')) return false;

	fseek(in, 28, SEEK_CUR);	// skip "expansion area"

	RI.ROMType = ROM_UNIF;

	RI.UNIF_BoardName = NULL;
	RI.UNIF_Mirroring = 0;
	RI.UNIF_NTSCPAL = 0;
	RI.UNIF_Battery = FALSE;
	RI.UNIF_NumPRG = 0;
	RI.UNIF_NumCHR = 0;
	for (i = 0; i < 0x10; i++) {
		RI.UNIF_PRGSize[i] = 0;
		RI.UNIF_CHRSize[i] = 0;
		tPRG[i] = NULL;
		tCHR[i] = NULL;
	}

	while (!feof(in)) {
		int id = 0;
		fread(&Signature, 4, 1, in);
		fread(&BlockLen, 4, 1, in);
		if (feof(in))
			continue;
		switch (Signature) {
		case MKID('FONT'):
			fread(GFX::pSan2Font, 1, BlockLen, in);
			break;
		case MKID('MAPR'):
			RI.UNIF_BoardName = new char[BlockLen];
			fread(RI.UNIF_BoardName, 1, BlockLen, in);
			break;
		case MKID('TVCI'):
			fread(&tp, 1, 1, in);
			RI.UNIF_NTSCPAL = tp;
			break;
		case MKID('BATR'):
			fread(&tp, 1, 1, in);
			RI.UNIF_Battery = TRUE;
			break;
		case MKID('MIRR'):
			fread(&tp, 1, 1, in);
			RI.UNIF_Mirroring = tp;
			break;
		case MKID('PRGF'):	id++;
		case MKID('PRGE'):	id++;
		case MKID('PRGD'):	id++;
		case MKID('PRGC'):	id++;
		case MKID('PRGB'):	id++;
		case MKID('PRGA'):	id++;
		case MKID('PRG9'):	id++;
		case MKID('PRG8'):	id++;
		case MKID('PRG7'):	id++;
		case MKID('PRG6'):	id++;
		case MKID('PRG5'):	id++;
		case MKID('PRG4'):	id++;
		case MKID('PRG3'):	id++;
		case MKID('PRG2'):	id++;
		case MKID('PRG1'):	id++;
		case MKID('PRG0'):
			RI.UNIF_NumPRG++;
			RI.UNIF_PRGSize[id] = BlockLen;
			PRGsize += BlockLen;
			tPRG[id] = new unsigned char[BlockLen];
			fread(tPRG[id], 1, BlockLen, in);
			break;

		case MKID('CHRF'):	id++;
		case MKID('CHRE'):	id++;
		case MKID('CHRD'):	id++;
		case MKID('CHRC'):	id++;
		case MKID('CHRB'):	id++;
		case MKID('CHRA'):	id++;
		case MKID('CHR9'):	id++;
		case MKID('CHR8'):	id++;
		case MKID('CHR7'):	id++;
		case MKID('CHR6'):	id++;
		case MKID('CHR5'):	id++;
		case MKID('CHR4'):	id++;
		case MKID('CHR3'):	id++;
		case MKID('CHR2'):	id++;
		case MKID('CHR1'):	id++;
		case MKID('CHR0'):
			RI.UNIF_NumCHR++;
			RI.UNIF_CHRSize[id] = BlockLen;
			CHRsize += BlockLen;
			tCHR[id] = new unsigned char[BlockLen];
			fread(tCHR[id], 1, BlockLen, in);
			break;
		case MKID('\0DIN'):
			fseek(in, -7, SEEK_CUR);
			break;
		case MKID('DINF'):
			if (BlockLen ==0) BlockLen =0xCC;
			fseek(in, BlockLen, SEEK_CUR);
			break;
		default:
			fseek(in, BlockLen, SEEK_CUR);
			break;
		}
	}

	PRGPoint = PRG_ROM[0];
	CHRPoint = CHR_ROM[0];

	// check for size overflow, but don't immediately return
	// since we need to clean up after this other stuff too
	if (PRGsize > MAX_PRGROM_SIZE * 0x1000)
		error = _T("PRG ROM is too large! Increase MAX_PRGROM_SIZE and recompile!");

	if (CHRsize > MAX_CHRROM_SIZE * 0x400)
		error = _T("CHR ROM is too large! Increase MAX_CHRROM_SIZE and recompile!");

	PRGSizeROM = (PRGsize / 0x1000) + ((PRGsize % 0x1000) ? 1 : 0);
	CHRSizeROM = (CHRsize / 0x400) + ((CHRsize % 0x400) ? 1 : 0);
	// allow unlimited RAM sizes for UNIF
	PRGSizeRAM = MAX_PRGRAM_SIZE;
	CHRSizeRAM = MAX_CHRRAM_SIZE;

	for (i = 0; i < 0x10; i++)
	{
		if (tPRG[i])
		{
			if (!error)
			{
				memcpy(PRGPoint, tPRG[i], RI.UNIF_PRGSize[i]);
				PRGPoint += RI.UNIF_PRGSize[i];
			}
			delete tPRG[i];
		}
		if (tCHR[i])
		{
			if (!error)
			{
				memcpy(CHRPoint, tCHR[i], RI.UNIF_CHRSize[i]);
				CHRPoint += RI.UNIF_CHRSize[i];
			}
			delete tCHR[i];
		}
	}
	if (error) {
		LoadErrorMessage =error;
		return false;
	}


	/* Check if the UNIF board type can also be expressed as an INES Mapper. If so, use the iNES mapper interface, so that the mapper does not have to be implemented twice. */
	ROMInfo RI2;
	memset(&RI2, 0, sizeof(ROMInfo));
	RI2.ROMType = ROM_INES;
	RI2.INES_MapperNum =65535;
	if (!strcmp(RI.UNIF_BoardName, "NES-NROM-128") || !strcmp(RI.UNIF_BoardName, "NES-NROM-256")) { RI2.INES_MapperNum =0; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-Transformer")) { RI2.INES_MapperNum =0; RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "NES-UNROM") || !strcmp(RI.UNIF_BoardName, "NES-UOROM")) { RI2.INES_MapperNum =2; }
	if (!strcmp(RI.UNIF_BoardName, "NES-CNROM")) { RI2.INES_MapperNum =3; }
	if (!strcmp(RI.UNIF_BoardName, "NES-TLROM") || !strcmp(RI.UNIF_BoardName, "NES-TBROM") || !strcmp(RI.UNIF_BoardName, "NES-TFROM")) { RI2.INES_MapperNum =4; }
	if (!strcmp(RI.UNIF_BoardName, "NES-TKROM")) { RI2.INES_MapperNum =4; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "NES-AMROM")) { RI2.INES_MapperNum =7; RI2.INES2_SubMapper =2; }
	if (!strcmp(RI.UNIF_BoardName, "NES-ANROM")) { RI2.INES_MapperNum =7; RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, "NES-AOROM")) { RI2.INES_MapperNum =7; RI2.INES2_SubMapper =0; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SL1632")) RI2.INES_MapperNum =14;
	if (!strcmp(RI.UNIF_BoardName, "UNL-CC-21")) RI2.INES_MapperNum =27;
	if (!strcmp(RI.UNIF_BoardName, "UNL-AC08")) { RI2.INES_MapperNum =42; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-LH09")) { RI2.INES_MapperNum =42; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-Supervision16in1")) {
		RI2.INES_MapperNum =53;
		// Move EPROM from end to beginning
		unsigned char EPROM[32768], *PRG = (unsigned char *) PRG_ROM;
		memcpy(EPROM, PRG +2048*1024, 32768);
		memmove(PRG+32768, PRG, 2048*1024);
		memcpy(PRG, EPROM, 32768);
	}
	if (!strcmp(RI.UNIF_BoardName, "BMC-SuperHIK8in1")) { RI2.INES_MapperNum =45; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-A60AS")) { RI2.INES_MapperNum =51; RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, " BMC-STREETFIGTER-GAME4IN1") || !strcmp(RI.UNIF_BoardName, "BMC-STREETFIGTER-GAME4IN1")) { RI2.INES_MapperNum =49; RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, "BTL-MARIO1-MALEE2")) { RI2.INES_MapperNum =55; RI2.INES2_PRGRAM=0x05; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-T3H53")) { RI2.INES_MapperNum =59; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-D1038")) { RI2.INES_MapperNum =59; }
	if (!strcmp(RI.UNIF_BoardName, "NES-NTBROM")) { RI2.INES_MapperNum =68; RI2.INES2_SubMapper =1; RI2.INES2_PRGRAM=0x70; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-016-1M")) RI2.INES_MapperNum =79;
	if (!strcmp(RI.UNIF_BoardName, "UNL-VRC7")) { RI2.INES_MapperNum =85; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-TEK90")) RI2.INES_MapperNum =90;
	if (!strcmp(RI.UNIF_BoardName, "UNL-BB")) { RI2.INES_MapperNum =108; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-H2288")) RI2.INES_MapperNum =123;
	if (!strcmp(RI.UNIF_BoardName, "UNL-22211")) { RI2.INES_MapperNum =132; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-72008")) { RI2.INES_MapperNum =133; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-T4A54A")) { RI2.INES_MapperNum =134; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-WX-KB4K")) { RI2.INES_MapperNum =134; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-Sachen-8259D")) RI2.INES_MapperNum =137;
	if (!strcmp(RI.UNIF_BoardName, "UNL-Sachen-8259B")) RI2.INES_MapperNum =138;
	if (!strcmp(RI.UNIF_BoardName, "UNL-Sachen-8259C")) RI2.INES_MapperNum =139;
	if (!strcmp(RI.UNIF_BoardName, "UNL-Sachen-8259A")) RI2.INES_MapperNum =141;
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7032")) RI2.INES_MapperNum =142;
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-NROM")) RI2.INES_MapperNum =143;
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-72007")) RI2.INES_MapperNum =145;
	if (!strcmp(RI.UNIF_BoardName, "UNL-TC-U01-1.5M")) RI2.INES_MapperNum =147;
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-0037")) RI2.INES_MapperNum =148;
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-0036")) RI2.INES_MapperNum =149;
	if (!strcmp(RI.UNIF_BoardName, "UNL-Sachen-74LS374N")) RI2.INES_MapperNum =150;
	if (!strcmp(RI.UNIF_BoardName, "UNL-FS304")) { RI2.INES_MapperNum =162; RI2.INES2_PRGRAM=0x70; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-SUPER24IN1SC03") || !strcmp(RI.UNIF_BoardName, "BMC-Super24in1SC03")) { RI2.INES_MapperNum =176; RI2.INES2_CHRRAM = 0x07;}
	if (!strcmp(RI.UNIF_BoardName, "WAIXING-FS005")) { RI2.INES_MapperNum =176; RI2.INES2_PRGRAM=0x88; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-FK23CA") || !strcmp(RI.UNIF_BoardName, "BMC-FK23C")) {
		RI2.INES_MapperNum =176;
		switch(PRGSizeROM) {
			case 2048/4:	RI2.INES2_CHRRAM = 0x0B; break; // 10-in-1 Omake Game
			case 8192/4:	RI2.INES2_CHRRAM = 0x0C; break; // 120-in-1
			case 16384/4:	RI2.INES2_CHRRAM = 0x0B; break; // 245-in-1 Real Game
		}
	}
	if (!strcmp(RI.UNIF_BoardName, "UNL-FC-28-5027")) { RI2.INES_MapperNum =197;  RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-NovelDiamond9999999in1")) { RI2.INES_MapperNum =201; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-JC-016-2")) { RI2.INES_MapperNum =205; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-8237")) { RI2.INES_MapperNum =215;  RI2.INES2_SubMapper =0; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-8237A")) { RI2.INES_MapperNum =215;  RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-N625092")) { RI2.INES_MapperNum =221; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-Ghostbusters63in1")) { RI2.INES_MapperNum =226; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-42in1ResetSwitch")) { RI2.INES_MapperNum =233; }
	if (!strcmp(RI.UNIF_BoardName, "BTL-150in1A")) { RI2.INES_MapperNum =235; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-212-HONG-KONG")) { RI2.INES_MapperNum =235; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-70in1")) { RI2.INES_MapperNum =236; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-70in1B")) { RI2.INES_MapperNum =236; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-603-5052")) RI2.INES_MapperNum =238;
	if (!strcmp(RI.UNIF_BoardName, "WAIXING-FW01")) { RI2.INES_MapperNum =227; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-43272")) RI2.INES_MapperNum =242;
	if (!strcmp(RI.UNIF_BoardName, "BMC-OneBus") || !strcmp(RI.UNIF_BoardName, "UNL-OneBus") || !strcmp(RI.UNIF_BoardName, "UNL-DANCE")) { RI2.INES_MapperNum =256; RI2.INES2_PRGRAM=0x07; RI.ConsoleType =CONSOLE_VT03; }
	if (!strcmp(RI.UNIF_BoardName, "PEC-586")) { RI2.INES_MapperNum =257;  RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-158B")) { RI2.INES_MapperNum =258; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-F-15")) { RI2.INES_MapperNum =259; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-HPxx") || !strcmp(RI.UNIF_BoardName, "BMC-HP2018-A")) { RI2.INES_MapperNum =260; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-810544-C-A1")) RI2.INES_MapperNum =261;
	if (!strcmp(RI.UNIF_BoardName, "UNL-SHERO")) { RI2.INES_MapperNum =262; RI2.INES_Flags |= 8; RI2.INES2_CHRRAM = 0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KOF97")) RI2.INES_MapperNum =263;
	if (!strcmp(RI.UNIF_BoardName, "UNL-YOKO")) RI2.INES_MapperNum =264;
	if (!strcmp(RI.UNIF_BoardName, "BMC-T-262")) { RI2.INES_MapperNum =265; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-CITYFIGHT")) RI2.INES_MapperNum =266;
	if (!strcmp(RI.UNIF_BoardName, "COOLBOY")) { RI2.INES_MapperNum =268; RI2.INES2_CHRRAM = 0x0C; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "MINDKIDS")) { RI2.INES_MapperNum =268; RI2.INES2_SubMapper =1; RI2.INES2_CHRRAM = 0x0C; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-22026")) RI2.INES_MapperNum =271;
	if (!strcmp(RI.UNIF_BoardName, "BMC-80013-B")) { RI2.INES_MapperNum =274; RI2.INES2_CHRRAM = 0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-GS-2004")) RI2.INES_MapperNum =283;
	if (!strcmp(RI.UNIF_BoardName, "BMC-GS-2013")) RI2.INES_MapperNum =283;
	if (!strcmp(RI.UNIF_BoardName, "UNL-DRIPGAME")) { RI2.INES_MapperNum =284; RI2.INES2_PRGRAM=0x70; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-A65AS")) { RI2.INES_MapperNum =285; RI2.INES2_CHRRAM = 0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-BS-5")) { RI2.INES_MapperNum =286; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-411120-C")) { RI2.INES_MapperNum =287; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3088")) { RI2.INES_MapperNum =287; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-GKCXIN1")) { RI2.INES_MapperNum =288; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-60311C")) RI2.INES_MapperNum =289;
	if (!strcmp(RI.UNIF_BoardName, "BMC-NTD-03")) RI2.INES_MapperNum =290;
	if (!strcmp(RI.UNIF_BoardName, "UNL-DRAGONFIGHTER")) { RI2.INES_MapperNum =292; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-13in1JY110")) { RI2.INES_MapperNum =295; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-TF1201")) { RI2.INES_MapperNum =298; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-11160")) RI2.INES_MapperNum =299;
	if (!strcmp(RI.UNIF_BoardName, "BMC-190in1")) { RI2.INES_MapperNum =300; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-8157")) { RI2.INES_MapperNum =301; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7057")) { RI2.INES_MapperNum =302; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7017")) { RI2.INES_MapperNum =303; RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SMB2J") && PRGSizeROM < 80/4) { RI2.INES_MapperNum =304; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7031") || !strcmp(RI.UNIF_BoardName, "KS7031")) { RI2.INES_MapperNum =305; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7016")) { RI2.INES_MapperNum =306; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7037")) { RI2.INES_MapperNum =307;  RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-TH2131-1")) { RI2.INES_MapperNum =308; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-LH51")) { RI2.INES_MapperNum =309;  RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-LH32")) { RI2.INES_MapperNum =125;  RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SMB2J") && PRGSizeROM >=80/4) { RI2.INES_MapperNum =311; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7013B")) { RI2.INES_MapperNum =312; }
	if (!strcmp(RI.UNIF_BoardName, " BMC-RESET-TXROM") || !strcmp(RI.UNIF_BoardName, "BMC-RESET-TXROM")) { RI2.INES_MapperNum =313; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-64in1NoRepeat")) { RI2.INES_MapperNum =314; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-830134C")) { RI2.INES_MapperNum =315; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-HP898F")) { RI2.INES_MapperNum =319; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-830425C-4391T")) { RI2.INES_MapperNum =320; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3033")) { RI2.INES_MapperNum =322; }
	if (!strcmp(RI.UNIF_BoardName, "FARID_SLROM_8-IN-1")) { RI2.INES_MapperNum =323; }
	if (!strcmp(RI.UNIF_BoardName, "FARID_UNROM_8-IN-1")) { RI2.INES_MapperNum =324; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-MALISB")) { RI2.INES_MapperNum =325; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-10-24-C-A1")) { RI2.INES_MapperNum =327; RI2.INES2_PRGRAM =0x07; RI2.INES2_CHRRAM = 0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-RT-01")) { RI2.INES_MapperNum =328; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-EDU2000")) { RI2.INES_MapperNum =329; RI2.INES2_PRGRAM =0x09; }
	if (!strcmp(RI.UNIF_BoardName, "3D-BLOCK")) { RI2.INES_MapperNum =330; RI2.INES2_CHRRAM = 0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-12-IN-1")) { RI2.INES_MapperNum =331; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-WS")) { RI2.INES_MapperNum =332; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-NEWSTAR-GRM070-8IN1") || !strcmp(RI.UNIF_BoardName, "BMC-8-IN-1")) { RI2.INES_MapperNum =333; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-CTC-09")) { RI2.INES_MapperNum =335; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3046")) { RI2.INES_MapperNum =336; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-CTC-12IN1")) { RI2.INES_MapperNum =337; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-SA005-A")) { RI2.INES_MapperNum =338; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3006")) { RI2.INES_MapperNum =339; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3036")) { RI2.INES_MapperNum =340; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-TJ-03")) { RI2.INES_MapperNum =341; }
	if (!strcmp(RI.UNIF_BoardName, "COOLGIRL")) { RI2.INES_MapperNum =342; RI2.INES2_CHRRAM = 0x0C; RI2.INES2_PRGRAM=0x09; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS106C")) { RI2.INES_MapperNum =343; RI2.INES2_SubMapper =1; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-RESETNROM-XIN1")) { RI2.INES_MapperNum =343; RI2.INES2_SubMapper =0; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-GN-26")) { RI2.INES_MapperNum =344; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-L6IN1")) { RI2.INES_MapperNum =345; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7012")) { RI2.INES_MapperNum =346; RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7030")) { RI2.INES_MapperNum =347; RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-830118C")) { RI2.INES_MapperNum =348; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-G-146")) { RI2.INES_MapperNum =349; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-891227")) { RI2.INES_MapperNum =350; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-820561C")) { RI2.INES_MapperNum =377; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-M2C52A")) { RI2.INES_MapperNum =380; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-830752C")) { RI2.INES_MapperNum =396; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-BS-400R")) { RI2.INES_MapperNum =422; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-BS-4040R")) { RI2.INES_MapperNum =422; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-AB-G1L")) { RI2.INES_MapperNum =428; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-WELL-NO-DG450")) { RI2.INES_MapperNum =428; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-TF2740")) { RI2.INES_MapperNum =428; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-830768C")) { RI2.INES_MapperNum =448; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-SA-9602B")) { RI2.INES_MapperNum =513; RI2.INES2_CHRRAM = 0x09; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-DANCE2000")) { RI2.INES_MapperNum =518; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-EH8813A")) { RI2.INES_MapperNum =519; }
	if (!strcmp(RI.UNIF_BoardName, "DREAMTECH01")) { RI2.INES_MapperNum =521; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-LH10")) { RI2.INES_MapperNum =522; RI2.INES2_PRGRAM =0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BTL-900218")) { RI2.INES_MapperNum =524;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-KS7021A")) { RI2.INES_MapperNum =525;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-BJ-56")) { RI2.INES_MapperNum =526;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-AX-40G")) { RI2.INES_MapperNum =527;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-831128C")) { RI2.INES_MapperNum =528;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-T-230")) { RI2.INES_MapperNum =529;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-AX5705")) { RI2.INES_MapperNum =530;}
	if (!strcmp(RI.UNIF_BoardName, "UNL-LH53")) { RI2.INES_MapperNum =535; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-N49C-300")) { RI2.INES_MapperNum =369; RI2.INES2_PRGRAM=0x07; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-S-2009")) { RI2.INES_MapperNum =434; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3010") || !strcmp(RI.UNIF_BoardName, "BMC-K-3071")) { RI2.INES_MapperNum =438; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-DS-07") || !strcmp(RI.UNIF_BoardName, "BMC-K86B")) { RI2.INES_MapperNum =439; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-BS-110")) { RI2.INES_MapperNum =444; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-DG574B")) { RI2.INES_MapperNum =445; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-TL-8042")) { RI2.INES_MapperNum =453; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-GN-23")) { RI2.INES_MapperNum =458; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-K-3102")) { RI2.INES_MapperNum =458; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-Yhc-Unrom-Cart")) { RI2.INES_MapperNum =500; }
	if (!strcmp(RI.UNIF_BoardName, "BMC-Yhc-Axrom-Cart")) { RI2.INES_MapperNum =501; }
	if (!strcmp(RI.UNIF_BoardName, "Yhc-A/B/Uxrom-Cart")) { RI2.INES_MapperNum =502; }
	if (!strcmp(RI.UNIF_BoardName, "UNL-82112C")) { RI2.INES_MapperNum =540; }
	if (!strcmp(RI.UNIF_BoardName, "KONAMI-QTAI")) { RI2.INES_MapperNum =547; RI2.INES2_PRGRAM=0x77; RI2.INES2_CHRRAM=0x07; }

	if (!strcmp(RI.UNIF_BoardName, "CHINA_ER_SAN2")) { RI2.INES_MapperNum = 19; RI2.INES2_SubMapper = 3; RI2.INES2_PRGRAM=0x71; RI.INES_Flags =2; GFX::g_bSan2 =TRUE; }

	if (RI2.INES_MapperNum ==65535) RI2.ROMType = ROM_UNIF; // No INES mapper found

	if (RI2.ROMType ==ROM_INES) {
		if (RI2.INES2_SubMapper || RI2.INES2_CHRRAM || RI2.INES2_PRGRAM) RI2.INES_Version = 2;
		RI2.INES2_TVMode = RI.UNIF_NTSCPAL;
		RI2.INES_PRGSize = (PRGSizeROM >>2) + ((PRGSizeROM &3)? 1: 0);
		RI2.INES_CHRSize = (CHRSizeROM >>3)  + ((CHRSizeROM &7)? 1: 0);
		if (!RI2.INES2_PRGRAM) RI2.INES2_PRGRAM = RI.UNIF_Battery? 0x70: 0x00;
		if (RI.UNIF_Battery && !(RI2.INES2_PRGRAM &0xF0)) RI2.INES2_PRGRAM <<=4;

		if (!RI2.INES2_CHRRAM && RI2.INES_MapperNum !=256 && RI2.INES_MapperNum !=270 && RI2.INES_MapperNum !=296) RI2.INES2_CHRRAM = CHRSizeROM? 0x00: 0x07;
		RI2.INES_Flags |= (RI.UNIF_Mirroring==1)? 1: 0;
		RI2.INES_Flags |= (RI.UNIF_Mirroring==4)? 8: 0;
		RI2.INES_Flags |= (RI.UNIF_Battery)? 2: 0;
		memcpy(&RI, &RI2, sizeof(ROMInfo));
		FillROMInfo(false);
		CorrectHeader(in);

		PRGSizeRAM = 0;
		if (RI.INES2_PRGRAM & 0xF)
			PRGSizeRAM += 64 << (RI.INES2_PRGRAM & 0xF);
		if (RI.INES2_PRGRAM & 0xF0)
			PRGSizeRAM += 64 << ((RI.INES2_PRGRAM & 0xF0) >> 4);
		PRGSizeRAM = (PRGSizeRAM / 0x1000) + ((PRGSizeRAM % 0x1000) ? 1 : 0);

		CHRSizeRAM = 0;
		if (RI.INES2_CHRRAM & 0xF)
			CHRSizeRAM += 64 << (RI.INES2_CHRRAM & 0xF);
		if (RI.INES2_CHRRAM & 0xF0)
			CHRSizeRAM += 64 << ((RI.INES2_CHRRAM & 0xF0) >> 4);
		CHRSizeRAM = (CHRSizeRAM / 0x400) + ((CHRSizeRAM % 0x400) ? 1 : 0);

		CreateMachine();
		if (!MapperInterface::LoadMapper(&RI)) {
			LoadErrorMessage =_T("Error during UNIF->NES 2.0 Translation!");
			DestroyMachine();
			return false;
		}
		switch(RI.INES2_TVMode &3) {
			case 0:	SetRegion(Settings::REGION_NTSC); break;
			case 1:	SetRegion(Settings::REGION_PAL); break;
			case 2:	break; // Both
			case 3:	SetRegion(Settings::REGION_DENDY); break;
		}
		EI.DbgOut(_T("UNIF mapper %i.%i: %s%s"), RI.INES_MapperNum, RI.INES2_SubMapper, MI->Description, MapperInterface::CompatLevel[MI->Compatibility]);
		EI.DbgOut(_T("ROM: PRG %i KiB, CHR %i KiB"), PRGSizeROM << 2, CHRSizeROM);
		EI.DbgOut(_T("RAM: PRG %i KiB, CHR %i KiB"), PRGSizeRAM << 2, CHRSizeRAM);
		EI.DbgOut(_T("Flags: %s%s"), RI.INES_Flags & 0x02 ? _T("Battery, ") : _T(""), RI.INES_Flags & 0x08 ? _T("Four-screen VRAM") : (RI.INES_Flags & 0x01 ? _T("Vertical mirroring") : _T("Horizontal mirroring")));
		return true;
	}

	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) 	{
		static TCHAR err[1024];
		_stprintf(err, _T("UNIF boardset \"%hs\" not supported!"), RI.UNIF_BoardName);
		DestroyMachine();
		LoadErrorMessage =err;
		return false;
	}

	if (RI.UNIF_NTSCPAL == 0) SetRegion(Settings::REGION_NTSC);
	if (RI.UNIF_NTSCPAL == 1) SetRegion(Settings::REGION_PAL);

	EI.DbgOut(_T("UNIF board %hs: %s%s"), RI.UNIF_BoardName, MI->Description, MapperInterface::CompatLevel[MI->Compatibility]);
	EI.DbgOut(_T("PRG: %iKB; CHR: %iKB"), PRGsize >> 10, CHRsize >> 10);
	EI.DbgOut(_T("Battery status: %s"), RI.UNIF_Battery ? _T("present") : _T("not present"));

	const TCHAR *mir[6] = {_T("Horizontal"), _T("Vertical"), _T("Single-screen L"), _T("Single-screen H"), _T("Four-screen"), _T("Dynamic")};
	EI.DbgOut(_T("Mirroring mode: %i (%s)"), RI.UNIF_Mirroring, mir[RI.UNIF_Mirroring]);

	const TCHAR *ntscpal[3] = {_T("NTSC"), _T("PAL"), _T("Dual")};
	EI.DbgOut(_T("Television standard: %s"), ntscpal[RI.UNIF_NTSCPAL]);
	return true;
}

std::wstring fdsDate(const uint8_t *raw) {
	if (raw[0] ==0x00 && raw[1] ==0x00 && raw[2] ==0x00) return L"-";
	if (raw[0] ==0xFF && raw[1] ==0xFF && raw[2] ==0xFF) return L"-";
	int year  =(raw[0] >>4) *10 +(raw[0] &0xF);
	if (year < 55) year +=1988; else // Heisei period
	if (year >=80) year +=1900; else // Western calendar
	year +=1925; // Showa period
	
	int month =(raw[1] >>4) *10 +(raw[1] &0xF);
	int day   =(raw[2] >>4) *10 +(raw[2] &0xF);
	std::string result =std::to_string(year) +"-" +(month <10? "0": "") +std::to_string(month) +"-" +(day <10? "0": "") +std::to_string(day);
	return std::wstring(result.begin(), result.end());
}

bool	load28 (FILE *handle, TCHAR *name) {
	unload28();

	FDSProtection protection(currentPlugThruDevice !=ID_PLUG_BUNG_GM20);
	std::vector<std::vector<uint8_t>> rawFiles;
	try {
		_tsplitpath(name, NULL, NULL, name28, NULL);
		_tcscpy(name28Full, name);

		fseek(handle, 0, SEEK_END);
		size_t rawSize =ftell(handle);
		fseek(handle, 0, SEEK_SET);
		std::vector<uint8_t> raw(rawSize);
		fread(&raw[0], 1, rawSize, handle);
		rawFiles.push_back(raw);
		writeProtected28 =!!(GetFileAttributesW(name) &FILE_ATTRIBUTE_READONLY);
		
		// If the file name ends with .RDA/.A/-A.xxx, look for and add .RDB.B/-B.xxx, .C/-C.xxx etc. images as well.
		TCHAR base[MAX_PATH];
		TCHAR ext[MAX_PATH];
		TCHAR name2[MAX_PATH];
		_tsplitpath(name, NULL, NULL, base, ext);
		_tcsncpy(name2, name, MAX_PATH);
		TCHAR* letter =NULL;
		
		if (_tcslen(ext) ==4 && ext[0] =='.' && (ext[1] &0xDF) =='R' && (ext[2] &0xDF) =='D' && (ext[3] &0xDF) =='A')
			letter =name2 +_tcslen(name2) -1;
		else
		if (_tcslen(ext) ==2 && ext[0] =='.' && (ext[1] &0xDF) =='A' && ext[2] ==0)
			letter =name2 +_tcslen(name2) -1;
		else
		if (base[_tcslen(base) -2] =='-' && (base[_tcslen(base) -1] &0xDF) =='A')
			letter =name2 +_tcslen(name2) -1 -_tcslen(ext);

		if (letter) {
			while (1) {
				*letter =*letter +1;
				FILE* handle2 =_tfopen(name2, _T("rb"));
				if (!handle2) break;
				if (GetFileAttributesW(name) &FILE_ATTRIBUTE_READONLY) writeProtected28 =true;
				
				fseek(handle2, 0, SEEK_END);
				rawSize =ftell(handle2);
				fseek(handle2, 0, SEEK_SET);
				raw.resize(rawSize);
				fread(&raw[0], 1, rawSize, handle2);
				rawFiles.push_back(raw);
				
				fclose(handle2);
				EI.DbgOut(_T("Also loaded %s"), name2);
			}
		}
		
		fqdDevice =0;
		fqdFusemap =0;
		fqdExpansion =0;
		fdsFormat =0;
		FDSFile_FDSStick_Binary binaryImage;
		std::string imageType;
		for (auto& c: rawFiles) {
			std::unique_ptr<FDSFile> sideImage =FDSFile::autodetect(c, protection);
			binaryImage.data.insert(binaryImage.data.end(), sideImage->data.begin(), sideImage->data.end());
			if (sideImage->type() =="fwNES")            fdsFormat =1; else
			if (sideImage->type() =="fwNES Headerless") fdsFormat =2; else
			if (sideImage->type() =="Fancy QuickDisk") {
				fqdDevice    =dynamic_cast<FDSFile_FQD*>(sideImage.get())->targetDevice;
				fqdFusemap   =dynamic_cast<FDSFile_FQD*>(sideImage.get())->targetConfig;
				fqdExpansion =dynamic_cast<FDSFile_FQD*>(sideImage.get())->expDevice;
				fdsFormat    =3;
				if (!ROMLoaded) switch (fqdDevice) {
					case 0x01: currentPlugThruDevice =ID_PLUG_NONE; break;
					case 0x10: currentPlugThruDevice =ID_PLUG_VENUS_GC1M; break;
					case 0x11: currentPlugThruDevice =ID_PLUG_VENUS_GC2M; break;
					case 0x12: currentPlugThruDevice =ID_PLUG_VENUS_TGD_4P; break;
					case 0x13: currentPlugThruDevice =ID_PLUG_VENUS_TGD_6P; break;
					case 0x14: currentPlugThruDevice =ID_PLUG_VENUS_TGD_6M; break;
					case 0x15: currentPlugThruDevice =ID_PLUG_VENUS_MD; break;
					case 0x20: currentPlugThruDevice =ID_PLUG_BUNG_GD1M; break;
					case 0x21: currentPlugThruDevice =ID_PLUG_BUNG_SGD2M; break;
					case 0x22: currentPlugThruDevice =ID_PLUG_BUNG_SGD4M; break;
					case 0x23: currentPlugThruDevice =ID_PLUG_BUNG_GM20; break;
					case 0x30: currentPlugThruDevice =ID_PLUG_FFE_SMC; break;
					case 0x31: currentPlugThruDevice =ID_PLUG_FFE_SMC; break;
					case 0x32: currentPlugThruDevice =ID_PLUG_FFE_SMC; break;
					case 0x33: currentPlugThruDevice =ID_PLUG_FFE_SMC; break;
					case 0x40: currentPlugThruDevice =ID_PLUG_SOUSEIKI_FAMMY; break;
				}
				if (!ROMLoaded) RI.InputType =fqdExpansion;
			} else
			if (sideImage->type() =="Nintendo QD")                       fdsFormat =4;  else
			if (sideImage->type() =="FDSStick Binary Data")              fdsFormat =5;  else
			if (sideImage->type() =="FDSStick 8-Bit Flux Reversal Data") fdsFormat =6;  else
			if (sideImage->type() =="FDSStick 2-Bit Flux Reversal Data") fdsFormat =7;  else
			if (sideImage->type() =="Bung MFC")                          fdsFormat =8;  else
			if (sideImage->type() =="Venus")                             fdsFormat =9;  else
			if (sideImage->type() =="QDC 8-Bit Flux Reversal Data")      fdsFormat =10; else
			if (sideImage->type() =="Front Fareast")                     fdsFormat =11;
			imageType =sideImage->type();
		}
		static const char tile2char[128] = {
			'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', // 00
			'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V', // 10
			'W','X','Y','Z',' ',',','.',';','®','&',' ',' ','=',' ',' ',' ', // 20
			'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?', // 30
			'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O', // 40
			'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',// 50
			'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o', // 60
			'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' '  // 70
		};
		for (int i =0; i <4; i++) {
			int c =binaryImage.data[0][0].data[0x10 -1 +i];
			fdsGameID[i] =c ==' '? c: tile2char[c &0x7F];
		}
		EI.DbgOut(L"");
		EI.DbgOut(L"FDS Disk Header:");
		EI.DbgOut(L"Game \"%S\" rev%u", fdsGameID, binaryImage.data[0][0].data[0x14 -1]);
		EI.DbgOut(L"Maker: $%02X (%s)", binaryImage.data[0][0].data[0x0F -1], getMakerName(binaryImage.data[0][0].data[0x0F -1]).c_str());
		EI.DbgOut(L"Disk Type: %u, Version: $%02X", binaryImage.data[0][0].data[0x17 -1], binaryImage.data[0][0].data[0x37 -1]);
		EI.DbgOut(L"Manufactured: %s", fdsDate(&binaryImage.data[0][0].data[0x1F -1]).c_str());
		EI.DbgOut(L"Rewritten: %s", fdsDate(&binaryImage.data[0][0].data[0x2C -1]).c_str());
		EI.DbgOut(L"");
		
		disk28 =binaryImage.outputSides();
		side28 =0;
		modified28 =false;
		EI.DbgOut(_T("%S disk image with %i disk side%s loaded"), imageType.c_str(), disk28.size(), disk28.size() >1? _T("s"): _T(""));

		EnableMenuItem(hMenu, ID_FDS_EJECT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FDS_INSERT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FDS_NEXT, MF_ENABLED);
		return true;
	} catch(std::runtime_error& e) {
		std::string message =e.what();
		EI.DbgOut(_T("%S"), message.c_str());
		return false;
	} catch(...) { // Other exception: not an FDS file at all
		return false;
	}
}

void	commit28 (void) {
	TCHAR FileName[MAX_PATH] = {0};
	_tcscpy(FileName, name28);
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter =_T("fwNES Disk Image (*.FDS)\0") _T("*.fds\0")                         // 1
                         _T("fwNES Headerless Disk Image (*.FDS)\0") _T("*.fds\0")              // 2
	                 _T("Fancy QuickDisk Image (*.FQD)\0") _T("*.fqd\0")                    // 3
			 _T("Nintendo QD Image (*.QD)\0") _T("*.qd\0")                          // 4
			 _T("FDSStick Binary Data (*.BIN)\0") _T("*.bin\0")                     // 5
			 _T("FDSStick 8-Bit Flux Reversal Data (*.RAW)\0") _T("*.raw\0")        // 6
			 _T("FDSStick 2-Bit Flux Reversal Data (*.RAW)\0") _T("*.raw\0")        // 7
			 _T("Bung MFC Disk Image (*.A)\0") _T("*.A\0")                          // 8
			 _T("Venus Disk Image (*.A)\0") _T("*.A\0")                             // 9
			 _T("QDC RAW Image (*.RAW)\0") _T("*.raw\0")                            // 10
			 _T("Front Fareast Disk Image (*.FFE)\0") _T("*.ffe\0")                 // 11
	                 _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex =fdsFormat;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_ROM;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("");
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;	
	if (!GetSaveFileName(&ofn)) return;
	_tcscpy(Settings::Path_ROM, FileName);
	Settings::Path_ROM[ofn.nFileOffset - 1] = 0;
	
	std::lock_guard<std::mutex> lock(busy28);
	std::unique_ptr<FDSFile> output;
	switch(ofn.nFilterIndex) {
		case 1: output =std::make_unique<FDSFile_fwNES>(); break;
		case 2: output =std::make_unique<FDSFile_fwNES_Headerless>(); break;
		case 3: output =std::make_unique<FDSFile_FQD>(fqdDevice, fqdFusemap, fqdExpansion); break;
		case 4: output =std::make_unique<FDSFile_NintendoQD>(); break;
		case 5: output =std::make_unique<FDSFile_FDSStick_Binary>(); break;
		case 6: output =std::make_unique<FDSFile_FDSStick_Raw>(); break;
		case 7: output =std::make_unique<FDSFile_FDSStick_Raw03>(); break;
		case 8: output =std::make_unique<FDSFile_BungMFC>(); break;
		case 9: output =std::make_unique<FDSFile_Venus>(); break;
		case 10: output =std::make_unique<FDSFile_QDC_Raw>(); break;
		case 11: output =std::make_unique<FDSFile_FFE>(); break;
		default: return;
	}
	try {
		FDSProtection protection(currentPlugThruDevice !=ID_PLUG_BUNG_GM20);
		for (auto& c: disk28) {
			FDSFile_FDSStick_Binary sideImage(c, protection);
			output->data.insert(output->data.end(), sideImage.data.begin(), sideImage.data.end());
		}
		std::vector<std::vector<uint8_t> > outSides =output->outputSides();
		
		TCHAR baseOutputName[MAX_PATH] = {0};
		_tsplitpath(FileName, NULL, NULL, baseOutputName, NULL);
		
		for (unsigned int i =0; i <outSides.size(); i++) {			
			FILE *F =_wfopen(output->getNameForSide(std::wstring(baseOutputName), i).c_str(), L"wb");
			if (F) {
				fwrite(&outSides[i][0], 1, outSides[i].size(), F);
				fclose(F);
			} else
				throw std::runtime_error("Cannot open file");
		}
		modified28 =false;
		RI.modifiedSide28 =false;
	} catch(std::exception& e) {
		std::string message =e.what();
		MessageBox(hMainWnd, _T("Cannot save file. Try a different output format."), _T("FDS"), MB_OK);
		return;
	}
}

bool	unload28 (void) {
	if (disk28.size() ==0) return true;
	if (RI.modifiedSide28) modified28 =true;
	while (modified28) {
		int action =MessageBox(hMainWnd, _T("Save changes to loaded 2.8\" disk image? You will be able to specify a different filename if you want."), _T("FDS"), MB_YESNOCANCEL);
		if (action ==IDCANCEL) return false;
		if (action ==IDNO) break;
		if (action ==IDYES) commit28();
	}
	eject28();
	EnableMenuItem(hMenu, ID_FDS_EJECT, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FDS_INSERT, MF_GRAYED);
	EnableMenuItem(hMenu, ID_FDS_NEXT, MF_GRAYED);
	std::vector<std::vector<uint8_t>> empty;
	disk28 =empty;
	name28[0] =0;
	name28AppData[0] =0;
	return true;
}

void	insert28 (void) {
	if (RI.diskData28 ==NULL && disk28.size() >0) {
		std::lock_guard<std::mutex> lock(busy28);
		RI.diskData28 =&disk28[side28];
		RI.changedDisk28 =true;
		RI.modifiedSide28 =false;
		EnableMenuItem(hMenu, ID_FDS_INSERT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FDS_EJECT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FDS_NEXT, MF_GRAYED);
	}
};

void	eject28 (void) {
	if (RI.diskData28 !=NULL) {
		std::lock_guard<std::mutex> lock(busy28);
		if (RI.modifiedSide28) modified28 =true;
		RI.diskData28 =NULL;
		RI.changedDisk28 =true;
		EnableMenuItem(hMenu, ID_FDS_EJECT, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FDS_INSERT, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FDS_NEXT, MF_ENABLED);
	}
}

void	next28 (void) {
	if (RI.diskData28 ==NULL && disk28.size()) {
		if (++side28 >=disk28.size()) side28 =0;
	}
}

void	previous28 (void) {
	if (RI.diskData28 ==NULL && disk28.size()) {
		if (--side28 <0) side28 =disk28.size() -1;
	}
}

int	FDSSave (FILE *out) {
	int clen = 0;

	writeLong(disk28.size());
	for (auto& side: disk28) {
		writeLong(side.size());
		for (auto &c: side) writeByte(c);
	}
	return clen;
}

int	FDSLoad (FILE *in, int version_id) {
	int clen = 0;
	int sides, bytes;

	disk28.clear();

	readLong(sides);
	while(sides--) {
		std::vector<uint8_t> side;
		readLong(bytes);
		while(bytes--) {
			uint8_t c;
			readByte(c);
			side.push_back(c);
		}
		disk28.push_back(side);
	}
	return clen;
}

BOOL	OpenFileFDS (FILE *handle, TCHAR *name) {
	if (load28(handle, name) && PlugThruDevice::loadBIOS(_T("BIOS\\DISKSYS.ROM"), &PRG_ROM[0][0], 8192)) {
		RI.ROMType =ROM_FDS;
		PRGSizeROM =8192 /4096;	// 8 KiB of FDS BIOS
		CHRSizeROM =0;
		PRGSizeRAM =32768/4096;	// 32 KiB of PRG-RAM in FDS RAM Adapter
		CHRSizeRAM =8192 /1024; // 8 KiB of CHR-RAM in FDS RAM Adapter
		FillROMInfo(false);
		CreateMachine();
		FDS::load(false);
		MapperInterface::LoadMapper(&RI);
		SetRegion(Settings::REGION_NTSC);
		return TRUE;
	} return FALSE;
}

bool	load35 (TCHAR *name) {
	FILE* handle =_tfopen(name, _T("rb"));
	if (handle) {
		fseek(handle, 0, SEEK_END);
		size_t rawSize =ftell(handle);
		fseek(handle, 0, SEEK_SET);

		if (rawSize ==720*1024 || rawSize ==1440*1024) {
			if (RI.diskData35) unload35();
			disk35.resize(rawSize);
			fread(&disk35[0], 1, rawSize, handle);
			_tcscpy(name35, name);
			RI.diskData35 =&disk35;
			RI.changedDisk35 =true;
			EI.DbgOut(_T("Loaded 3.5 disk image"));
			fclose(handle);
			return true;
		} else {
			EI.DbgOut(_T("Not a raw sector image of a 720K or 1.44M disk"));
			fclose(handle);
			return false;
		}
	} else {
		EI.DbgOut(_T("Cannot open file"));
		return false;
	}
}

void	unload35 (void) {
	FILE* handle =_tfopen(name35, _T("wb"));
	if (handle) {
		fwrite(&disk35[0], 1, disk35.size(), handle);
		fclose(handle);
		EI.DbgOut(L"Changes written to %s", name35);
	}
	disk35.clear();
	RI.diskData35 =NULL;
	RI.changedDisk35 =true;
	name35[0] =0;
}

BOOL	OpenFileNSF (FILE *in) {
	unsigned char Header[128];	// Header Bytes
	size_t ROMlen;
	int LoadAddr;

	fseek(in, 0, SEEK_END);
	ROMlen = ftell(in) - 128;
	fseek(in, 0, SEEK_SET);
	fread(Header, 1, 128, in);
	if (memcmp(Header, "NESM\x1a", 5)) return false;
	if (Header[5] > 2) {
		LoadErrorMessage =_T("This NSF version is not supported!");
		return false;
	}
	size_t dataBlockSize =Header[0x7D] | (Header[0x7E] <<8) | (Header[0x7F] <<16);
	if (dataBlockSize && dataBlockSize < ROMlen) {
		RI.MiscROMSize =ROMlen -dataBlockSize;
		ROMlen =dataBlockSize;
	}

	RI.ROMType = ROM_NSF;
	RI.NSF_DataSize = ROMlen;
	RI.NSF_NumSongs = Header[0x06];
	RI.NSF_SoundChips = Header[0x7B];
	RI.NSF_NTSCPAL = Header[0x7A];
	RI.NSF_NTSCSpeed = Header[0x6E] | (Header[0x6F] << 8);
	RI.NSF_PALSpeed = Header[0x78] | (Header[0x79] << 8);
	memcpy(RI.NSF_InitBanks, &Header[0x70], 8);
	RI.NSF_InitSong = Header[0x07];
	LoadAddr = Header[0x08] | (Header[0x09] << 8);
	RI.NSF_InitAddr = Header[0x0A] | (Header[0x0B] << 8);
	RI.NSF_PlayAddr = Header[0x0C] | (Header[0x0D] << 8);
	RI.NSF_Title = new char[32];
	RI.NSF_Artist = new char[32];
	RI.NSF_Copyright = new char[32];
	memcpy(RI.NSF_Title, &Header[0x0E], 32);
	RI.NSF_Title[31] = 0;
	memcpy(RI.NSF_Artist, &Header[0x2E], 32);
	RI.NSF_Artist[31] = 0;
	memcpy(RI.NSF_Copyright, &Header[0x4E], 32);
	RI.NSF_Copyright[31] = 0;

	if (ROMlen > MAX_PRGROM_SIZE * 0x1000) {
		LoadErrorMessage =_T("NSF is too large! Increase MAX_PRGROM_SIZE and recompile!");
		return false;
	}

	if (memcmp(RI.NSF_InitBanks, "\0\0\0\0\0\0\0\0", 8)) {
		fread(&PRG_ROM[0][0] + (LoadAddr & 0x0FFF), 1, ROMlen, in);
		PRGSizeROM = ROMlen + (LoadAddr & 0xFFF);
	} else {
		memcpy(RI.NSF_InitBanks, "\x00\x01\x02\x03\x04\x05\x06\x07", 8);
		fread(&PRG_ROM[0][0] + (LoadAddr & 0x7FFF), 1, ROMlen, in);
		PRGSizeROM = ROMlen + (LoadAddr & 0x7FFF);
	}
	if (dataBlockSize) {	// If the NSF data block size was specified properly, then load the metadata chunks as a Misc. ROM
		RI.MiscROMData =new unsigned char[RI.MiscROMSize];
		fread(RI.MiscROMData, 1, RI.MiscROMSize, in);
	}
	if (RI.NSF_SoundChips &4 && PRGSizeROM <32768) PRGSizeROM =32768;

	PRGSizeROM = (PRGSizeROM / 0x1000) + ((PRGSizeROM % 0x1000) ? 1 : 0);
	PRGSizeRAM = 0x2;	// 8KB at $6000-$7FFF
	// no CHR at all
	CHRSizeROM = 0;
	CHRSizeRAM = 0;
	
	SetRegion(RI.NSF_NTSCPAL &2? Settings::DefaultRegion: RI.NSF_NTSCPAL &1? Settings::REGION_PAL: Settings::REGION_NTSC);

	if ((RI.NSF_NTSCSpeed == 16666) || (RI.NSF_NTSCSpeed == 16667)) {
		EI.DbgOut(_T("Adjusting NSF playback speed for NTSC..."));
		RI.NSF_NTSCSpeed = 16639;
	}
	if (RI.NSF_PALSpeed == 20000) 	{
		EI.DbgOut(_T("Adjusting NSF playback speed for PAL..."));
		RI.NSF_PALSpeed = 19997;
	}

	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) {
		LoadErrorMessage =_T("NSF support not found!");
		DestroyMachine();
		return false;
	}
	EI.DbgOut(_T("NSF file loaded"));
	EI.DbgOut(_T("Data length: %iKB"), RI.NSF_DataSize >> 10);
	return true;
}

BOOL	OpenFileMFC (FILE *in) {
	unsigned char Header[16];
	BOOL encrypted;

	fseek(in, 0, SEEK_SET);
	fread(Header, 1, 16, in);
	if (*(unsigned long *)Header != MKID('mfc\x00')) return false;

	encrypted =Header[11] !=0;
	RI.ROMType =ROM_INES;

	RI.INES_Version =2;
	RI.INES_PRGSize =Header[4];
	RI.INES_CHRSize =Header[5];
	Settings::Region region =Settings::REGION_DENDY;
	TCHAR* MapperName;

	int MFCmapper = ((Header[6] & 0xF0) >> 4) | (Header[7] & 0xF0);
	switch (MFCmapper) {
		case 1:		RI.INES_MapperNum =256;
				RI.INES2_SubMapper =1;
				RI.INES_Flags =1; // Vertical Mirroring
				RI.ConsoleType =CONSOLE_VT03;
				MapperName =_T("V.R. Technology VT03");
				break;
		case 3:		RI.INES_MapperNum =4;
				RI.INES2_SubMapper =0;
				RI.INES_Flags =1; // Vertical Mirroring
				MapperName =_T("Nintendo MMC3");
				break;
		case 30:	RI.INES_MapperNum = 176;
				RI.INES2_SubMapper = 2;
				RI.INES2_PRGRAM = 0x90; // 32 KiB
				RI.INES2_CHRRAM = 7; // 8 KiB
				RI.INES_Flags = 2; //Battery
				MapperName =_T("Waixing FS005");
				break;
		case 61:	// Both mappers 164 and 227 use the exact same header values.
				// If the first 32 KiB of PRG ROM have 8D 00 50 (STA $5000) in them, it's mapper 164, otherwise 227.
				RI.INES_MapperNum = 164; // Will be changed to 227 later if necessary.
				RI.INES2_PRGRAM = 0x70; // 8 KiB NVRAM
				RI.INES2_CHRRAM = 0x07;
				RI.INES_Flags = 2; // Battery
				MapperName =_T("Waixing FS301");
				break;
		default:	EI.DbgOut(_T("MFC Mapper number %d"), MFCmapper);
				LoadErrorMessage =_T("Unknown MFC mapper number!");
				return false;
	}
	PRGSizeRAM = 0;
	if (RI.INES2_PRGRAM & 0xF)
		PRGSizeRAM += 64 << (RI.INES2_PRGRAM & 0xF);
	if (RI.INES2_PRGRAM & 0xF0)
		PRGSizeRAM += 64 << ((RI.INES2_PRGRAM & 0xF0) >> 4);
	PRGSizeRAM = (PRGSizeRAM / 0x1000) + ((PRGSizeRAM % 0x1000) ? 1 : 0);

	CHRSizeRAM = 0;
	if (RI.INES2_CHRRAM & 0xF)  CHRSizeRAM += 64 << (RI.INES2_CHRRAM & 0xF);
	if (RI.INES2_CHRRAM & 0xF0) CHRSizeRAM += 64 << ((RI.INES2_CHRRAM & 0xF0) >> 4);
	CHRSizeRAM = (CHRSizeRAM / 0x400) + ((CHRSizeRAM % 0x400) ? 1 : 0);

	PRGSizeROM = RI.INES_PRGSize <<2;
	CHRSizeROM = RI.INES_CHRSize <<3;

	fread(PRG_ROM, 1, RI.INES_PRGSize <<14, in);
	fread(CHR_ROM, 1, RI.INES_CHRSize <<13, in);
	if (encrypted) DecryptWXN();

	if (RI.INES_MapperNum ==164) { // Check if it's really 164, or actually 227
		unsigned char binMapperWrite[3] = { 0x8D, 0x00, 0x50 };
		unsigned char *first32K =(unsigned char*) PRG_ROM;
		if (std::search(first32K, first32K +0x8000, binMapperWrite, binMapperWrite+3) ==first32K +0x8000) {
			RI.INES_MapperNum =227;
			MapperName =_T("Waixing FW01");
		}
	}

	FillROMInfo(false);
	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) {
		static TCHAR err[256];
		_stprintf(err, _T("Mapper %i not supported!"), RI.INES_MapperNum);
		LoadErrorMessage =err;
		DestroyMachine();
		return false;
	}
	EI.DbgOut(_T("MFC Mapper %d (%s)"), MFCmapper, MapperName);
	EI.DbgOut(_T("ROM: PRG %i KiB, CHR %i KiB"), RI.INES_PRGSize << 4, RI.INES_CHRSize << 3);
	SetRegion(region);
	return true;
}

BOOL	OpenFileBIN (FILE *in) {
	fseek(in, 0, SEEK_END);
	size_t fileSize =ftell(in);

	// Detection step 1: File size must be a power of two. This means that only one bit in fileSize must be set.
	int bits =0;
	while (fileSize) {
		if (fileSize &1) bits++;
		fileSize >>=1;
	}
	fileSize =ftell(in);
	fseek(in, 0, SEEK_SET);
	if (bits !=1) return false;

	RI.INES_PRGSize = (WORD) (fileSize >>14) + ((fileSize &16383)? 1: 0);
	RI.INES_CHRSize = 0;
	if (RI.INES_PRGSize >=64*1024*1024/16384) RI.INES_PRGSize =64*1024*1024/16384; // Use only first 64 MiB

	RI.ROMType = ROM_INES;
	RI.INES_Version = 1; // For CorrectHeader
	RI.INES_MapperNum = 256;
	RI.INES2_SubMapper = 0;
	RI.INES_Flags =1; // Vertical Mirroring
	RI.ConsoleType =CONSOLE_VT03;

	RI.INES2_PRGRAM = 7; // 8K
	PRGSizeRAM = 64 << (RI.INES2_PRGRAM & 0xF);
	PRGSizeRAM = (PRGSizeRAM / 0x1000) + ((PRGSizeRAM % 0x1000) ? 1 : 0);
	PRGSizeROM = RI.INES_PRGSize <<2;

	fseek(in, 0, SEEK_SET);
	fread(PRG_ROM, 1, RI.INES_PRGSize <<14, in);

	// Detection step 2: There must be a valid Reset vector. This means that the word at the end of the first 512 KiB must be between $C000 and $FFFA
	uint8_t* cPRGROM =(uint8_t*) &PRG_ROM[0][0];
	uint32_t fileMask =fileSize -1;
	uint16_t ResetVector =cPRGROM[0x7FFFC &fileMask] | (cPRGROM[0x7FFFD &fileMask] <<8);
	if (ResetVector <0xC000 || ResetVector >=0xFFFA) return false;

	FillROMInfo(false);
	CorrectHeader(in);

	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) {
		static TCHAR err[256];
		_stprintf(err, _T("Mapper %i not supported!"), RI.INES_MapperNum);
		LoadErrorMessage =err;
		DestroyMachine();
		return false;
	}
	EI.DbgOut(_T("VT02/VT03 ROM image loaded!"));
	EI.DbgOut(_T("ROM: PRG %i KiB, CHR %i KiB"), RI.INES_PRGSize << 4, RI.INES_CHRSize << 3);
	EI.DbgOut(_T("RAM: PRG %i KiB, CHR %i KiB"), PRGSizeRAM << 2, CHRSizeRAM);
	EI.DbgOut(_T("Flags: %s%s"), RI.INES_Flags & 0x02 ? _T("Battery, ") : _T(""), RI.INES_Flags & 0x08 ? _T("Four-screen VRAM") : (RI.INES_Flags & 0x01 ? _T("Vertical mirroring") : _T("Horizontal mirroring")));
	return true;
}

BOOL	OpenFileSMC (FILE *in) {
		unsigned char Header[512];

	// Detect SMC file by size. It will always be a multiple of 512 bytes, and either 512 or 1024 bytes above an 8192 boundary.
	fseek(in, 0, SEEK_END);
	size_t bytesToRead =ftell(in);
	if (bytesToRead ==0x20208) { // FFE Disk Writer image
		fseek(in, 0, SEEK_SET);
		fread(Header, 1, 8, in);
		if (Header[0] !=0xC0 && Header[1] !=0x00) return false;
	} else {
		if (bytesToRead &0x1FF || (bytesToRead &0x1FFF) !=512 && (bytesToRead &0x1FFF) !=1024) return false; // SMC files are always multiples of 512 bytes in size
	
		fseek(in, 0, SEEK_SET);
		fread(Header, 1, 512, in);
		if ( Header[0] &0x40 && (bytesToRead &0x1FFF) !=1024) return false; // If a trainer is present, file size must be 1024 bytes above 8192 boundary
		if (~Header[0] &0x40 && (bytesToRead &0x1FFF) != 512) return false; // If no trainer is present, file size must be 512 bytes above 8192 boundary
	}
	PlugThruDevice::SuperMagicCard::fileData.resize(bytesToRead);

	fseek(in, 0, SEEK_SET);
	fread(&PlugThruDevice::SuperMagicCard::fileData[0], 1, bytesToRead, in);
	PlugThruDevice::SuperMagicCard::haveFileToSend =true;
	NES::currentPlugThruDevice =ID_PLUG_FFE_SMC;

	// Connect the FDS RAM Adapter, so we can use the 2.8 disk functionalities
	RI.ROMType =ROM_FDS;
	PRGSizeROM =8192 /4096;	// 8 KiB of FDS BIOS
	CHRSizeROM =0;
	PRGSizeRAM =32768/4096;	// 32 KiB of PRG-RAM in FDS RAM Adapter
	CHRSizeRAM =8192 /1024; // 8 KiB of CHR-RAM in FDS RAM Adapter
	FillROMInfo(false);
	CreateMachine();
	MapperInterface::LoadMapper(&RI);
	SetRegion(Settings::REGION_NTSC);

	int mode =Header[7] ==0xAA? 8: Header[1] >>5;
	static const TCHAR* modeNames[9] = {
		_T("#0 (UNROM)"),
		_T("#1 (UN1ROM+CHRSW)"),
		_T("#2 (UOROM)"),
		_T("#3 (Reverse UNROM)"),
		_T("#4 (GNROM)"),
		_T("#5 (CNROM-256)"),
		_T("#6 (CNROM-128)"),
		_T("#7 (NROM-256)"),
		_T("Super Magic Card")
	};
	EI.DbgOut(_T("Loaded FFE Super Magic Card image:"));
	int PRGSize =0, CHRSize =0;
	bool pseudoCHR =false;
	if (Header[7] ==0xAA) {
		PRGSize =Header[3] *8;
		CHRSize =Header[4] *8;
	} else
	if (Header[0] &0x30) {
		PRGSize =(Header[0] &0x20)? 256: 128;
		CHRSize =(Header[0] &0x10)? 256: 128;
		pseudoCHR =true;
	} else switch(mode) {
		case 0: PRGSize =128; CHRSize =  0; break;
		case 1: PRGSize =128; CHRSize =128; pseudoCHR =true; break;
		case 2: PRGSize =256; CHRSize =  0; break;
		case 3: PRGSize =256; CHRSize =  0; break;
		case 4: PRGSize =128; CHRSize = 32; break;
		case 5: PRGSize = 32; CHRSize = 32; break;
		case 6: PRGSize = 32; CHRSize = 16; break;
		case 7: PRGSize = 32; CHRSize =  8; break;
	}
	EI.DbgOut(_T("- PRG data: %d KiB"), PRGSize);
	EI.DbgOut(_T("- CHR data: %d KiB%s"), CHRSize, pseudoCHR? _T(" (cached)"): _T(""));
	EI.DbgOut(_T("- Banking mode: %s"), modeNames[mode]);
	EI.DbgOut(_T("- Mirroring: %s"), Header[1] &0x10? _T("Horizontal"): _T("Vertical"));
	if (Header[0] &0x40) {
		int trainerAddr =Header[7] ==0xAA? Header[2] <<8: 0x7000;
		int trainerInit =Header[7] ==0xAA? trainerAddr: 0x7003;
		EI.DbgOut(_T("- Trainer: 512 bytes @$%04X, init @$%04X"), trainerAddr, trainerInit);
	}
	return true;
}

BOOL	OpenFileTNES (FILE *in) {
	unsigned char Header[16];

	fseek(in, 0, SEEK_SET);
	fread(Header, 1, 16, in);
	if (memcmp(Header, "TNES", 4) !=0) return false;
	RI.ROMType =ROM_INES;
	RI.INES_Version =2;
	switch(Header[4]) {
		case 0:	RI.INES_MapperNum = 0; break;	// NROM
		case 1:	RI.INES_MapperNum = 1; break;	// SxROM
		case 2:	RI.INES_MapperNum = 9; break;	// PNROM
		case 3:	RI.INES_MapperNum = 4; break;	// TxROM
		case 4:	RI.INES_MapperNum =10; break;	// FxROM
		case 5:	RI.INES_MapperNum = 5; break;	// ExROM
		case 6:	RI.INES_MapperNum = 2; break;	// UxROM
		case 7:	RI.INES_MapperNum = 3; break;	// CNROM
		case 9:	RI.INES_MapperNum = 7; break;	// AxROM
		case 100: return false;			// Famicom Disk System, handled by OpenFileFDS
		default: LoadErrorMessage =_T("Unknown TNES Mapper"); return false;
	}
	RI.INES_PRGSize =(Header[5] >>1) + (Header[5] &1);
	RI.INES_CHRSize =Header[6];
	RI.INES2_PRGRAM =Header[7]? (Header[9]? 0x70: 0x07): 0x00;
	RI.INES2_CHRRAM =(!Header[6])? 0x07: 0x00;
	PRGSizeROM = RI.INES_PRGSize <<2;
	CHRSizeROM = RI.INES_CHRSize <<3;
	PRGSizeRAM =Header[7]? 8192/4096: 0;
	CHRSizeRAM =(!Header[6])? 8192/1024: 0;
	RI.INES_Flags =((Header[8] ==2)? 0x01: 0x00) | ((Header[9])? 0x02:0x00);

	fread(PRG_ROM, 1, Header[5] <<13, in);
	fread(CHR_ROM, 1, Header[6] <<13, in);
	CreateMachine();
	if (!MapperInterface::LoadMapper(&RI)) {
		static TCHAR err[256];
		_stprintf(err, _T("Mapper %i not supported!"), RI.INES_MapperNum);
		LoadErrorMessage =err;
		DestroyMachine();
		return false;
	}
	EI.DbgOut(_T("TNES Mapper %i: %s%s"), Header[4], MI->Description, MapperInterface::CompatLevel[MI->Compatibility]);
	EI.DbgOut(_T("ROM: PRG %i KiB, CHR %i KiB"), RI.INES_PRGSize << 4, RI.INES_CHRSize << 3);
	EI.DbgOut(_T("RAM: PRG %i KiB, CHR %i KiB"), PRGSizeRAM << 2, CHRSizeRAM);
	EI.DbgOut(_T("Flags: %s%s"), RI.INES_Flags & 0x02 ? _T("Battery, ") : _T(""), RI.INES_Flags & 0x08 ? _T("Four-screen VRAM") : (RI.INES_Flags & 0x01 ? _T("Vertical mirroring") : _T("Horizontal mirroring")));

	SetRegion(Settings::REGION_NTSC);
	return true;
}

void	CreateMachine(void) {
	GFX::smallLCDMode =false;
	GFX::verticalMode =false;
	CPU::callbacks.clear();
	switch(RI.ConsoleType) {
		case CONSOLE_BITCORP:
			CPU::CPU[0] =new CPU::CPU_RP2A03_Decimal(CPU_RAM);
			APU::APU[0] =new APU::APU_RP2A03();
			PPU::PPU[0] =new PPU::PPU_RP2C02();
			break;
		case CONSOLE_VT01STN:
			CPU::CPU[0] =new CPU::CPU_RP2A03(CPU_RAM);
			APU::APU[0] =new APU::APU_RP2A03();
			PPU::PPU[0] =new PPU::PPU_VT01STN();
			break;
		case CONSOLE_VT02:
			CPU::CPU[0] =new CPU::CPU_OneBus(CPU_RAM);
			APU::APU[0] =new APU::APU_OneBus();
			PPU::PPU[0] =new PPU::PPU_OneBus();
			break;
		case CONSOLE_VT03:
			CPU::CPU[0] =new CPU::CPU_OneBus(CPU_RAM);
			APU::APU[0] =new APU::APU_OneBus();
			PPU::PPU[0] =new PPU::PPU_VT03();
			break;
		case CONSOLE_VT09:
			CPU::CPU[0] =new CPU::CPU_VT09(CPU_RAM);
			APU::APU[0] =new APU::APU_OneBus();
			PPU::PPU[0] =new PPU::PPU_VT03();
			break;
		case CONSOLE_VT32:
			CPU::CPU[0] =new CPU::CPU_VT32(CPU_RAM);
			APU::APU[0] =new APU::APU_VT32();
			PPU::PPU[0] =new PPU::PPU_VT32();
			break;
		case CONSOLE_VT369:
			CPU::CPU[0] =new CPU::CPU_VT369(CPU_RAM);
			APU::APU[0] =new APU::APU_VT369();
			PPU::PPU[0] =new PPU::PPU_VT369();
			if (!Settings::VT369SoundHLE) {
				CPU::CPU[1] =new CPU::CPU_VT369_Sound(vt369SoundRAM);
				PPU::PPU[1] =new PPU::PPU_Dummy(1);
			}
			break;
		case CONSOLE_UM6578:
			CPU::CPU[0] =new CPU::CPU_UM6578(CPU_RAM, um6578extraRAM);
			APU::APU[0] =new APU::APU_UM6578();
			PPU::PPU[0] =new PPU::PPU_UM6578();
			break;
		case CONSOLE_VS:
			CPU::CPU[0] =new CPU::CPU_RP2A03(0, CPU_RAM);
			APU::APU[0] =new APU::APU_RP2A03(0);
			PPU::PPU[0] =new PPU::PPU_RP2C02(0);
			if (VSDUAL) {
				CPU::CPU[1] =new CPU::CPU_RP2A03(1, CPU_RAM +2048);
				APU::APU[1] =new APU::APU_RP2A03(1);
				PPU::PPU[1] =new PPU::PPU_RP2C02(1);
			}
			break;
		default:
			CPU::CPU[0] =new CPU::CPU_RP2A03(CPU_RAM);
			if (RI.ROMType ==ROM_NSF && RI.NSF_SoundChips &0x40)
				APU::APU[0] =new APU::APU_OneBus();
			else
				APU::APU[0] =new APU::APU_RP2A03();
			PPU::PPU[0] =new PPU::PPU_RP2C02();
			break;
	}
	if (PlugThruDevice::needSecondCPU(currentPlugThruDevice)) {
		CPU::CPU[1] =new CPU::CPU_RP2A03(1, 2048, CPU_RAM +2048);
		PPU::PPU[1] =new PPU::PPU_Dummy(1);
	}
	APU::SetRegion();
	PPU::SetRegion();
	EI.OpenBus = &CPU::CPU[0]->LastRead;
	EI.cpuPC =&CPU::CPU[0]->PC;
	EI.cpuFI =&CPU::CPU[0]->FI;
}

void	DestroyMachine() {
	if (CPU::CPU[0]) delete CPU::CPU[0];
	if (CPU::CPU[1]) delete CPU::CPU[1];
	if (APU::APU[0]) delete APU::APU[0];
	if (APU::APU[1]) delete APU::APU[1];
	if (PPU::PPU[0]) delete PPU::PPU[0];
	if (PPU::PPU[1]) delete PPU::PPU[1];
	CPU::CPU[0] =NULL;
	CPU::CPU[1] =NULL;
	APU::APU[0] =NULL;
	APU::APU[1] =NULL;
	PPU::PPU[0] =NULL;
	PPU::PPU[1] =NULL;
}

void	SetRegion (Settings::Region NewRegion) {
	CurRegion = NewRegion;
	PPU::SetRegion();
	APU::SetRegion();
	Sound::SetRegion();
	GFX::SetRegion();
	Settings::ApplySettingsToMenu();
	// Resize window if aspect ratio would change
	if (Settings::FixAspect) UpdateInterface();
}

void	InitHandlers (void) {
	PlugThruDevice::uninitHandlers();
	int i;
	for (i =0x0; i <0x10; i++) {
		EI.SetCPUReadHandler(i, CPU::ReadPRG);
		EI.SetCPUWriteHandler(i, CPU::WritePRG);
		EI.SetPRG_OB4(i);
	}
	for (i =0x0; i <0x40; i++) {
		EI.SetPPUReadHandler(i, PPU::BusRead);
		EI.SetPPUWriteHandler(i, PPU::BusWriteCHR);
		EI.SetCHR_OB1(i);
	}
	EI.SetCPUReadHandler(0, CPU::ReadRAM);	EI.SetCPUWriteHandler(0, CPU::WriteRAM);
	EI.SetCPUReadHandler(1, CPU::ReadRAM);	EI.SetCPUWriteHandler(1, CPU::WriteRAM);
	EI.SetCPUReadHandler(2, PPU::IntRead);	EI.SetCPUWriteHandler(2, PPU::IntWrite);
	EI.SetCPUReadHandler(3, PPU::IntRead);	EI.SetCPUWriteHandler(3, PPU::IntWrite);
	if (RI.ConsoleType ==CONSOLE_UM6578) {
		EI.SetCPUReadHandler(5, CPU::ReadRAM2);
		EI.SetCPUReadHandlerDebug(5, CPU::ReadRAM2);
		EI.SetCPUWriteHandler(5, CPU::WriteRAM2);
	}
	if (CPU::CPU[1]) {
		for (i =0x10; i <0x20; i++) {
			EI.SetCPUReadHandler(i, CPU::ReadPRG);
			EI.SetCPUWriteHandler(i, CPU::WritePRG);
			EI.SetPRG_OB4(i);
		}
		EI.SetCPUReadHandler(0x10, CPU::ReadRAM);	EI.SetCPUWriteHandler(0x10, CPU::WriteRAM);
		EI.SetCPUReadHandler(0x11, CPU::ReadRAM);	EI.SetCPUWriteHandler(0x11, CPU::WriteRAM);
		if (PPU::PPU[1]) {
			EI.SetCPUReadHandler(0x12, PPU::IntRead);	EI.SetCPUWriteHandler(0x12, PPU::IntWrite);
			EI.SetCPUReadHandler(0x13, PPU::IntRead);	EI.SetCPUWriteHandler(0x13, PPU::IntWrite);
		}
	}
	if (RI.ConsoleType ==CONSOLE_VT369) {
		// Embedded ROM: main CPU at $1000-$1FFF, sound CPU at $4000-4FFFF
		EI.SetCPUReadHandler(0x01, CPU::ReadPRG);
		EI.SetCPUWriteHandler(0x01, CPU::WritePRG);
		if (RI.MiscROMSize &0x1000) EI.SetPRG_Ptr4(0x01, RI.MiscROMData, FALSE);
		if (!Settings::VT369SoundHLE) {
			EI.SetCPUReadHandler(0x14, CPU::ReadPRG);
			EI.SetCPUWriteHandler(0x14, CPU::WritePRG);
			if (RI.MiscROMSize &0x1000) EI.SetPRG_Ptr4(0x14, RI.MiscROMData, FALSE);
		}
		
		// sound CPU registers
		if (!Settings::VT369SoundHLE) {
			EI.SetCPUReadHandler(0x12, CPU::readVT369Sound);
			EI.SetCPUWriteHandler(0x12, CPU::writeVT369Sound);
		}
		EI.SetCPUReadHandlerDebug(0x04, CPU::readVT369SoundRAM);
		
		// VRAM mapped into CPU address space
		EI.SetCPUReadHandler(0x03, CPU::ReadPRG);
		EI.SetCPUWriteHandler(0x03, CPU::WritePRG);
		EI.SetCPUWriteHandler(0x05, CPU::writeVT369Palette);
	}

	// special check for Vs. Unisystem ROMs with NES 2.0 headers to apply a special PPU
	if (RI.ROMType ==ROM_INES && RI.ConsoleType ==CONSOLE_VS && RI.INES_Version ==2) {
		int VsPPU = RI.INES2_VSPalette;
		// only 5 of the special PPUs need this behavior
		if ((VsPPU >= 0x8) && (VsPPU <= 0xC))
		{
			EI.SetCPUReadHandler(2, PPU::IntReadVs);	EI.SetCPUWriteHandler(2, PPU::IntWriteVs);
			EI.SetCPUReadHandler(3, PPU::IntReadVs);	EI.SetCPUWriteHandler(3, PPU::IntWriteVs);
			switch (VsPPU) {
			case 0x8:	PPU::PPU[0]->VsSecurity = 0x1B;
					if (PPU::PPU[1]) PPU::PPU[1]->VsSecurity = 0x1B;
					break;
			case 0x9:	PPU::PPU[0]->VsSecurity = 0x3D;
					if (PPU::PPU[1]) PPU::PPU[1]->VsSecurity = 0x3D;
					break;
			case 0xA:	PPU::PPU[0]->VsSecurity = 0x1C;
					if (PPU::PPU[1]) PPU::PPU[1]->VsSecurity = 0x1C;
					break;
			case 0xB:	PPU::PPU[0]->VsSecurity = 0x1B;
					if (PPU::PPU[1]) PPU::PPU[1]->VsSecurity = 0x1B;
					break;
			case 0xC:	PPU::PPU[0]->VsSecurity = 0x00;
					if (PPU::PPU[1]) PPU::PPU[1]->VsSecurity = 0x00;
					break;	// don't know what value the 2C05-05 uses
			}
		}
	}

	EI.SetCPUReadHandler(4, APU::IntRead);
	//EI.SetCPUReadHandlerDebug(4, APU::IntRead);
	EI.SetCPUWriteHandler(4, APU::IntWrite);
	if (APU::APU[1]) {
		EI.SetCPUReadHandler(0x14, APU::IntRead);	EI.SetCPUWriteHandler(0x14, APU::IntWrite);
	}

	if (RI.ConsoleType ==CONSOLE_UM6578) {
		for (i =0x0; i <0x8; i++) { // PPU 0000-1FFF: 8 KiB of PPU-internal VRAM
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteNT);
			PPU::PPU[0]->CHRPointer[i] = PPU::PPU[0]->VRAM[i];
			PPU::PPU[0]->Writable[i] =TRUE;
		}
		for (i =0x8; i <0x10; i++) { // PPU 2000-3FFF: 2 KiB of PPU-internal VRAM, not mirrored
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteNT);
			if (i <=0x9) {
				PPU::PPU[0]->CHRPointer[i] =PPU::PPU[0]->VRAM[i &1 |8];
				PPU::PPU[0]->Writable[i] =TRUE;
			} else {
				PPU::PPU[0]->CHRPointer[i] =PPU::OpenBus;
				PPU::PPU[0]->Writable[i] =FALSE;				
			}
		}
		for (i =0x10; i <0x20; i++) { // PPU 4000-7FFF: Nothing
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteCHR);
			EI.SetCHR_OB1(i);
		}
		for (i =0x20; i <0x40; i++) {
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteCHR);
			EI.SetCHR_OB1(i);
		}
	} else {
		for (i = 0x0; i < 0x8; i++) {
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteCHR);
			EI.SetCHR_OB1(i);
		}
		for (i = 0x8; i < 0x10; i++) {
			EI.SetPPUReadHandler(i, PPU::BusRead);
			EI.SetPPUWriteHandler(i, PPU::BusWriteNT);
			EI.SetCHR_OB1(i);
		}
		if (PPU::PPU[1]) {
			for (i = 0x40; i < 0x48; i++) {
				EI.SetPPUReadHandler(i, PPU::BusRead);
				EI.SetPPUWriteHandler(i, PPU::BusWriteCHR);
				EI.SetCHR_OB1(i);
			}
			for (i = 0x48; i < 0x50; i++) {
				EI.SetPPUReadHandler(i, PPU::BusRead);
				EI.SetPPUWriteHandler(i, PPU::BusWriteNT);
				EI.SetCHR_OB1(i);
			}
		}
	}
	PlugThruDevice::initHandlers();
	EI.ReadPRG  =EI.GetCPUReadHandler(0x8);
	EI.WritePRG =EI.GetCPUWriteHandler(0x8);
	EI.ReadCHR  =EI.GetPPUReadHandler(0x0);
	EI.WriteCHR =EI.GetPPUWriteHandler(0x0);
	EI.ReadAPU  =EI.GetCPUReadHandler(0x4);
	EI.WriteAPU =EI.GetCPUWriteHandler(0x4);
}

int	NewPC;
void	Reset (RESET_TYPE ResetType) {
	GFX::frameCount =0;
	NewPC =0;
	size_t PRGSaveSize =0, CHRSaveSize =0;
	InitHandlers();
	if (RI.ROMType ==ROM_FDS) FDS::reset(ResetType);

	switch (ResetType) {
	case RESET_HARD:
		//EI.DbgOut(_T("Performing hard reset..."));
		if (RI.ReloadUponHardReset && RI.ROMType !=ROM_FDS) {
			TCHAR PreviousFilename[MAX_PATH];
			_tcscpy(PreviousFilename, CurrentFilename);
			//EI.DbgOut(_T("ROM data has been modified, reloading."));
			OpenFile(PreviousFilename);
			return;
		}
		if (RI.ROMType ==ROM_INES && RI.INES_Version >=2 && (RI.INES2_PRGRAM &0xF0 || RI.INES2_CHRRAM &0xF0)) {
			if (RI.INES2_PRGRAM &0xF0) PRGSaveSize =64 << ((RI.INES2_PRGRAM & 0xF0) >> 4);
			if (RI.INES2_CHRRAM &0xF0) CHRSaveSize =64 << ((RI.INES2_CHRRAM & 0xF0) >> 4);
		} else {
			PRGSaveSize =SRAM_Size;
			CHRSaveSize =0;
		}
		CPU::RandomMemory((unsigned char *)PRG_RAM+PRGSaveSize, sizeof(PRG_RAM)-PRGSaveSize);
		CPU::RandomMemory((unsigned char *)CHR_RAM+CHRSaveSize, sizeof(CHR_RAM)-CHRSaveSize);
		if (MI2) {
			MI = MI2;
			MI2 = NULL;
		}
		CPU::PowerOn();
		PPU::PowerOn();
		APU::PowerOn();
		*EI.multiCanSave =FALSE;
		*EI.multiMapper =RI.INES_MapperNum;
		*EI.multiSubmapper =RI.INES2_SubMapper;
		if (MI && MI->Reset)
			MI->Reset(RESET_HARD);
		break;
	case RESET_SOFT:
		//EI.DbgOut(_T("Performing soft reset..."));
		if (MI2) {
			MI = MI2;
			MI2 = NULL;
		}
		if ((MI) && (MI->Reset))
			MI->Reset(RESET_SOFT);
		break;
	}
	APU::Reset();
	CPU::Reset();
	if (CPU::CPU[0]) CPU::CPU[0]->WantNMI = FALSE;
	if (CPU::CPU[1]) CPU::CPU[1]->WantNMI = FALSE;
	PPU::Reset();
	//if (PPU::PPU[0]) PPU::PPU[0]->RunAlign(1);
	if (NewPC) {
		if (CPU::CPU[0]) CPU::CPU[0]->PC =NewPC;
		if (CPU::CPU[1]) CPU::CPU[1]->PC =NewPC;
		NewPC =0;
	}
#ifdef	ENABLE_DEBUGGER
	Debugger::PalChanged = TRUE;
	Debugger::PatChanged = TRUE;
	Debugger::NTabChanged = TRUE;
	Debugger::SprChanged = TRUE;
	if (Debugger::Enabled)
		Debugger::Update(Debugger::Mode);
#endif	/* ENABLE_DEBUGGER */
	Scanline = FALSE;
	coin1 =coin2 =0;
	coinDelay1 =coinDelay2 =0;
	//EI.DbgOut(_T("Reset complete."));
}

DWORD	WINAPI	Thread (void *param) {

	if (!DoStop && Settings::SoundEnabled) Sound::SoundON();	// don't turn on sound if we're only stepping 1 instruction

	// clear "soft start", unless we only started in order to wait until vblank
	if (!(DoStop & STOPMODE_WAIT)) DoStop &= ~STOPMODE_SOFT;

	if (PPU::PPU[0] && PPU::PPU[0]->SLnum == 240) {
		if (FrameStep) {	// if we save or load while paused, we want to end up here
			// so we don't end up advancing another frame
			GotStep = FALSE;
			Movie::ShowFrame();
			while (FrameStep && !GotStep && !DoStop)
				Sleep(1);
		}
		// if savestate was triggered, stop before we even started
		if (DoStop & STOPMODE_WAIT)
		{
			DoStop &= ~STOPMODE_WAIT;
			DoStop |= STOPMODE_NOW;
		}
	}

	while (!(DoStop & STOPMODE_NOW))
	{
#ifdef	ENABLE_DEBUGGER
		if (Debugger::Enabled) 	Debugger::AddInst();
#endif	/* ENABLE_DEBUGGER */
		if (CPU::CPU[1]) {
			if (CPU::CPU[0]->CycleCount <= CPU::CPU[1]->CycleCount) {
				CPU::CPU[0]->ExecOp();
				#ifdef	ENABLE_DEBUGGER
				if (Debugger::Enabled && Debugger::which ==0 && !(DoStop & STOPMODE_WAIT)) Debugger::Update(DEBUG_MODE_CPU);
				#endif	/* ENABLE_DEBUGGER */
			} else {
				CPU::CPU[1]->ExecOp();
				#ifdef	ENABLE_DEBUGGER
				if (Debugger::Enabled && Debugger::which ==1 && !(DoStop & STOPMODE_WAIT)) Debugger::Update(DEBUG_MODE_CPU);
				#endif	/* ENABLE_DEBUGGER */
			}
		} else {
			CPU::CPU[0]->ExecOp();
			#ifdef	ENABLE_DEBUGGER
			if (Debugger::Enabled && !(DoStop & STOPMODE_WAIT)) Debugger::Update(DEBUG_MODE_CPU);
			#endif	/* ENABLE_DEBUGGER */
		}
		if (Scanline) {
			Scanline = FALSE;
			if (PPU::PPU[0] && PPU::PPU[0]->SLnum == 240) {
#ifdef	ENABLE_DEBUGGER
				if (Debugger::Enabled && !(DoStop & STOPMODE_WAIT))	// don't update debugger while in wait mode
					Debugger::Update(DEBUG_MODE_PPU);
#endif	/* ENABLE_DEBUGGER */
				if (FrameStep) { // if we pause during emulation
					// it'll get caught down here at scanline 240
					GotStep = FALSE;
					Movie::ShowFrame();
					while (FrameStep && !GotStep && !DoStop)
						Sleep(1);
				}
				// if savestate was triggered, stop here
				if (DoStop & STOPMODE_WAIT) {
					DoStop &= ~STOPMODE_WAIT;
					DoStop |= STOPMODE_NOW;
#ifdef	ENABLE_DEBUGGER
					// and immediately update the debugger if it's open
					if (Debugger::Enabled)
						Debugger::Update(Debugger::Mode);
#endif	/* ENABLE_DEBUGGER */
				}
			} else
			if (PPU::PPU[0] && PPU::PPU[0]->SLnum ==(RI.InputType ==INPUT_FOURSCORE? 10: PPU::PPU[0]->SLStartNMI)) {
				if (NES::GenericMulticart) Controllers::SwitchControllersAutomatically();
				Controllers::UpdateInput();
			}
		}
	}

	// special case - do not silence audio during soft stop
	if (!(DoStop & STOPMODE_SOFT))
		Sound::SoundOFF();
	Movie::ShowFrame();

	UpdateTitlebar();
	Running = FALSE;
	// If Stop was triggered from within the emulation thread
	// then signal the message loop in the main thread
	// to unacquire the controllers as soon as possible
	if (DoStop & STOPMODE_BREAK)
		PostMessage(hMainWnd, WM_USER, 0, 0);
	if (GFX::apertureChanged) {
		GFX::apertureChanged =false;
		GFX::Stop();	
		GFX::Start();
		UpdateInterface(); // Must come after GFX::Start()
		Start(FALSE);
	}
	return 0;
}
void	Start (BOOL step) {
	DWORD ThreadID;
	if (Running) return;
	Running = TRUE;
#ifdef	ENABLE_DEBUGGER
	Debugger::Step = step;
#endif	/* ENABLE_DEBUGGER */
	DoStop = 0;
	Controllers::Acquire();
	CloseHandle(CreateThread(NULL, 0, Thread, NULL, 0, &ThreadID));
}
void	Stop (void)
{
	if (!Running)
		return;
	DoStop = 1;

	while (Running)
	{
		ProcessMessages();
		Sleep(1);
	}
	Controllers::UnAcquire();
}

void	Pause (BOOL wait) {
	if (!Running) return;
	if (wait)
		DoStop = STOPMODE_SOFT | STOPMODE_WAIT;
	else
		DoStop = STOPMODE_SOFT | STOPMODE_NOW;

	while (Running)
	{
		ProcessMessages();
		Sleep(1);
	}
}
void	Resume (void) {
	DWORD ThreadID;
	if (Running)
		return;
	Running = TRUE;
#ifdef	ENABLE_DEBUGGER
	Debugger::Step = FALSE;
#endif	/* ENABLE_DEBUGGER */
	DoStop = STOPMODE_SOFT;
	CloseHandle(CreateThread(NULL, 0, Thread, NULL, 0, &ThreadID));
}
void	SkipToVBlank (void)
{
	DWORD ThreadID;
	if (Running)	// this should never happen
		return;
	Running = TRUE;
#ifdef	ENABLE_DEBUGGER
	Debugger::Step = FALSE;
#endif	/* ENABLE_DEBUGGER */
	DoStop = STOPMODE_SOFT | STOPMODE_WAIT;
	CloseHandle(CreateThread(NULL, 0, Thread, NULL, 0, &ThreadID));
	while (Running)
	{
		ProcessMessages();
		Sleep(1);
	}
}

void	MapperConfig (void)
{
	if ((MI) && (MI->Config))
		MI->Config(CFG_WINDOW, TRUE);
}

void	SetupDataPath (void)
{
	TCHAR path[MAX_PATH];

	// Create data subdirectories
	// Savestates
	_tcscpy(path, DataPath);
	PathAppend(path, _T("States"));
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(path, NULL);

	// SRAM data
	_tcscpy(path, DataPath);
	PathAppend(path, _T("SRAM"));
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(path, NULL);

	// FDS disk changes
	_tcscpy(path, DataPath);
	PathAppend(path, _T("FDS"));
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(path, NULL);

	// Debug dumps
	_tcscpy(path, DataPath);
	PathAppend(path, _T("Dumps"));
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(path, NULL);
}

// Stuff for testing CopyNES Plugins
std::vector<uint8_t>	fromCopyNEStoPC;
std::vector<uint8_t>	crcCopyNES;
int			batonCount;
char			namePlugin[128];

void getExponentDivisor(const size_t n, int& exponent, int& multiplier, int& divisor) {
	multiplier =1;
	if ((n %3) ==0) multiplier =3;
	if ((n %5) ==0) multiplier =5;
	if ((n %7) ==0) multiplier =7;
	exponent =log2 (n /multiplier);
	divisor =pow(2, exponent) *multiplier;
}

void	cbSendByte (void) {
	fromCopyNEStoPC.push_back(CPU::CPU[0]->A);
}
void	cbBaton (void) {
	EI.StatusOut(_T("CopyNES %S: %d"), namePlugin, ++batonCount);
}
void	cbChkVRAM (void) {
	CPU::CPU[0]->FZ =RI.CHRRAMSize? 1: 0;
}
void	cbChkWRAM (void) {
	CPU::CPU[0]->FZ =RI.PRGRAMSize? 1: 0;
}
void	cbWrPPU (void) {
	PPU::PPU[0]->VRAMAddr =CPU::CPU[0]->A <<8;
}
void	cbReadByte (void) {
	CPU::CPU[0]->A =0xFF; // Not implemented
}
void	cbInitCRC (void) {
	crcCopyNES.clear();
}
void	cbDoCRC (void) {
	crcCopyNES.push_back(CPU::CPU[0]->A);
}
void	cbFinishCRC (void) {
	uint32_t crc =Hash::CRC32C(0, &crcCopyNES[0], crcCopyNES.size());
	CPU::CPU[0]->RAM[0x080] =crc >> 0 &0xFF;
	CPU::CPU[0]->RAM[0x081] =crc >> 8 &0xFF;
	CPU::CPU[0]->RAM[0x082] =crc >>16 &0xFF;
	CPU::CPU[0]->RAM[0x083] =crc >>24 &0xFF;
}
void	cbDone (void) {
	CPU::callbacks.clear();
	
	std::vector<uint8_t> prgrom;
	std::vector<uint8_t> chrrom;	
	uint8_t mirroring =0;
	uint8_t state =4;
	uint8_t blockType =0xFF;
	size_t bytesLeft =0;
	for (auto& c: fromCopyNEStoPC) {
		switch(state) {
			case 0: bytesLeft =c <<8;
				state++;
				break;
			case 1:	bytesLeft|=c <<16;
				state++;
				break;
			case 2:	blockType =c;
				state++;
				if (blockType ==0) state =0;
				if (blockType >3) EI.DbgOut(L"Invalid block type %d", blockType);
				break;
			case 3:	switch (blockType) {
					case 1: prgrom.push_back(c); break;
					case 2: chrrom.push_back(c); break;
				}
				if (!--bytesLeft) state =0;
				break;
			case 4:	mirroring =c;
				state =0;
				break;
		}
		if (blockType ==0) break;
	}
	if (bytesLeft >0) EI.DbgOut(L"Unexpected end of CopyNES data, %d bytes left in block", bytesLeft);
	
	std::vector<uint8_t> result(16);
	result[0] ='N';
	result[1] ='E';
	result[2] ='S';
	result[3] =0x1A;
	result[6] =mirroring ==2? 0x08: mirroring &1;
	result[6]|=RI.INES_MapperNum <<4 &0xF0;
	result[7] =RI.INES_MapperNum &0xF0 |0x08;
	result[8] =RI.INES_MapperNum >>8 &0x0F | RI.INES2_SubMapper <<4;	

	if (prgrom.size() %16384 || prgrom.size() >=64*1024*1024) {
		int exponent, multiplier, divisor;
		getExponentDivisor(prgrom.size(), exponent, multiplier, divisor);
		result[9] |=0x0F;
		result[4] =(exponent <<2) | ((multiplier -1) >>1);
	} else {
		result[4]  = (prgrom.size() /16384) &0xFF;
		result[9] |=((prgrom.size() /16384) >>8) &0x0F;
	}
	if (chrrom.size() %8192 || chrrom.size() >=32*1024*1024) {
		int exponent, multiplier, divisor;
		getExponentDivisor(chrrom.size(), exponent, multiplier, divisor);
		result[9] |=0xF0;
		result[5] =(exponent <<2) | ((multiplier -1) >>1);
	} else {
		result[5]  = (chrrom.size() /8192) &0xFF;
		result[9] |=((chrrom.size() /8192) >>8) <<4;
	}
	
	FILE *handle =fopen("D:\\copynes.bin", "wb");
	if (handle) {
		fwrite(&fromCopyNEStoPC[0], 1, fromCopyNEStoPC.size(), handle);
		fclose(handle);
	}
	handle =fopen("D:\\copynes.nes", "wb");
	if (handle) {
		fwrite(&result[0], 1, result.size(), handle);
		fwrite(&prgrom[0], 1, prgrom.size(), handle);
		fwrite(&chrrom[0], 1, chrrom.size(), handle);
		fclose(handle);
	}
	MessageBox(hMainWnd, _T("Done!"), _T("CopyNES"), MB_OK);
}

void	runCopyNES (TCHAR *filename) {
	FILE *handle =_tfopen(filename, _T("rb"));
	if (handle) {
		fread(namePlugin, 1, 128, handle);
		fread(&CPU::CPU[0]->RAM[0x400], 1, 1024, handle);
		fclose(handle);

		EI.StatusOut(_T("CopyNES %S: started"), namePlugin);
		batonCount =0;

		fromCopyNEStoPC.clear();
		crcCopyNES.clear();
		CPU::callbacks.clear();
		CPU::callbacks.push_back({0, 0x0200, cbSendByte});
		CPU::callbacks.push_back({0, 0x0203, cbBaton});
		CPU::callbacks.push_back({0, 0x0206, cbChkVRAM});
		CPU::callbacks.push_back({0, 0x0209, cbChkWRAM});
		CPU::callbacks.push_back({0, 0x020C, cbWrPPU});
		CPU::callbacks.push_back({0, 0x020F, cbReadByte});
		CPU::callbacks.push_back({0, 0x0212, cbInitCRC});
		CPU::callbacks.push_back({0, 0x0215, cbDoCRC});
		CPU::callbacks.push_back({0, 0x0218, cbFinishCRC});
		CPU::callbacks.push_back({0, CPU::CPU[0]->PC, cbDone});
		CPU::CPU[0]->RAM[0x200] =0x60;
		CPU::CPU[0]->RAM[0x203] =0x60;
		CPU::CPU[0]->RAM[0x206] =0x60;
		CPU::CPU[0]->RAM[0x209] =0x60;
		CPU::CPU[0]->RAM[0x20C] =0x60;
		CPU::CPU[0]->RAM[0x20F] =0x60;
		CPU::CPU[0]->RAM[0x212] =0x60;
		CPU::CPU[0]->RAM[0x215] =0x60;
		CPU::CPU[0]->RAM[0x218] =0x60;

		// Assume a JSR to 400
		CPU::CPU[0]->PC--;
		CPU::CPU[0]->Push(CPU::CPU[0]->PCH);
		CPU::CPU[0]->Push(CPU::CPU[0]->PCL);
		CPU::CPU[0]->PC =0x3FF;
		CPU::CPU[0]->RAM[0x3FF] =0x78;
	}
}

void	loadMemoryDump(TCHAR *filename)  {
	bool gotHeader =false;
	uint8_t chunkHeader[8];
	uint32_t saveType;
	
	FILE* handle =_tfopen(filename, L"rb");
	if (handle) {
		while (1) {
			auto bytesRead =fread(chunkHeader, 1, 8, handle);
			if (bytesRead !=8) break;
			uint32_t chunkSize =chunkHeader[4] <<0 | chunkHeader[5] <<8 | chunkHeader[6] <<16 | chunkHeader[7] <<24;
			std::vector<uint8_t> chunkData(chunkSize);
			fread(&chunkData[0], 1, chunkSize, handle);
			
			if (!gotHeader) {
				if (memcmp(chunkHeader, "FDAT", 4)) { MessageBox(hMainWnd, _T("Not a true FDAT file."), _T("Load Memory Dump"), MB_OK | MB_ICONERROR); break; }
				saveType =chunkData[4] <<0 | chunkData[5] <<8 | chunkData[6] <<16 | chunkData[7] <<24;
				gotHeader =true;
				continue;
			} else
			if (!memcmp(chunkHeader, "CPUD", 4)) {
				int32_t loadSize =chunkSize -2;
				uint16_t loadAddress =chunkData[0] | chunkData[1] <<8;
				EI.DbgOut(L"Loading %d bytes to $%04X", loadSize, loadAddress);
				for (int i =0; i <loadSize; i++) {
					(EI.GetCPUWriteHandler(loadAddress >>12))(loadAddress >>12, loadAddress &0xFFF, chunkData[2 +i]);
					loadAddress++;
				}
			} else
			if (!memcmp(chunkHeader, "PPUD", 4)) {
				int32_t loadSize =chunkSize -2;
				uint16_t loadAddress =chunkData[0] | chunkData[1] <<8;
				for (int i =0; i <loadSize; i++) {
					if (loadAddress >=0x3F00)
						PPU::PPU[0]->Palette[loadAddress &0x1F] =chunkData[2 +i];
					else
						(EI.GetPPUWriteHandler(loadAddress >>10))(loadAddress >>10, loadAddress &0x3FF, chunkData[2 +i]);
					loadAddress++;
				}
			} else
				EI.DbgOut(L"Unknown chunk: %S", &chunkHeader[0]);
		}
		fclose(handle);
	}
}

void	saveMemoryDump(TCHAR *filename)  {
	// What game are we dealing with anyway?
	struct CodeString {
		uint32_t saveType;
		uint16_t CompareAddress;
		size_t CompareSize;
		const char *CompareData;
	} codeStrings[] = {
		{  1, 0xE970, 12, "\xA0\x00\xC8\xAD\x17\x40\x29\x04\xF0\xF8\x98\x60" },                         // Sharp My Computer TV C1
		{  2, 0x86D1, 16, "\xA9\x04\x8D\x16\x40\xA9\x06\x8D\x16\x40\xAD\x17\x40\x29\x10\x60" },         // Family BASIC v1.0
		{  2, 0x86C4, 16, "\xA9\x04\x8D\x16\x40\xA9\x06\x8D\x16\x40\xAD\x17\x40\x29\x10\x60" },         // Family BASIC v2.0/v2.1
		{  3, 0x8845, 16, "\xA9\x04\x8D\x16\x40\xA9\x06\x8D\x16\x40\xAD\x17\x40\x29\x10\x60" },         // Family BASIC v3
		{  4, 0xC65C, 16, "\x8D\x16\x40\xC6\x07\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x0C\xD0\xFC" },         // Excitebike
		{  5, 0xB91C, 16, "\x8D\x16\x40\xC6\x01\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x02\xD0\xFC" },         // Wrecking Crew
		{  6, 0xAAF5, 16, "\x8D\x16\x40\xC6\x08\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x09\xD0\xFC" },         // Mach Rider (NTSC rev0/1)
		{  6, 0xAAFD, 16, "\x8D\x16\x40\xC6\x08\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x09\xD0\xFC" },         // Mach Rider (PAL)
		{  7, 0xE9B8, 18, "\xA9\x05\x8D\x16\x40\xEA\xEA\xEA\xEA\xEA\xEA\xA2\x00\xA9\x04\x8D\x16\x40" }, // Lode Runner (U.S. and Japan)
	};

	uint32_t saveType =0x00;
	for (auto& theString: codeStrings) {
		bool match =true;
		for (size_t i =0; i <theString.CompareSize; i++) {
			int Address =theString.CompareAddress +i;
			if ((theString.CompareData[i] &0xFF) != (EI.GetCPUReadHandlerDebug(Address >>12))(Address >>12, Address &0xFFF)) {
				match =false;
				break;
			}
		}
		if (match) {
			saveType =theString.saveType;
			break;
		}
	}

	// Determine save areas by save type
	struct SaveArea {
		uint8_t type;
		uint16_t address;
		uint16_t length;
	};
	std::vector<SaveArea> saveAreas;
	switch(saveType) {
		case 1: // Sharp My Computer TV C1
			saveAreas.push_back({ 0, 0x0400, 0x01FC}); // TV Note
			saveAreas.push_back({ 0, 0x0700, 0x00FC}); // TV Note
			break;
		case 2:{// Family BASIC v1/v2
			saveType =0x02;
			saveAreas.push_back({ 0, 0x703E, static_cast<uint16_t>((CPU::CPU[0]->RAM[0x0008] <<8 | CPU::CPU[0]->RAM[0x0007]) -0x703E) });
			saveAreas.push_back({ 0, 0x0007, 2});
			saveAreas.push_back({ 0, 0x001D, 6});
			saveAreas.push_back({ 0, 0x00AD, 4});
			bool nonzero =false;
			for (int i =0; i <0x3C0; i++) if (PPU::PPU[0]->VRAM[1][i] !=0x20) nonzero =true;
			if (nonzero) saveAreas.push_back({ 1, 0x2C00, 1024});
			break;
			}
		case 3:{// Family BASIC v3
			saveType =0x03;
			saveAreas.push_back({ 0, 0x6006, static_cast<uint16_t>((CPU::CPU[0]->RAM[0x0008] <<8 | CPU::CPU[0]->RAM[0x0007]) -0x6006) });
			saveAreas.push_back({ 0, 0x0007, 2});
			saveAreas.push_back({ 0, 0x001F, 2});
			bool nonzero =false;
			for (int i =0; i <0x3C0; i++) if (PPU::PPU[0]->VRAM[1][i] !=0x20) nonzero =true;
			if (nonzero) saveAreas.push_back({ 1, 0x2C00, 1024});
			break;
			}
		case 4: // Excitebike
			saveAreas.push_back({ 0, 0x0060, 0x0040});
			saveAreas.push_back({ 0, 0x06E0, 0x0100}); // Buffered at 05E0
			break;
		case 5: // Wrecking Crew
			saveAreas.push_back({ 0, 0x0300, 0x0040});
			saveAreas.push_back({ 0, 0x0600, 0x0100});
			break;
		case 6: // Mach Rider
			saveAreas.push_back({ 0, 0x0600, 0x0040});
			saveAreas.push_back({ 0, 0x0700, 0x0100});
			break;
		case 7: // Lode Runner
			saveAreas.push_back({ 0, 0x0388, 0x0278});
			break;
	}
	if (saveAreas.size()) {
		FILE* handle =_tfopen(filename, L"wb");
		if (handle) {
			uint8_t chunkHeader[8];
			uint8_t fdat[8];
			
			chunkHeader[0] ='F'; chunkHeader[1] ='D'; chunkHeader[2] ='A'; chunkHeader[3] ='T'; 
			chunkHeader[4] =8 >>0  &0xFF; chunkHeader[5] =8 >>8  &0xFF; chunkHeader[6] =8 >>16 &0xFF; chunkHeader[7] =8 >>24 &0xFF;
			fwrite(chunkHeader, 1, 8, handle);
			
			uint32_t version =0x00000100;
			fdat[0] =version >>0  &0xFF; fdat[1] =version >>8  &0xFF; fdat[2] =version >>16 &0xFF; fdat[3] =version >>24 &0xFF;
			fdat[4] =saveType >>0  &0xFF; fdat[5] =saveType >>8  &0xFF; fdat[6] =saveType >>16 &0xFF; fdat[7] =saveType >>24 &0xFF;
			fwrite(fdat, 1, 8, handle);
			
			for (auto& sa: saveAreas) {
				int chunkSize =sa.length +2;
				chunkHeader[0] =sa.type? 'P': 'C'; chunkHeader[1] ='P'; chunkHeader[2] ='U'; chunkHeader[3] ='D'; 
				chunkHeader[4] =chunkSize >>0  &0xFF; chunkHeader[5] =chunkSize >>8  &0xFF; chunkHeader[6] =chunkSize >>16 &0xFF; chunkHeader[7] =chunkSize >>24 &0xFF;
				fwrite(chunkHeader, 1, 8, handle);
				
				std::vector<uint8_t> chunkData;
				chunkData.push_back(sa.address >>0 &0xFF);
				chunkData.push_back(sa.address >>8 &0xFF);
				for (int i =0; i <sa.length; i++) {
					chunkData.push_back(sa.type? (EI.GetPPUReadHandlerDebug(sa.address >>10))(sa.address >>10, sa.address &0x3FF): (EI.GetCPUReadHandlerDebug(sa.address >>12))(sa.address >>12, sa.address &0xFFF));
					sa.address++;
				}
				fwrite(&chunkData[0], 1, chunkData.size(), handle);
			}
			fclose(handle);
		}
	} else
		MessageBox(hMainWnd, _T("No supported ROM loaded."), _T("Save Memory Dump"), MB_OK | MB_ICONERROR);
}

} // namespace NES