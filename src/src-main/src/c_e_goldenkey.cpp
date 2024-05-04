#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_GoldenKeyboard_State
{
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[11];
	unsigned char Strobe;
};
#include <poppack.h>
int	ExpPort_GoldenKeyboard::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_GoldenKeyboard::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

#define DIK_BREAK		0x63
#define DIK_GRETURN		0x65

void	ExpPort_GoldenKeyboard::Frame (unsigned char mode)
{
	int row, col;
	if (mode & MOV_PLAY)
	{
		for (row = 0; row < 11; row++)
			State->Keys[row] = MovData[row];
	}
	else
	{
		const int keymap[11][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 0},
			{ DIK_2, DIK_W, DIK_S, DIK_X, DIK_3, DIK_E, DIK_D, DIK_C},
			{ DIK_4, DIK_R, DIK_F, DIK_V, DIK_5, DIK_T, DIK_G, DIK_B},
			{ DIK_6, DIK_Y, DIK_H, DIK_N, DIK_7, DIK_U, DIK_J, DIK_M},
			{ DIK_8, DIK_I, DIK_K, DIK_COMMA, DIK_9, DIK_O, DIK_L, DIK_PERIOD},
			{ DIK_0, DIK_P, DIK_SEMICOLON, DIK_SLASH, DIK_MINUS, DIK_LBRACKET, DIK_APOSTROPHE, DIK_BACKSLASH},
			{ DIK_EQUALS, DIK_RBRACKET, 0, 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, DIK_ESCAPE, 0, 0, 0},
			{ 0, 0, DIK_SPACE, 0, 0, 0, 0, 0},
			{ DIK_F1, DIK_F2, DIK_F3, DIK_F4, DIK_F5, DIK_F6, DIK_F7, DIK_F8},
			{ 0, 0, 0, 0, 0, DIK_LSHIFT, 0, 0},
		};

		for (row = 0; row < 11; row++) {
			State->Keys[row] = 0;
			for (col = 0; col < 8; col++)
				if (KeyState[keymap[row][col]] & 0x80)
					State->Keys[row] |= 1 << col;
		}
		// special cases
		if (KeyState[DIK_RSHIFT] & 0x80)
			State->Keys[7] |= 0x80;
		if (KeyState[DIK_RCONTROL] & 0x80)
			State->Keys[5] |= 0x80;
	}
	if (mode & MOV_RECORD)
	{
		for (row = 0; row < 11; row++)
			MovData[row] = State->Keys[row];
	}
}
unsigned char	ExpPort_GoldenKeyboard::Read1 (void)
{
	return 0;	// tape, not yet implemented
}
unsigned char	ExpPort_GoldenKeyboard::Read2 (void)
{
	unsigned char result = 0;
	if (State->Row < 11) {
		//EI.DbgOut(L"Column: %d", State->Column);
		if (State->Keys[State->Row] & (1 <<(7-State->Column))) result |=2;
		State->Column =(State->Column +1) &7;
	}
	return result;
}

unsigned char	ExpPort_GoldenKeyboard::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_GoldenKeyboard::Write (unsigned char Val) {
	if (~State->Strobe &2 && Val &2) State->Row =0;
	if ( State->Strobe &1 &&~Val &1) State->Column =0;
	if ( State->Strobe &4 &&~Val &4) State->Row =(State->Row +1) %11;
	//EI.DbgOut(L"Row: %d", State->Row);
	State->Strobe =Val;
}

void	ExpPort_GoldenKeyboard::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void	ExpPort_GoldenKeyboard::SetMasks (void) {
	MaskKeyboard = TRUE;
}

ExpPort_GoldenKeyboard::~ExpPort_GoldenKeyboard (void) {
	delete State;
	delete[] MovData;
}

ExpPort_GoldenKeyboard::ExpPort_GoldenKeyboard (DWORD *buttons) {
	Type = EXP_GOLDENKEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_GoldenKeyboard_State;
	MovLen = 11;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_GoldenKeyboard::CPUCycle (void) { }
} // namespace Controllers