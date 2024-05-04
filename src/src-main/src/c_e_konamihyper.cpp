#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "PPU.h"
#include "GFX.h"

namespace Controllers {

#include <pshpack1.h>
struct ExpPort_KonamiHyperShot_State
{
	unsigned char Jump1;
	unsigned char Run1;
	unsigned char Jump2;
	unsigned char Run2;
	unsigned char Disabled;
};
#include <poppack.h>
int	ExpPort_KonamiHyperShot::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_KonamiHyperShot::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_KonamiHyperShot::Frame (unsigned char mode)
{
	if (mode & MOV_PLAY)
	{
		State->Jump1 = MovData[0];
		State->Run1 = MovData[1];
		State->Jump2 = MovData[2];
		State->Run2 = MovData[3];
	}
	else
	{
		State->Jump1 = IsPressed(Buttons[0]);
		State->Run1 = IsPressed(Buttons[1]);
		State->Jump2 = IsPressed(Buttons[2]);
		State->Run2 = IsPressed(Buttons[3]);
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = State->Jump1;
		MovData[1] = State->Run1;
		MovData[2] = State->Jump1;
		MovData[3] = State->Run1;
	}
}
unsigned char	ExpPort_KonamiHyperShot::Read1 (void)
{
	return 0;
}

unsigned char	ExpPort_KonamiHyperShot::Read2 (void) {
	int result =0;
	if (~State->Disabled &2) result |=(State->Run1? 0x02: 0x00) | (State->Jump1? 0x04: 0x00);
	if (~State->Disabled &4) result |=(State->Run2? 0x08: 0x00) | (State->Jump2? 0x10: 0x00);
	return result;
}

unsigned char	ExpPort_KonamiHyperShot::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_KonamiHyperShot::Write (unsigned char Val) {
	State->Disabled =Val;
}

INT_PTR	CALLBACK	ExpPort_KonamiHyperShot_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[4] = {IDC_CONT_D0, IDC_CONT_D1, IDC_CONT_D2, IDC_CONT_D3};
	const int dlgButtons[4] = {IDC_CONT_K0, IDC_CONT_K1, IDC_CONT_K2, IDC_CONT_K3};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 4, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	ExpPort_KonamiHyperShot::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_KONAMI_HYPER_SHOT), hWnd, ExpPort_KonamiHyperShot_ConfigProc, (LPARAM)this);
}
void	ExpPort_KonamiHyperShot::SetMasks (void)
{
}
ExpPort_KonamiHyperShot::~ExpPort_KonamiHyperShot (void) {
	delete State;
	delete[] MovData;
	CurrentCursor =&CursorNormal;
}
ExpPort_KonamiHyperShot::ExpPort_KonamiHyperShot (DWORD *buttons)
{
	Type = EXP_KONAMIHYPERSHOT;
	NumButtons = 4;
	Buttons = buttons;
	State = new ExpPort_KonamiHyperShot_State;
	MovLen = 4;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Jump1 =0;
	State->Run1 =0;
	State->Jump2 =0;
	State->Run2 =0;
	State->Disabled =0;
}
void ExpPort_KonamiHyperShot::CPUCycle (void) { }
} // namespace Controllers