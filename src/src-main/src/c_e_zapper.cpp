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
struct ExpPort_Zapper_State
{
	unsigned char PosX;
	unsigned char PosY;
	unsigned char Trigger;
	unsigned char Reload;
	int ReloadCount;
};
#include <poppack.h>
int	ExpPort_Zapper::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_Zapper::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_Zapper::Frame (unsigned char mode)
{
	POINT pos;
	if (mode & MOV_PLAY)
	{
		State->PosX = MovData[0];
		State->PosY = MovData[1];
		State->Trigger = MovData[2];
		State->Reload = MovData[3];
		GFX::SetCursorPos(State->PosX, State->PosY);
	}
	else
	{
		GFX::GetCursorPos(&pos);
		if ((pos.x >= 0) && (pos.x <= 255) && (pos.y >= 0) && (pos.y <= 239)) {
			State->PosX = pos.x;
			State->PosY = pos.y;
			CurrentCursor =&CursorCross;
		} else {
			State->PosX = State->PosY = 255;
			CurrentCursor =&CursorNormal;
		}
		State->Trigger = IsPressed(Buttons[0]);
		State->Reload = IsPressed(Buttons[1]);
		if (State->ReloadCount) State->ReloadCount--;
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = State->PosX;
		MovData[1] = State->PosY;
		MovData[2] = State->Trigger;
		MovData[3] = State->Reload;
	}
}
unsigned char	ExpPort_Zapper::Read1 (void)
{
	return 0;
}

static const unsigned char colorTrigger[0x40] = {
/*		0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F */
/* 0x */	0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
/* 1x */	1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
/* 2x */	1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
/* 3x */	1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0
};

#define S_WHITE 0x00
#define S_BLACK 0x08
#define S_RELEASED 0x00
#define S_PULLED 0x10
unsigned char	ExpPort_Zapper::Read2 (void) {
	/* Duck Hunt and Crime Busters check whether Trigger was *released*, then whether White was detected.
	   For "Reload" to work properly, this means that White must not be detected at least a few frames *after* reload was released. */
	if (State->Reload) {
		State->ReloadCount =30;
		return S_BLACK | S_PULLED;
	} else if (State->Trigger)
		State->ReloadCount =0;
	
	int x = State->PosX, y = State->PosY;
	if ((x <0) || (x >=256) || (y <0) || (y >=240)) return S_BLACK | (State->Trigger? S_PULLED: S_RELEASED);

	int WhiteCount =0;
	if (State->ReloadCount ==0 && PPU::PPU[0]->OnScreen) for (int Y =y -8; Y <y +8; Y++) {
			if (Y <0) Y =0;
			if (Y <PPU::PPU[0]->SLnum -32) continue;
			if (Y >PPU::PPU[0]->SLnum) break;
			for (int X =x -16; X <x +16; X++) {
				if (X <0) X =0;
				if (X >255) break;
				if ((Y == PPU::PPU[0]->SLnum) && (X >= PPU::PPU[0]->Clockticks)) break;
				if (colorTrigger[PPU::PPU[0]->DrawArray[Y * 341 + X +16] &0x3F]) WhiteCount++;
			}
	}
	return ((WhiteCount <32)? S_BLACK: S_WHITE) | (State->Trigger? S_PULLED: S_RELEASED);
}

unsigned char	ExpPort_Zapper::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_Zapper::Write (unsigned char Val) {
}

INT_PTR	CALLBACK	ExpPort_Zapper_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[2] = {IDC_CONT_D0, IDC_CONT_D1};
	const int dlgButtons[2] = {IDC_CONT_K0, IDC_CONT_K1};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 2, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	ExpPort_Zapper::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_ZAPPER), hWnd, ExpPort_Zapper_ConfigProc, (LPARAM)this);
}
void	ExpPort_Zapper::SetMasks (void)
{
}
ExpPort_Zapper::~ExpPort_Zapper (void) {
	delete State;
	delete[] MovData;
	CurrentCursor =&CursorNormal;
}
ExpPort_Zapper::ExpPort_Zapper (DWORD *buttons)
{
	Type = EXP_ZAPPER;
	NumButtons = 2;
	Buttons = buttons;
	State = new ExpPort_Zapper_State;
	MovLen = 4;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->PosX = 0;
	State->PosY = 0;
	State->Trigger = 0;
	State->Reload = 0;
	GFX::SetFrameskip(-2);	
}
void ExpPort_Zapper::CPUCycle (void) { }
} // namespace Controllers