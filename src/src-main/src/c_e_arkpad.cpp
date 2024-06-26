#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_ArkanoidPaddle_State
{
	unsigned char Bits;
	unsigned short Pos;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char Button;
	unsigned char NewBits;
};
#include <poppack.h>
int	ExpPort_ArkanoidPaddle::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_ArkanoidPaddle::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_ArkanoidPaddle::Frame (unsigned char mode)
{
	int x, i, bits;
	if (mode & MOV_PLAY)
	{
		State->Pos = MovData[0] | ((MovData[1] << 8) & 0x7F);
		State->Button = MovData[1] >> 7;
	}
	else
	{
		State->Button = IsPressed(Buttons[0]);
		State->Pos += GetDelta(Buttons[1]);
		// Arkanoid's expected range is 196-484 (code caps to 196-516)
		// Arkanoid 2 SP expected range is 156-452 (code caps to 156-372/420/452/484)
		// Arkanoid 2 VS expected range is 168-438
		if (State->Pos < 156)
			State->Pos = 156;
		if (State->Pos > 484)
			State->Pos = 484;
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = (unsigned char)(State->Pos & 0xFF);
		MovData[1] = (unsigned char)((State->Pos >> 8) | (State->Button << 7));
	}
	bits = 0;
	x = ~State->Pos;
	for (i = 0; i < 9; i++)
	{
		bits <<= 1;
		bits |= x & 1;
		x >>= 1;
	}
	State->NewBits = bits;
}
unsigned char	ExpPort_ArkanoidPaddle::Read1 (void)
{
	if (State->Button)
		return 0x02;
	else	return 0;
}
unsigned char	ExpPort_ArkanoidPaddle::Read2 (void)
{
	if (State->BitPtr < 8)
		return (unsigned char)(((State->Bits >> State->BitPtr++) & 1) << 1);
	else	return 0x02;

	}
unsigned char	ExpPort_ArkanoidPaddle::ReadIOP (uint8_t) {
	return 0;
}
void	ExpPort_ArkanoidPaddle::Write (unsigned char Val)
{
	if ((!State->Strobe) && (Val & 1))
	{
		State->Bits = State->NewBits;
		State->BitPtr = 0;
	}
	State->Strobe = Val & 1;
}
INT_PTR	CALLBACK	ExpPort_ArkanoidPaddle_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[2] = {IDC_CONT_D0,IDC_CONT_D1};
	const int dlgButtons[2] = {IDC_CONT_K0,IDC_CONT_K1};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 1, 1, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	ExpPort_ArkanoidPaddle::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_ARKANOIDPADDLE), hWnd, ExpPort_ArkanoidPaddle_ConfigProc, (LPARAM)this);
}
void	ExpPort_ArkanoidPaddle::SetMasks (void)
{
	MaskMouse = ((Buttons[1] >> 16) == 1);
}
ExpPort_ArkanoidPaddle::~ExpPort_ArkanoidPaddle (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_ArkanoidPaddle::ExpPort_ArkanoidPaddle (DWORD *buttons)
{
	Type = EXP_ARKANOIDPADDLE;
	NumButtons = 2;
	Buttons = buttons;
	State = new ExpPort_ArkanoidPaddle_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Pos = 340;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Button = 0;
	State->NewBits = 0;
}
void ExpPort_ArkanoidPaddle::CPUCycle (void) { }
} // namespace Controllers