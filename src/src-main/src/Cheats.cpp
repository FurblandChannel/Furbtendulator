#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "resource.h"
#include "Wordify.hpp"
#include "Cheats.hpp"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace Cheats {
Game	cheatDefinition;
bool	anyCheatsActive =false;

void CheatLocation::clear() {
	bitmask =0xFF;
	address =0;
	compare.clear();
	replace.clear();
}

CheatLocation::CheatLocation():
	bitmask(0xFF),
	address(0) {
}

void Cheat::clear() {
	enabled =false;
	name =L"";
	locations.clear();
}

void Game::clear() {
	fdsID =L"";
	crcs.clear();
	cheats.clear();
}

bool Game::hasCRC (uint32_t wantedCRC) {
	for (auto& crc: crcs) if (crc ==wantedCRC) return true;
	return false;
}

std::vector<Game> parseCheatInfo(const std::vector<wchar_t>& text) {
	std::vector<std::wstring> words =wordify(text);

	std::vector<Game> games;
	Game game;
	Cheat cheat;
	CheatLocation location;
	std::wstring level;
	std::wstring field;

	for (auto& word: words) {
		if (word ==L"game") {
			if (location.replace.size()) { cheat.locations.push_back(location); location.clear(); }
			if (cheat.locations.size()) { game.cheats.push_back(cheat); cheat.clear(); }
			if (game.fdsID != L"" || game.crcs.size()) { games.push_back(game); game.clear(); }
			level =word;
			field =L"";
		} else
		if (word ==L"cheat") {
			if (location.replace.size()) { cheat.locations.push_back(location); location.clear(); }
			if (cheat.locations.size()) { game.cheats.push_back(cheat); cheat.clear(); }
			level =word;
			field =L"";
		} else
		if (word ==L"bitmask" || word ==L"crc" || word ==L"fdsid" || word ==L"address" || word ==L"compare" || word ==L"replace" || word ==L"ignoreNMI") {
			field =word;
		} else
		if (field ==L"" && level ==L"cheat") {
			cheat.name =word;
		} else
		if (field !=L"") {
			if (field ==L"crc" && level ==L"game") {
				game.crcs.push_back(wcstoul(word.c_str(), NULL, 0));
			} else
			if (field ==L"fdsid" && level ==L"game") {
				game.fdsID =word;
				field =L"";
			} else
			if (field ==L"address" && level ==L"cheat") {
				if (location.replace.size()) { cheat.locations.push_back(location); location.clear(); }
				location.address =wcstoul(word.c_str(), NULL, 16);
				field =L"";
			} else
			if (field ==L"compare" && level ==L"cheat")
				location.compare.push_back(wcstoul(word.c_str(), NULL, 16));
			else
			if (field ==L"replace" && level ==L"cheat")
				location.replace.push_back(wcstoul(word.c_str(), NULL, 16));
			else
			if (field ==L"bitmask" && level ==L"cheat")
				location.bitmask =wcstoul(word.c_str(), NULL, 16) &0xFF;
		} else
			EI.DbgOut(L"cheats.cfg: Syntax error: %s", word.c_str());
	}
	if (location.replace.size()) { cheat.locations.push_back(location); location.clear(); }
	if (cheat.locations.size()) { game.cheats.push_back(cheat); cheat.clear(); }
	if (game.fdsID != L"" || game.crcs.size()) { games.push_back(game); game.clear(); }

	// Plausibility checks
	// Check for fdsID values that have been assigned to more than one game
	for (unsigned int i =0; i <games.size(); i++) {
		if (games[i].fdsID ==L"") continue;
		for (unsigned int j =i+1; j<games.size(); j++) {
			if (i !=j && games[i].fdsID ==games[j].fdsID) EI.DbgOut(L"cheat.cfg: fds id '%s' appears more than once", games[i].fdsID.c_str());
		}
	}
	// Check for CRC values that have been assigned to more than one game
	for (unsigned int i =0; i <games.size(); i++) {
		for (unsigned int j =i+1; j<games.size(); j++) {
			for (auto& c1: games[i].crcs)
				for (auto& c2: games[j].crcs)
					if (c1 ==c2) EI.DbgOut(L"cheat.cfg: CRC value 0x%08X occurs more than once", c1);
		}
	}
	return games;
}

INT_PTR CALLBACK cheatsWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)  {
	int wmId =LOWORD(wParam);
	//int wmEvent =HIWORD(wParam);
	HWND cheatList =GetDlgItem(hwnd, IDC_CHEAT_LIST);

	switch(uMsg) {
	case WM_INITDIALOG: {
		// Columns
		tagLVCOLUMNW column_name    ={ LVCF_FMT|LVCF_TEXT|LVCF_WIDTH, LVCFMT_LEFT,   285, L"Setting"};
		tagLVCOLUMNW column_address ={ LVCF_FMT|LVCF_TEXT|LVCF_WIDTH, LVCFMT_CENTER,  55, L"Address"};
		SendMessage(cheatList, LVM_INSERTCOLUMN, 0, (LPARAM) &column_name);
		SendMessage(cheatList, LVM_INSERTCOLUMN, 1, (LPARAM) &column_address);

		// View
		SendMessage(cheatList, LVM_SETVIEW, LV_VIEW_DETAILS, 0);
		SendMessage(cheatList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES,
		                                                     LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES)
		;
		for (int i =0; i <(int) cheatDefinition.cheats.size(); i++) {
			auto& cheat =cheatDefinition.cheats[i];
			wchar_t address[16];
			swprintf(address, L"%04X%s", cheat.locations.at(0).address, cheat.locations.size() >1? L"...": L"");
			tagLVITEMW lvName    ={ LVIF_TEXT, i, 0, 0, 0, const_cast<LPWSTR>(cheat.name.c_str())};
			tagLVITEMW lvAddress ={ LVIF_TEXT, i, 1, 0, 0, address };
			SendMessage(cheatList, LVM_INSERTITEM, 0, (LPARAM) &lvName);
			SendMessage(cheatList, LVM_SETITEM,    0, (LPARAM) &lvAddress);
			ListView_SetCheckState(cheatList, i, cheat.enabled);
		}
		return TRUE;
		break; }
	case WM_COMMAND:
		switch (wmId) {
		case ID_ENABLE_ALL:
			for (unsigned int i =0; i <(int) cheatDefinition.cheats.size(); i++) ListView_SetCheckState(cheatList, i, TRUE);
			return TRUE;
			break;
		case ID_DISABLE_ALL:
			for (unsigned int i =0; i <(int) cheatDefinition.cheats.size(); i++) ListView_SetCheckState(cheatList, i, FALSE);
			return TRUE;
			break;
		case IDCANCEL:
		case IDOK:
			anyCheatsActive =false;
			for (unsigned int i =0; i <(int) cheatDefinition.cheats.size(); i++) {
				cheatDefinition.cheats[i].enabled =!!ListView_GetCheckState(cheatList, i);
				if (cheatDefinition.cheats[i].enabled) anyCheatsActive =true;
			}
			EndDialog(hwnd, 0);
			return TRUE;
			break;
		}
	default:
		break;
	}
	return FALSE;
}

void	findCheats () {
	// Parse cheat definitions
	EnableMenuItem(hMenu, ID_CHEATS, MF_GRAYED);
	TCHAR cheatFileName[MAX_PATH];
	int i =GetModuleFileName(NULL, cheatFileName, MAX_PATH);
	if (i) {
		while (i) if (cheatFileName[--i] =='\\') break;
		cheatFileName[i++] ='\\';
		cheatFileName[i++] =0;
		_tcscat(cheatFileName, L"cheats.cfg");
		FILE *handle =_tfopen(cheatFileName, _T("rt,ccs=UTF-16LE"));
		if (handle) {
			std::vector<wchar_t> text;
			wint_t c;
			while (1) {
				c =fgetwc(handle);
				if (c ==WEOF) break;
				text.push_back(c);
			};
			std::vector<Cheats::Game> cheatDefinitions =Cheats::parseCheatInfo(text);
			for (auto& game: cheatDefinitions) if (game.hasCRC(RI.PRGROMCRC32)) {
				cheatDefinition =game;
				EnableMenuItem(hMenu, ID_CHEATS, MF_ENABLED);
				break;
			}
			fclose(handle);
		} else
			EI.DbgOut(L"Could not open cheats.cfg in emulator directory.");
	}
	anyCheatsActive =false;
}

void	findFDSCheats (std::wstring& fdsID) {
	EnableMenuItem(hMenu, ID_CHEATS, MF_GRAYED);
	TCHAR cheatFileName[MAX_PATH];
	int i =GetModuleFileName(NULL, cheatFileName, MAX_PATH);
	if (i) {
		while (i) if (cheatFileName[--i] =='\\') break;
		cheatFileName[i++] ='\\';
		cheatFileName[i++] =0;
		_tcscat(cheatFileName, L"cheats.cfg");
		FILE *handle =_tfopen(cheatFileName, _T("rt,ccs=UTF-16LE"));
		if (handle) {
			std::vector<wchar_t> text;
			wint_t c;
			while (1) {
				c =fgetwc(handle);
				if (c ==WEOF) break;
				text.push_back(c);
			};
			std::vector<Cheats::Game> cheatDefinitions =Cheats::parseCheatInfo(text);
			for (auto& game: cheatDefinitions) if (game.fdsID ==fdsID) {
				cheatDefinition =game;
				EnableMenuItem(hMenu, ID_CHEATS, MF_ENABLED);
				break;
			}
			fclose(handle);
		} else
			EI.DbgOut(L"Could not open cheats.cfg in emulator directory.");
	}
	anyCheatsActive =false;
	EI.DbgOut(L"Found %d cheats", cheatDefinition.cheats.size());
}

void	cheatsWindow (const HWND hWnd) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_CHEATS), hWnd, cheatsWindowProc);
}

uint8_t	readCPUMemory (unsigned int address, uint8_t originalValue) {
	if (anyCheatsActive) for (auto& cheat: cheatDefinition.cheats) {
		if (cheat.enabled) for (auto& item: cheat.locations) {
			if (address >= item.address && address < (item.address +item.replace.size())) {
				bool match =true;
				for (unsigned int i =0; i <item.compare.size(); i++) {
					int tmpaddr =item.address +i;
					if ((EI.GetCPUReadHandlerDebug(tmpaddr >>12))(tmpaddr >>12, tmpaddr &0xFFF) != item.compare[i]) {
						match =false;
						break;
					}
				}			
				if (match) originalValue = originalValue &~item.bitmask | item.replace[address -item.address] &item.bitmask;
			}
		}
	}
	return originalValue;
}
} // namespace Cheats
