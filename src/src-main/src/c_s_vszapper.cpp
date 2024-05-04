#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "PPU.h"
#include "GFX.h"

namespace Controllers
{
#include <pshpack1.h>
struct StdPort_VSZapper_State
{
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char PosX;
	unsigned char PosY;
	unsigned char Button;
	unsigned char Button2;
};
#include <poppack.h>
int	StdPort_VSZapper::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	StdPort_VSZapper::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	StdPort_VSZapper::Frame (unsigned char mode)
{
	POINT pos;
	if (mode & MOV_PLAY)
	{
		State->PosX = MovData[0];
		State->PosY = MovData[1];
		State->Button = MovData[2];
		State->Button2 = MovData[3];
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
		State->Button = IsPressed(Buttons[0]);
		State->Button2 = IsPressed(Buttons[1]);
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = State->PosX;
		MovData[1] = State->PosY;
		MovData[2] = State->Button;
		MovData[3] = State->Button2;
	}
}
unsigned char	StdPort_VSZapper::Read (void)
{
	unsigned char result;
	if (State->Strobe)
	{
		State->BitPtr = 0;
		result = (unsigned char)(State->Bits & 1);
	}
	else
	{
		if (State->BitPtr < 8)
			result = (unsigned char)(State->Bits >> State->BitPtr++) & 1;
		else	result = 0;
	}
	return result;
}
void	StdPort_VSZapper::Write (unsigned char Val)
{
	int x = State->PosX, y = State->PosY;

	State->Strobe = Val & 1;
	if (!State->Strobe)
		return;
		
	State->Bits = 0x10;
	State->BitPtr = 0;
	if (State->Button || State->Button2)
		State->Bits |= 0x80;

	if ((x < 0) || (x >= 256) || (y < 0) || (y >= 240) || State->Button2)
		return;

	if (PPU::PPU[0]->IsRendering && PPU::PPU[0]->OnScreen)
	{
		int X, Y;
		int WhiteCount = 0;
		for (Y = y - 8; Y < y + 8; Y++)
		{
			if (Y < 0)
				Y = 0;
			if (Y < PPU::PPU[0]->SLnum - 32)
				continue;
			if (Y > PPU::PPU[0]->SLnum)
				break;
			for (X = x - 16; X < x + 16; X++)
			{
				if (X < 0)
					X = 0;
				if (X > 255)
					break;
				if ((Y == PPU::PPU[0]->SLnum) && (X >= PPU::PPU[0]->Clockticks))
					break;
				if (GFX::ZapperHit(PPU::PPU[0]->DrawArray[Y * 341 + X +16]))
					WhiteCount++;
			}
		}
		if (WhiteCount >= 64)
			State->Bits |= 0x40;
	}
}
INT_PTR	CALLBACK	StdPort_VSZapper_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[2] = {IDC_CONT_D0, IDC_CONT_D1};
	const int dlgButtons[2] = {IDC_CONT_K0, IDC_CONT_K1};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	}
	else	Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 2, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	StdPort_VSZapper::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_VSZAPPER), hWnd, StdPort_VSZapper_ConfigProc, (LPARAM)this);
}
void	StdPort_VSZapper::SetMasks (void)
{
}
StdPort_VSZapper::~StdPort_VSZapper (void)
{
	delete State;
	delete[] MovData;
	CurrentCursor =&CursorNormal;
}
StdPort_VSZapper::StdPort_VSZapper (DWORD *buttons)
{
	Type = STD_VSZAPPER;
	NumButtons = 2;
	Buttons = buttons;
	State = new StdPort_VSZapper_State;
	MovLen = 4;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->PosX = 0;
	State->PosY = 0;
	State->Button = 0;
	State->Button2 = 0;
	State->Bits = 0x10;
	State->BitPtr = 0;
	State->Strobe = 0;
	GFX::SetFrameskip(-2);
}
} // namespace Controllers