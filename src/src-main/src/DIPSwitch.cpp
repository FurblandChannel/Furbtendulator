#include "stdafx.h"
#include <time.h>
#include <windowsx.h>
#include <vector>
#include <algorithm>
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "Wordify.hpp"
#include "DIPSwitch.h"

namespace DIP {
void Choice::clear() {
	name =L"";
	value =0;
}

void Setting::clear() {
	name =L"";
	mask =0;
	defaultValue =0;
	choices.clear();
}

bool Game::hasCRC (uint32_t wantedCRC) {
	for (auto& crc: crcs) if (crc ==wantedCRC) return true;
	return false;
}

uint32_t Game::getDefaultValue() {
	uint32_t result =0;
	for (auto& setting: settings) result |=setting.defaultValue &setting.mask;
	return result;
}

std::vector<Game> parseDIP(const std::vector<wchar_t>& text) {
	std::vector<std::wstring> words =wordify(text);
	
	std::vector<Game> games;
	Choice choice;
	Setting setting;
	Game game;
	std::wstring level;
	std::wstring field;
	
	for (auto& word: words) {
		if (word ==L"game") {
			if (choice.name !=L"") { setting.choices.push_back(choice); choice.clear(); }
			if (setting.name !=L"") { game.settings.push_back(setting); setting.clear(); }
			if (game.name !=L"") { games.push_back(game); game.clear(); }
			level =word;
		} else
		if (word ==L"setting") {
			if (choice.name !=L"") { setting.choices.push_back(choice); choice.clear(); }
			if (setting.name !=L"") { game.settings.push_back(setting); setting.clear(); }
			level =word;
		} else
		if (word ==L"choice") {
			if (choice.name !=L"") { setting.choices.push_back(choice); choice.clear(); }
			level =word;
		} else
		if ((word ==L"value" || word ==L"crc" || word ==L"mask" || word ==L"default" || word == L"name") && field ==L"") {
			field =word;
		} else
		if (field !=L"") {
			if (field ==L"value" && level ==L"choice") 
				choice.value =wcstoul(word.c_str(), NULL, 0);
			else
			if (field ==L"default" && level ==L"setting") 
				setting.defaultValue =wcstoul(word.c_str(), NULL, 0);
			else
			if (field ==L"mask" && level ==L"setting") 
				setting.mask =wcstol(word.c_str(), NULL, 0);
			else
			if (field ==L"crc" && level ==L"game") {
				game.crcs.push_back(wcstoul(word.c_str(), NULL, 0));
			} else
			if (field ==L"name") {
				if (level ==L"choice") choice.name =word; else
				if (level ==L"setting") setting.name =word; else
				if (level ==L"game") game.name =word;
			}

			field =L"";
		} else {
			EI.DbgOut(L"dip.cfg: Syntax error: %s (game was \"%s\")", word.c_str(), game.name.c_str());
			exit(1);
		}
	}
	if (choice.name !=L"") setting.choices.push_back(choice);
	if (setting.name !=L"") game.settings.push_back(setting);
	if (game.name !=L"") games.push_back(game);
	
	// Plausibility checks
	// Check for CRC values that have been assigned to more than one game
	for (unsigned int i =0; i <games.size(); i++) {
		for (unsigned int j =i+1; j<games.size(); j++) {
			for (auto& c1: games[i].crcs)
				for (auto& c2: games[j].crcs)
					if (c1 ==c2) EI.DbgOut(L"Games '%s' and '%s' both have CRC value 0x%08X", games[i].name.c_str(), games[j].name.c_str(), c1);
		}
	}
	for (auto& g: games) {
		// Every game must have at least one CRC value
		if (g.crcs.size() ==0) EI.DbgOut(L"Game '%s' has no CRC values", g.name.c_str());
		
		// No CRC value should occur more than once for the same game (not a big problem if it does though)
		for (unsigned int i =0; i <g.crcs.size(); i++) {
			for (unsigned j =i+1; j <g.crcs.size(); j++)
				if (g.crcs[i] ==g.crcs[j]) EI.DbgOut(L"Game '%s' CRC value 0x%08X occurs more than once", g.name.c_str(), g.crcs[i], g.crcs[j]);
		}
		
		for (auto& s: g.settings) {
			// The setting must have a mask specified and its value must not be zero
			if (s.mask ==0) EI.DbgOut(L"Game '%s' setting '%s': no or zero-value mask", g.name.c_str(), s.name.c_str());
			// The default value must match at least one of the choices
			uint32_t defaultValue =s.defaultValue;
			auto match =std::find_if(s.choices.begin(), s.choices.end(), [&defaultValue](const Choice& p) { return p.value ==defaultValue; });
			if (match ==s.choices.end()) EI.DbgOut(L"Game '%s' setting '%s': default value 0x%X not among the choices", g.name.c_str(), s.name.c_str(), defaultValue);
			
			// The default value, and the value of all choices, must not have bits outside the mask
			if (s.defaultValue & ~s.mask) EI.DbgOut(L"Game '%s' setting '%s': default value has bits outside the mask", g.name.c_str(), s.name.c_str());
			for (auto &p: s.choices)
				if (p.value & ~s.mask) EI.DbgOut(L"Game '%s' setting '%s' choice '%s': value has bits outside the mask", g.name.c_str(), s.name.c_str(), p.name.c_str());
		}
	}
	return games;
}

class DialogTemplate {
	std::vector<BYTE> v;
public:
	LPCDLGTEMPLATE Template () {
		return (LPCDLGTEMPLATE)&v[0];
	}
	void	alignToDword () {
		if (v.size() % 4) write(NULL, 4 - (v.size() % 4));
	}
	void	write (LPCVOID pvWrite, DWORD cbWrite) {
			v.insert(v.end(), cbWrite, 0);
			if (pvWrite) CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
		}
	template<typename T> void write (T t) {
		write(&t, sizeof(T));
	}
	void	writeString (LPCWSTR psz) {
		write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR));
	}
};

#define IDD_DIP_SETTING	50000
#define IDD_DIP_DEFAULT	59999
const Game*	theGame;
uint32_t*	theValue;
uint32_t	prevValue;

INT_PTR CALLBACK dipWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)  {
	unsigned int i, j;
	unsigned int msgAddString =theGame->settings.size() ==1? LB_ADDSTRING: CB_ADDSTRING;
	unsigned int msgSetCurSel =theGame->settings.size() ==1? LB_SETCURSEL: CB_SETCURSEL;
	unsigned int msgGetCurSel =theGame->settings.size() ==1? LB_GETCURSEL: CB_GETCURSEL;
	uint32_t commandID =GET_WM_COMMAND_ID(wParam, lParam);
	
	switch(msg) {
	case WM_HOTKEY:
		if (wParam ==0 && GetForegroundWindow() ==hwnd) return dipWindowProc(hwnd, WM_COMMAND, IDCANCEL, 0); // 'D' is like Cancel
		break;
	case WM_INITDIALOG: {
		SendMessage(hwnd, DM_SETDEFID, IDOK, 0);

		for (i =0; i <theGame->settings.size(); i++) {
			auto& setting =theGame->settings[i];
			HWND box =GetDlgItem(hwnd, IDD_DIP_SETTING +i);
			
			for (j =0; j <setting.choices.size(); j++) {
				auto& choice =setting.choices[j];
				SendMessage(box, (UINT) msgAddString, (WPARAM) 0, (LPARAM) choice.name.c_str()); 
				if ((*theValue &setting.mask) ==choice.value) SendMessage(box, msgSetCurSel, (WPARAM) j, (LPARAM) 0);
			}
		}
		RegisterHotKey(hwnd, 0, 0, 'D'); // 'D' is like Cancel
		return TRUE;
	}
	case WM_COMMAND:
		// When double-clicking on a choice and there is only one setting, treat this as "OK"
		if (HIWORD(wParam) ==LBN_DBLCLK && commandID >=IDD_DIP_SETTING && commandID <IDD_DIP_DEFAULT && theGame->settings.size() ==1)
			return dipWindowProc(hwnd, WM_COMMAND, IDOK, 0);
		else
		switch (commandID) {
		case IDD_DIP_DEFAULT:
			for (i =0; i <theGame->settings.size(); i++) {
				auto& setting =theGame->settings[i];
				HWND box =GetDlgItem(hwnd, IDD_DIP_SETTING +i);
				
				*theValue =(*theValue &~setting.mask) | (setting.defaultValue &setting.mask);
				for (j =0; j <setting.choices.size(); j++) {
					auto& choice =setting.choices[j];
					if ((*theValue &setting.mask) ==choice.value) SendMessage(box, msgSetCurSel, (WPARAM) j, (LPARAM) 0);
				}
			}
			break;
		case IDOK:
			for (i =0; i <theGame->settings.size(); i++) {
				auto& setting =theGame->settings[i];
				HWND box =GetDlgItem(hwnd, IDD_DIP_SETTING +i);
				
				j =SendMessage(box, (UINT) msgGetCurSel, (WPARAM) 0, (LPARAM) 0);
				if (j !=(unsigned) CB_ERR) *theValue =(*theValue &~setting.mask) | (setting.choices[j].value &setting.mask);	
			}
			UnregisterHotKey(hwnd, 0);
			EndDialog(hwnd, 0);
			break;
		case IDCANCEL:
			*theValue =prevValue;
			UnregisterHotKey(hwnd, 0);
			EndDialog(hwnd, 0);
			break;
		}
		break;
	}
	return FALSE;
}

unsigned int stringWidth(const HDC& hDC, const std::wstring& string) {
	SIZE size;
	GetTextExtentPoint32(hDC, string.c_str(), string.size(), &size);
	return size.cx +1;
}

unsigned int stringHeight(const HDC& hDC, const std::wstring& string) {
	SIZE size;
	GetTextExtentPoint32(hDC, string.c_str(), string.size(), &size);
	return size.cy;
}

bool	windowDIP(const HWND hWnd, const Game& game, uint32_t& value) {
	theGame =&game;
	theValue =&value;
	prevValue =value;
	
	// Determine the widest setting and choice names, and calculate the correct window and element dimensions from that
	const HDC hDC =GetDC(NULL);
	HFONT hFont =CreateFont(11, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"MS Shell Dlg");
	HGDIOBJ previousFont =GetCurrentObject(hDC, OBJ_FONT);
	SelectFont(hDC, hFont);	
	unsigned int widestChoice =0;
	unsigned int widestSetting =0;	
	for (auto& setting: theGame->settings) {
		if (stringWidth(hDC, setting.name) >widestSetting) widestSetting =stringWidth(hDC, setting.name);
		for (auto& choice: setting.choices) if (stringWidth(hDC, choice.name) >widestChoice) widestChoice =stringWidth(hDC, choice.name);
	}
	static constexpr unsigned int distanceX =5;
	static constexpr unsigned int distanceY =5;
	static constexpr unsigned int arrowWidth =24;
	unsigned int buttonSizeX  =stringWidth(hDC, L"Default") *2;
	unsigned int buttonSizeY =stringHeight(hDC, L"Default") *3/2;
	
	unsigned int windowSizeX, windowSizeY, elementSizeY;
	if (game.settings.size() ==1) {	// Just one setting: show all choices in one listbox
		elementSizeY =stringHeight(hDC, L"Default");
		windowSizeX =(widestSetting +widestChoice) +3*distanceX;
		windowSizeY =theGame->settings[0].choices.size()*elementSizeY +buttonSizeY +distanceY*3;
	} else {			// Otherwise use combobox
		elementSizeY =stringHeight(hDC, L"Default") *3/2;
		windowSizeX =(widestSetting +widestChoice +arrowWidth) +3*distanceX;
		windowSizeY =theGame->settings.size()*elementSizeY +buttonSizeY +distanceY*3;
	}
	// If the strings were too narrow to accomodate three buttons, widen the box to that miminum amount
	if (windowSizeX <buttonSizeX*3 +distanceX *4) windowSizeX =buttonSizeX*3 +distanceX *4;
		
	// Initialize the dialog template
	DialogTemplate tmp;
	tmp.write<WORD>(1);		// Dialog version
	tmp.write<WORD>(0xFFFF);	// Extended dialog template
	tmp.write<DWORD>(0); 		// Help ID
	tmp.write<DWORD>(0); 		// Extended style
	tmp.write<DWORD>(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME);
	tmp.write<WORD>(game.settings.size()*2 +3); // # of controls: number of settings in this game, one static text and one combobox per game, plus three buttons
	tmp.write<WORD>(32);		// X
	tmp.write<WORD>(32);		// Y
	tmp.write<WORD>(windowSizeX);
	tmp.write<WORD>(windowSizeY);
	tmp.writeString(L"");		// no menu
	tmp.writeString(L"");		// default dialog class
	tmp.writeString(game.name.c_str()); // Title
	tmp.write<WORD>(11); 		// Font Points
	tmp.write<WORD>(400);		// Font Weight
	tmp.write<BYTE>(0);		// Font Italic
	tmp.write<BYTE>(0x1);		// Font Charset
	tmp.writeString(L"MS Shell Dlg");
	
	for (unsigned int i =0; i <game.settings.size(); i++) {
		auto& setting =game.settings[i];
		
		// Setting name as static text
		tmp.alignToDword();
		tmp.write<DWORD>(0);		 	// Help id
		tmp.write<DWORD>(0);			// Window extended style
		tmp.write<DWORD>(WS_CHILD | WS_VISIBLE);
		tmp.write<WORD>(distanceX); 					// X
		tmp.write<WORD>(distanceY +i*elementSizeY +elementSizeY/5);	// Y
		tmp.write<WORD>(widestSetting);		// As wide as the longest setting name requies
		tmp.write<WORD>(elementSizeY);		// As high as the highest setting name requires
		tmp.write<DWORD>(-1);			// Control ID, none for static text
		tmp.write<DWORD>(0x0082FFFF);		// Indicate "static text" control
		tmp.writeString(setting.name.c_str());	// Text
		tmp.write<WORD>(0);			// No Extra Data
		
		// Combo box for choices
		if (game.settings.size() ==1) {
			tmp.alignToDword();
			tmp.write<DWORD>(0);		 	// Help id
			tmp.write<DWORD>(0);			// Window extended style
			tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | LBS_HASSTRINGS | WS_TABSTOP | WS_BORDER | LBS_NOTIFY);
			tmp.write<WORD>(windowSizeX -widestChoice -distanceX); 		// X
			tmp.write<WORD>(distanceY +i*elementSizeY); 			// Y
			tmp.write<WORD>(widestChoice);					// As wide as the longest choice name requies
			tmp.write<WORD>(setting.choices.size() *elementSizeY);		// As many choices as there are
			tmp.write<DWORD>(IDD_DIP_SETTING +i);
			tmp.write<DWORD>(0x0083FFFF);		// Indicate listbox control
			tmp.writeString(L"");			// No text for combo boes
			tmp.write<WORD>(0);			// No Extra Data
		} else {
			tmp.alignToDword();
			tmp.write<DWORD>(0);		 	// Help id
			tmp.write<DWORD>(0);			// Window extended style
			tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_TABSTOP);
			tmp.write<WORD>(windowSizeX -widestChoice -arrowWidth -distanceX); // X
			tmp.write<WORD>(distanceY +i*elementSizeY); 		// Y
			tmp.write<WORD>(widestChoice +arrowWidth +1);		// As wide as the longest choice name requies
			tmp.write<WORD>(setting.choices.size() *elementSizeY*2);// As many choices as there are
			tmp.write<DWORD>(IDD_DIP_SETTING +i);
			tmp.write<DWORD>(0x0085FFFF);		// Indicate combobox control
			tmp.writeString(L"");			// No text for combo boes
			tmp.write<WORD>(0);			// No Extra Data
		}
	}
	// "OK" Button
	tmp.alignToDword();
	tmp.write<DWORD>(0);		 	// Help id
	tmp.write<DWORD>(0);			// Window extended style
	tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP);
	tmp.write<WORD>(distanceX);					// X: Left
	tmp.write<WORD>(windowSizeY -distanceY -buttonSizeY);		// Y: Bottom
	tmp.write<WORD>(buttonSizeX);
	tmp.write<WORD>(buttonSizeY);
	tmp.write<DWORD>(IDOK);
	tmp.write<DWORD>(0x0080FFFF);		// Indicate "button" control
	tmp.writeString(L"OK");
	tmp.write<WORD>(0);			// No Extra Data
	
	// "Cancel" Button
	tmp.alignToDword();
	tmp.write<DWORD>(0);		 	// Help id
	tmp.write<DWORD>(0);			// Window extended style
	tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | WS_TABSTOP);
	tmp.write<WORD>((windowSizeX -buttonSizeX)/2);			// X: Center
	tmp.write<WORD>(windowSizeY -distanceY -buttonSizeY);		// Y: Bottom
	tmp.write<WORD>(buttonSizeX);
	tmp.write<WORD>(buttonSizeY);
	tmp.write<DWORD>(IDCANCEL);
	tmp.write<DWORD>(0x0080FFFF);		// Indicate "button" control
	tmp.writeString(L"Cancel");
	tmp.write<WORD>(0);			// No Extra Data

	// "Default" Button
	tmp.alignToDword();
	tmp.write<DWORD>(0);		 	// Help id
	tmp.write<DWORD>(0);			// Window extended style
	tmp.write<DWORD>(WS_CHILD | WS_VISIBLE | WS_TABSTOP);
	tmp.write<WORD>(windowSizeX -buttonSizeX -distanceX);		// X: Right
	tmp.write<WORD>(windowSizeY -distanceY -buttonSizeY);		// Y: Bottom
	tmp.write<WORD>(buttonSizeX);
	tmp.write<WORD>(buttonSizeY);
	tmp.write<DWORD>(IDD_DIP_DEFAULT);
	tmp.write<DWORD>(0x0080FFFF);		// Indicate "button" control
	tmp.writeString(L"Default");
	tmp.write<WORD>(0);			// No Extra Data
	
	DialogBoxIndirect(GetModuleHandle(NULL), tmp.Template(), hWnd, dipWindowProc);
	
	SelectFont(hDC, previousFont);
	DeleteObject(hFont);
	ReleaseDC(NULL, hDC); 
	return value !=prevValue;
}
} // namespace DIP