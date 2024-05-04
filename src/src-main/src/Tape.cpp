#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "Tape.h"
#include "MapperInterface.h"
#include "resource.h"

#define STATE_IDLE 0
#define STATE_PLAYBACK 1
#define STATE_RECORDING 2

#include "Tape_helper.hpp"
#include "Filter.h"

namespace Tape {
std::vector<uint8_t>	Data;
uint32_t		Bits =0, Position =0;
FILE*			Handle =NULL;
int			State =STATE_IDLE;
bool			Level;
int                     fileFormat;

void	Play (void) {
	TCHAR FileName[MAX_PATH] = {0};
	OPENFILENAME ofn;
	FileName[0] = 0;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter =_T("Tape File (*.TAP)\0") _T("*.tap\0")
	                 _T("Wave File (*.WAV)\0") _T("*.wav\0")
	                 _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_AVI;
	ofn.lpstrTitle = _T("Load Tape Image");
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (!GetOpenFileName(&ofn)) return;
	_tcscpy(Settings::Path_AVI, FileName);
	Settings::Path_AVI[ofn.nFileOffset-1] = 0;
	
	fileFormat =ofn.nFilterIndex;
	Stop();
	_wfopen_s(&Handle, FileName, L"rb");
	if (Handle) {
		fseek(Handle, 0, SEEK_END);
		size_t fileSize =ftell(Handle);
		Data.resize(fileSize);
		fseek(Handle, 0, SEEK_SET);
		fread(&Data[0], 1, fileSize, Handle);
		Bits =fileSize <<3;
		State =STATE_PLAYBACK;
		EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_MISC_STOPTAPE, MF_ENABLED);
	}
}

void	Record (void) {
	TCHAR FileName[MAX_PATH] = {0};
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter =_T("Tape File (*.TAP)\0") _T("*.tap\0")
	                 _T("Wave File (*.WAV)\0") _T("*.wav\0")
	                 _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_AVI;
	ofn.lpstrTitle = _T("Record Tape Image");
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("TAP");
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (!GetSaveFileName(&ofn)) return;
	_tcscpy(Settings::Path_AVI, FileName);
	Settings::Path_AVI[ofn.nFileOffset-1] = 0;
	fileFormat =ofn.nFilterIndex;

	Stop();
	_wfopen_s(&Handle, FileName, L"wb");
	if (Handle) {
		State =STATE_RECORDING;
		EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_GRAYED);
		EnableMenuItem(hMenu, ID_MISC_STOPTAPE, MF_ENABLED);
	}
}

void	Stop (void) {
	if (Handle) {
		if (State ==STATE_RECORDING) {
			switch (fileFormat) {
				case 1: // .TAP
					fwrite(&Data[0], 1, Data.size(), Handle);
					break;
				case 2: {// .WAV
					Filter::Butterworth filterPCM(20, 1789773, 4000.0);
					uint32_t count =0;
					// Convert bitfield to samples
					std::vector<uint8_t> samples;
					for (auto byte: Data) {
						for (int bit =0; bit <8; bit++) {
							count +=176;
							float filteredOutput =filterPCM.output(byte &0x80? 0.50: -0.50);
							if (count >=39375) {
								count -=39375;
								samples.push_back((filteredOutput *127.0) -0x80);
							}
							byte <<=1;
						}
					}
					
					std::vector<uint8_t> waveFile =makeWAVEFile8Bit(samples, 8000, 1);
					fwrite(&waveFile[0], 1, waveFile.size(), Handle);
				}
			}
			
		}
		fclose(Handle);
		Handle =NULL;
	}
	Data.clear();
	Bits =Position =0;
	Level =false;
	State =STATE_IDLE;
	EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_MISC_STOPTAPE, MF_GRAYED);
}

void	CPUCycle (void) {
	switch(State) {
		case STATE_RECORDING:
			if ((Bits &7) ==0) Data.push_back(0);
			if (Level)
				Data[Position >>3] |=  0x80 >> (Position &7);
			else
				Data[Position >>3] &=~(0x80 >> (Position &7));
			Position++;
			Bits++;
			break;
		case STATE_PLAYBACK:
			if (Position < Bits) {
				Level = (Data[Position >>3] & (0x80 >> (Position &7)))? true: false;
			} else
				Stop();
			Position++;
			break;
	}
}

void	Output (bool _Level) {
	if (State ==STATE_RECORDING) (EI.GetCPUWriteHandler(4))(4, 0x11, _Level? 1: 0);
	Level =_Level;
}

bool	Input (void) {
	if (State ==STATE_PLAYBACK) (EI.GetCPUWriteHandler(4))(4, 0x11, Level? 1: 0);
	return Level;
}

}