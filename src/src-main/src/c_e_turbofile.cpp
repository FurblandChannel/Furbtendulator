#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "States.h"
#include <stdint.h>

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_TurboFile_State {
	uint8_t Memory[8192];
	int Bit;
};
#include <poppack.h>
int	ExpPort_TurboFile::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_TurboFile::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	ExpPort_TurboFile::Frame (unsigned char mode) {
}

unsigned char	ExpPort_TurboFile::Read1 (void) {
	return 0;
}

unsigned char	ExpPort_TurboFile::Read2 (void) {
	return (State->Memory[State->Bit >>3] &(0x80 >>(State->Bit &7)) )? 4: 0;
}

unsigned char	ExpPort_TurboFile::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_TurboFile::Write (unsigned char Val) {
	if (~Val &2) State->Bit =0;
	if (Val &4) {
		if (Val &1)
			State->Memory[State->Bit >>3] |= 0x80 >>(State->Bit &7);
		else
			State->Memory[State->Bit >>3] &=~0x80 >>(State->Bit &7);
	} else {
		State->Bit++;
		State->Bit &=0xFFFF;
	}
}

void	ExpPort_TurboFile::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void	ExpPort_TurboFile::SetMasks (void) {
}

ExpPort_TurboFile::~ExpPort_TurboFile (void) {
	TCHAR Filename[MAX_PATH];
	_stprintf(Filename, _T("%s\\SRAM\\ASCIITurboFile.sav"), DataPath);
	FILE *Handle = _tfopen(Filename, _T("wb"));
	if (Handle) {
		fwrite(State->Memory, 1, sizeof(State->Memory), Handle);
		fclose(Handle);
	}
	delete State;
}

ExpPort_TurboFile::ExpPort_TurboFile (DWORD *buttons) {
	Type = EXP_TURBOFILE;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_TurboFile_State;
	MovLen = 0;
	MovData = NULL;
	State->Bit = 0;
	ZeroMemory(State->Memory, sizeof(State->Memory));
	
	TCHAR Filename[MAX_PATH];
	_stprintf(Filename, _T("%s\\SRAM\\ASCIITurboFile.sav"), DataPath);
	FILE *Handle = _tfopen(Filename, _T("rb"));
	if (Handle) {
		fread(State->Memory, 1, sizeof(State->Memory), Handle);
		fclose(Handle);
		
	}
}
void ExpPort_TurboFile::CPUCycle (void) { 
}
} // namespace Controllers