#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "Tape.h"
#include "MapperInterface.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_Bit79Keyboard_State
{
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[10];
	unsigned char Strobe;
};
#include <poppack.h>
int	ExpPort_Bit79Keyboard::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_Bit79Keyboard::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

#define DIK_BREAK		0x63
#define DIK_GRETURN		0x65

void	ExpPort_Bit79Keyboard::Frame (unsigned char mode)
{
	int row, col;
	if (mode & MOV_PLAY)
	{
		for (row = 0; row < 10; row++)
			State->Keys[row] = MovData[row];
	}
	else
	{
		const int keymap[10][8] = {
			{ 0,            DIK_SPACE,    0,            DIK_UP,       DIK_LEFT,     0,            0,             0},
			{ DIK_COMMA,    DIK_PERIOD,   0,            0,            0,            DIK_SLASH,    0,             DIK_RSHIFT},
			{ DIK_M,        DIK_N,        DIK_X,        DIK_Z,        DIK_LSHIFT,   DIK_B,        DIK_V,         DIK_C},
			{ DIK_K,        DIK_L,        0,            0,            0,            DIK_SEMICOLON,DIK_APOSTROPHE,DIK_RETURN},
			{ DIK_J,        DIK_H,        DIK_S,        DIK_A,        DIK_LCONTROL, DIK_G,        DIK_F,         DIK_D},
			{ DIK_U,        DIK_Y,        DIK_W,        DIK_Q,        DIK_TAB,      DIK_T,        DIK_R,         DIK_E},
			{ DIK_I,        DIK_O,        0,            0,            0,            DIK_P,        DIK_LBRACKET,  DIK_RBRACKET},
			{ DIK_7,        DIK_6,        DIK_2,        DIK_1,        DIK_GRAVE,    DIK_5,        DIK_4,         DIK_3},
			{ DIK_8,        DIK_9,        0,            0,            0,            DIK_0,        DIK_MINUS,     DIK_EQUALS},
			{ DIK_BACKSPACE,DIK_BACKSLASH,0,            0,            0,            DIK_DOWN,     0,             DIK_RIGHT},
		};

		for (row = 0; row < 10; row++) {
			State->Keys[row] = 0;
			for (col = 0; col < 8; col++)
				if (KeyState[keymap[row][col]] & 0x80)
					State->Keys[row] |= 1 << col;
		}
		/*// special cases
		if (KeyState[DIK_RSHIFT] & 0x80)
			State->Keys[7] |= 0x80;
		if (KeyState[DIK_RCONTROL] & 0x80)
			State->Keys[5] |= 0x80;*/
	}
	if (mode & MOV_RECORD)
	{
		for (row = 0; row < 10; row++)
			MovData[row] = State->Keys[row];
	}
}
unsigned char	ExpPort_Bit79Keyboard::Read1 (void) {
	unsigned char result = 0;
	if (State->Row < 10) {
		if (State->Keys[State->Row] & (1 <<(7-State->Column))) result |=2;
		State->Column =(State->Column +1) &7;
	}
	result |= (Tape::Input() &1) <<2;
	return result;
}

unsigned char	ExpPort_Bit79Keyboard::Read2 (void) {
	return 0;
}

unsigned char	ExpPort_Bit79Keyboard::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_Bit79Keyboard::Write (unsigned char Val) {
	if (~State->Strobe &2 && Val &2) State->Row =0;
	if ( State->Strobe &1 &&~Val &1) State->Column =0;
	if ( State->Strobe &4 &&~Val &4) State->Row =(State->Row +1) %10;
	Tape::Output(!!(Val &2));
	State->Strobe =Val;
}

void	ExpPort_Bit79Keyboard::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void	ExpPort_Bit79Keyboard::SetMasks (void) {
	MaskKeyboard = TRUE;
}

ExpPort_Bit79Keyboard::~ExpPort_Bit79Keyboard (void) {
	delete State;
	delete[] MovData;
}

ExpPort_Bit79Keyboard::ExpPort_Bit79Keyboard (DWORD *buttons) {
	Type = EXP_BIT79KEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_Bit79Keyboard_State;
	MovLen = 10;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_Bit79Keyboard::CPUCycle (void) { 
	Tape::CPUCycle();
}
} // namespace Controllers