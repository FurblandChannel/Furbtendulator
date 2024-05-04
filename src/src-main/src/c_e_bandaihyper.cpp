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
struct ExpPort_BandaiHyperShot_State {
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char PosX;
	unsigned char PosY;
	unsigned char NewBits;
};
#include <poppack.h>
int	ExpPort_BandaiHyperShot::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_BandaiHyperShot::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_BandaiHyperShot::Frame (unsigned char mode) {
	int i;
	if (mode & MOV_PLAY) {
		State->NewBits = MovData[0];
		State->PosX = MovData[1];
		State->PosY = MovData[2];
	} else 	{
		State->NewBits = 0;
		for (i = 0; i < 8; i++) {
			if (IsPressed(Buttons[i]))
				State->NewBits |= 1 << i;
		}
		if (!EnableOpposites) {	// prevent simultaneously pressing left+right or up+down
			if ((State->NewBits & 0xC0) == 0xC0)
				State->NewBits &= 0x3F;
			if ((State->NewBits & 0x30) == 0x30)
				State->NewBits &= 0xCF;
		}
		POINT pos;
		GFX::GetCursorPos(&pos);
		if ((pos.x >= 0) && (pos.x <= 255) && (pos.y >= 0) && (pos.y <= 239)) {
			State->PosX = pos.x;
			State->PosY = pos.y;
			CurrentCursor =&CursorCross;
		} else {
			State->PosX = State->PosY = 255;
			CurrentCursor =&CursorNormal;
		}
	}
	if (mode & MOV_RECORD) {
		MovData[0] = State->NewBits;
		MovData[1] = State->PosX;
		MovData[2] = State->PosY;
	}
}
unsigned char	ExpPort_BandaiHyperShot::Read1 (void) {
	unsigned char result;
	if (State->Strobe) {
		State->Bits = State->NewBits;
		State->BitPtr = 0;
		result = (unsigned char)(State->Bits & 1);
	}
	else {
		if (State->BitPtr < 8)
			result = (unsigned char)(State->Bits >> State->BitPtr++) & 1;
		else	result = 1;
	}
	return result <<1;
}

static const unsigned char colorTrigger[0x40] = {
/*		0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F */
/* 0x */	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 1x */	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 2x */	1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
/* 3x */	1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0
};

unsigned char	ExpPort_BandaiHyperShot::Read2 (void) {
	int x = State->PosX, y = State->PosY, z = State->NewBits &1;
	int WhiteCount = 0;
	unsigned char Bits = 0x00;
	int X, Y;

	if (z)
		Bits |= 0x10;

	if ((x < 0) || (x >= 256) || (y < 0) || (y >= 240))
		return Bits | 0x08;

	if (PPU::PPU[0]->OnScreen)
		for (Y = y -8; Y < y +8; Y++)
		{
			if (Y < 0)
				Y = 0;
			if (Y < PPU::PPU[0]->SLnum - 32)
				continue;
			if (Y > PPU::PPU[0]->SLnum)
				break;
			for (X = x -8; X < x +8; X++)
			{
				if (X < 0)
					X = 0;
				if (X > 255)
					break;
				if ((Y == PPU::PPU[0]->SLnum) && (X >= PPU::PPU[0]->Clockticks))
					break;
				if (colorTrigger[PPU::PPU[0]->DrawArray[Y * 341 + X +16] &0x3F]) WhiteCount++;
			}
		}
	if (WhiteCount < 32)
		Bits |= 0x08;
	return Bits;
}
unsigned char	ExpPort_BandaiHyperShot::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_BandaiHyperShot::Write (unsigned char Val) {
	if (State->Strobe || Val & 1) {
		State->Strobe = Val & 1;
		State->Bits = State->NewBits;
		State->BitPtr = 0;
	}
}

INT_PTR	CALLBACK	ExpPort_BandaiHyperShot_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[8] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5,IDC_CONT_D6,IDC_CONT_D7};
	const int dlgButtons[8] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5,IDC_CONT_K6,IDC_CONT_K7};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	}
	else	Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 8, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}

void	ExpPort_BandaiHyperShot::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_BANDAIHYPERSHOT), hWnd, ExpPort_BandaiHyperShot_ConfigProc, (LPARAM)this);
}

void	ExpPort_BandaiHyperShot::SetMasks (void) {
}

ExpPort_BandaiHyperShot::~ExpPort_BandaiHyperShot (void) {
	delete State;
	delete[] MovData;
}

ExpPort_BandaiHyperShot::ExpPort_BandaiHyperShot (DWORD *buttons) {
	Type = EXP_BANDAIHYPERSHOT;
	NumButtons = 8;
	Buttons = buttons;
	State = new ExpPort_BandaiHyperShot_State;
	MovLen = 3;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->NewBits = 0;
}

void ExpPort_BandaiHyperShot::CPUCycle (void) { }
} // namespace Controllers
