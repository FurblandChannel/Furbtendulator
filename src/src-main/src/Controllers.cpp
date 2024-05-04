#include "stdafx.h"
#include "Settings.h"
#include "Nintendulator.h"
#include "resource.h"
#include "MapperInterface.h"
#include "Movie.h"
#include "Controllers.h"
#include "Tape.h"
#include "NES.h"
#include <commdlg.h>
#include <list>
#include "HeaderEdit.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Controllers {
bool	capsLock;
bool	scrollLock;
HWND	key;

StdPort *Port1, *Port2;
StdPort *FSPort1, *FSPort2, *FSPort3, *FSPort4;
ExpPort *PortExp;

DWORD	Port1_Buttons[CONTROLLERS_MAXBUTTONS],
	Port2_Buttons[CONTROLLERS_MAXBUTTONS];
DWORD	FSPort1_Buttons[CONTROLLERS_MAXBUTTONS],
	FSPort2_Buttons[CONTROLLERS_MAXBUTTONS],
	FSPort3_Buttons[CONTROLLERS_MAXBUTTONS],
	FSPort4_Buttons[CONTROLLERS_MAXBUTTONS];
DWORD	PortExp_Buttons[CONTROLLERS_MAXBUTTONS];

BOOL	EnableOpposites;

int	NumDevices;
BOOL	DeviceUsed[MAX_CONTROLLERS];
TCHAR	*DeviceName[MAX_CONTROLLERS];

int	NumButtons[MAX_CONTROLLERS];
BYTE	AxisFlags[MAX_CONTROLLERS],
	POVFlags[MAX_CONTROLLERS];

TCHAR	*ButtonNames[MAX_CONTROLLERS][128],
	*AxisNames[MAX_CONTROLLERS][8],
	*POVNames[MAX_CONTROLLERS][4],
	*KeyNames[256];

LPDIRECTINPUT8		DirectInput;
LPDIRECTINPUTDEVICE8	DIDevices[MAX_CONTROLLERS];
GUID		DeviceGUID[MAX_CONTROLLERS];

BYTE		KeyState[256];
DIMOUSESTATE2	MouseState;
DIJOYSTATE2	JoyState[MAX_CONTROLLERS];	// first 2 entries are unused

void	StdPort_SetControllerType (StdPort *&Port, STDCONT_TYPE Type, DWORD *buttons) {
	HKEY SettingsBase;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Nintendulator\\"), 0, KEY_ALL_ACCESS, &SettingsBase))
		RegCreateKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Nintendulator\\"), 0, _T("NintendulatorClass"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &SettingsBase, NULL);
	SaveButtons(SettingsBase);

	if (Port != NULL)
	{
		delete Port;
		Port = NULL;
	}
	switch (Type)
	{
	case STD_UNCONNECTED:		Port = new StdPort_Unconnected(buttons);	break;
	case STD_STDCONTROLLER:		Port = new StdPort_StdController(buttons);	break;
	case STD_ZAPPER:		Port = new StdPort_Zapper(buttons);		break;
	case STD_ARKANOIDPADDLE:	Port = new StdPort_ArkanoidPaddle(buttons);	break;
	case STD_POWERPADA:		Port = new StdPort_PowerPadA(buttons);		break;
	case STD_POWERPADB:		Port = new StdPort_PowerPadB(buttons);		break;
	case STD_FOURSCORE:		Port = new StdPort_FourScore(buttons);		break;
	case STD_SNESCONTROLLER:	Port = new StdPort_SnesController(buttons);	break;
	case STD_VSZAPPER:		Port = new StdPort_VSZapper(buttons);		break;
	case STD_SNESMOUSE:		Port = new StdPort_SnesMouse(buttons);		break;
	case STD_SUBORMOUSE:		Port = new StdPort_SuborMouse(buttons);		break;
	case STD_FOURSCORE2:		Port = new StdPort_FourScore2(buttons);		break;
	case STD_SUDOKU_EXCALIBUR:      Port = new StdPort_SudokuExcalibur(buttons);	break;
	case STD_SUDOKU_EXCALIBUR2:     Port = new StdPort_SudokuExcalibur2(buttons);	break;
	default:MessageBox(hMainWnd, _T("Error: selected invalid controller type for standard port!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);	break;
	}
	LoadButtons(SettingsBase);
	if (Type ==STD_SUDOKU_EXCALIBUR2 && Port ==Port2) {
		for (int i =0; i <CONTROLLERS_MAXBUTTONS; i++) Port2_Buttons[i] =Port1_Buttons[i];
	}
}
const TCHAR	*StdPort_Mappings[STD_MAX];
void	StdPort_SetMappings (void)
{
	StdPort_Mappings[STD_UNCONNECTED] = _T("Unconnected");
	StdPort_Mappings[STD_STDCONTROLLER] = _T("Standard Controller");
	StdPort_Mappings[STD_ZAPPER] = _T("Zapper");
	StdPort_Mappings[STD_ARKANOIDPADDLE] = _T("Arkanoid Paddle");
	StdPort_Mappings[STD_POWERPADA] = _T("Power Pad Side A");
	StdPort_Mappings[STD_POWERPADB] = _T("Power Pad Side B");
	StdPort_Mappings[STD_FOURSCORE] = _T("Four Score (port 1 only)");
	StdPort_Mappings[STD_SNESCONTROLLER] = _T("SNES Controller");
	StdPort_Mappings[STD_VSZAPPER] = _T("VS Unisystem Zapper");
	StdPort_Mappings[STD_SNESMOUSE] = _T("SNES Mouse");
	StdPort_Mappings[STD_SUBORMOUSE] = _T("Subor Mouse");
	StdPort_Mappings[STD_FOURSCORE2] = _T("Four Score (port 2 only)");
	StdPort_Mappings[STD_SUDOKU_EXCALIBUR] = _T("Sudoku (Excalibur)(port 1 only)");
	StdPort_Mappings[STD_SUDOKU_EXCALIBUR2] = _T("Sudoku (Excalibur)(port 2 only)");
}

void    ExpPort_SetControllerType (ExpPort *&Port, EXPCONT_TYPE Type, DWORD *buttons)
{
	HKEY SettingsBase;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Nintendulator\\"), 0, KEY_ALL_ACCESS, &SettingsBase))
		RegCreateKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Nintendulator\\"), 0, _T("NintendulatorClass"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &SettingsBase, NULL);
	SaveButtons(SettingsBase);

	if (Port != NULL)
	{
		delete Port;
		Port = NULL;
	}
        switch (Type)
        {
        case EXP_UNCONNECTED:		Port = new ExpPort_Unconnected(buttons);		break;
        case EXP_FAMI4PLAY:		Port = new ExpPort_Fami4Play(buttons);			break;
        case EXP_ARKANOIDPADDLE:	Port = new ExpPort_ArkanoidPaddle(buttons);		break;
        case EXP_FAMILYBASICKEYBOARD:	Port = new ExpPort_FamilyBasicKeyboard(buttons);	break;
        case EXP_SUBORKEYBOARD:		Port = new ExpPort_SuborKeyboard(buttons);		break;
        case EXP_FAMTRAINERA:		Port = new ExpPort_FamTrainerA(buttons);		break;
	case EXP_FAMTRAINERB:		Port = new ExpPort_FamTrainerB(buttons);		break;
        case EXP_TABLET:		Port = new ExpPort_Tablet(buttons);			break;
	case EXP_ZAPPER:		Port = new ExpPort_Zapper(buttons);			break;
	case EXP_BANDAIHYPERSHOT:	Port = new ExpPort_BandaiHyperShot(buttons);		break;
	case EXP_PEC586KEYBOARD:	Port = new ExpPort_PEC586Keyboard(buttons);		break;
	case EXP_BIT79KEYBOARD:		Port = new ExpPort_Bit79Keyboard(buttons);		break;
	case EXP_CITYPATROLMAN:		Port = new ExpPort_CityPatrolman(buttons);		break;
	case EXP_TURBOFILE:		Port = new ExpPort_TurboFile(buttons);			break;
	case EXP_MOGURAA:  		Port = new ExpPort_Moguraa(buttons);			break;
	case EXP_KONAMIHYPERSHOT:	Port = new ExpPort_KonamiHyperShot(buttons);		break;
	case EXP_SHARPC1CASSETTE:	Port = new ExpPort_SharpC1Cassette(buttons);		break;
	case EXP_GOLDENNUGGETCASINO:	Port = new ExpPort_GoldenNuggetCasino(buttons);		break;
	case EXP_GOLDENKEYBOARD:	Port = new ExpPort_GoldenKeyboard(buttons);		break;
	case EXP_HORI4PLAY:		Port = new ExpPort_Hori4Play(buttons);			break;
        default:MessageBox(hMainWnd, _T("Error: selected invalid controller type for expansion port!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);	break;
        }
	LoadButtons(SettingsBase);
}
const TCHAR	*ExpPort_Mappings[EXP_MAX];
void	ExpPort_SetMappings (void)
{
	ExpPort_Mappings[EXP_UNCONNECTED] = _T("Unconnected");
	ExpPort_Mappings[EXP_FAMI4PLAY] = _T("Famicom 4-Player Adapter");
	ExpPort_Mappings[EXP_ARKANOIDPADDLE] = _T("Famicom Arkanoid Paddle");
	ExpPort_Mappings[EXP_FAMILYBASICKEYBOARD] = _T("Family Basic Keyboard");
	ExpPort_Mappings[EXP_SUBORKEYBOARD] = _T("Subor Keyboard");
	ExpPort_Mappings[EXP_FAMTRAINERA] = _T("Family Trainer Side A");
	ExpPort_Mappings[EXP_FAMTRAINERB] = _T("Family Trainer Side B");
	ExpPort_Mappings[EXP_TABLET] = _T("Oeka Kids Tablet");
	ExpPort_Mappings[EXP_ZAPPER] = _T("Zapper");
	ExpPort_Mappings[EXP_BANDAIHYPERSHOT] = _T("Bandai Hyper Shot");
	ExpPort_Mappings[EXP_PEC586KEYBOARD] = _T("PEC-586 Keyboard");
	ExpPort_Mappings[EXP_TURBOFILE] = _T("ASCII Turbo File");
	ExpPort_Mappings[EXP_BIT79KEYBOARD] = _T("Bit-79 Keyboard");
	ExpPort_Mappings[EXP_CITYPATROLMAN] = _T("City Patrolman Lightgun");
	ExpPort_Mappings[EXP_MOGURAA] = _T("Pokkun Moguraa Mat");
	ExpPort_Mappings[EXP_KONAMIHYPERSHOT] = _T("Konami Hyper Shot");
	ExpPort_Mappings[EXP_SHARPC1CASSETTE] = _T("Sharp C1 Cassette Interface");
	ExpPort_Mappings[EXP_GOLDENNUGGETCASINO] = _T("Majesco Golden Nugget Casino");
	ExpPort_Mappings[EXP_GOLDENKEYBOARD] = _T("Golden Keyboard");
	ExpPort_Mappings[EXP_HORI4PLAY] = _T("Hori 4-Player Adapter");
}

BOOL	POVAxis = TRUE;

INT_PTR	CALLBACK	ControllerProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	int i;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < STD_MAX; i++)
		{
			SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[i]);
			SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[i]);
		}
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, Port1->Type, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, Port2->Type, 0);

		SendDlgItemMessage(hDlg, IDC_CONT_SEXPPORT, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < EXP_MAX; i++)
			SendDlgItemMessage(hDlg, IDC_CONT_SEXPPORT, CB_ADDSTRING, 0, (LPARAM)ExpPort_Mappings[i]);
		SendDlgItemMessage(hDlg, IDC_CONT_SEXPPORT, CB_SETCURSEL, PortExp->Type, 0);
		if (Movie::Mode)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT1), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT2), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SEXPPORT), FALSE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT1), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT2), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SEXPPORT), TRUE);
		}

		if (EnableOpposites)
			CheckDlgButton(hDlg, IDC_CONT_UDLR, BST_CHECKED);
		else	CheckDlgButton(hDlg, IDC_CONT_UDLR, BST_UNCHECKED);

		return TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_CONT_POV:
			POVAxis = !(IsDlgButtonChecked(hDlg, IDC_CONT_POV) == BST_CHECKED);
			return TRUE;
		case IDOK:
			EnableOpposites = (IsDlgButtonChecked(hDlg, IDC_CONT_UDLR) == BST_CHECKED);
			POVAxis = TRUE;
			EndDialog(hDlg, 1);
			return TRUE;
		case IDC_CONT_SPORT1:
			if (wmEvent == CBN_SELCHANGE)
			{
				STDCONT_TYPE Type = (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_GETCURSEL, 0, 0);
				if (Type ==STD_FOURSCORE2 || Type ==STD_SUDOKU_EXCALIBUR2) {
					// undo selection - can NOT set port 1 to this!
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, Port1->Type, 0);
				} else
				if (Type ==STD_FOURSCORE && Port1->Type !=STD_FOURSCORE) {
					STDCONT_TYPE Prev1 = Port1->Type;
					STDCONT_TYPE Prev2 = Port2->Type;
					SET_STDCONT(Port1, STD_FOURSCORE);
					SET_STDCONT(Port2, STD_FOURSCORE2);
					SET_STDCONT(FSPort1, Prev1);
					SET_STDCONT(FSPort2, Prev2);
					SET_STDCONT(FSPort3, STD_STDCONTROLLER);
					SET_STDCONT(FSPort4, STD_STDCONTROLLER);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, STD_FOURSCORE, 0);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, STD_FOURSCORE2, 0);
				} else
				if (Type ==STD_SUDOKU_EXCALIBUR && Port1->Type !=STD_SUDOKU_EXCALIBUR) {
					SET_STDCONT(Port1, STD_SUDOKU_EXCALIBUR);
					SET_STDCONT(Port2, STD_SUDOKU_EXCALIBUR2);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, STD_SUDOKU_EXCALIBUR, 0);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, STD_SUDOKU_EXCALIBUR2, 0);
				} else
					SET_STDCONT(Port1, Type);
			}
			return TRUE;
		case IDC_CONT_SPORT2:
			if (wmEvent == CBN_SELCHANGE)
			{
				STDCONT_TYPE Type = (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_GETCURSEL, 0, 0);
				if (Type == STD_FOURSCORE || Type ==STD_SUDOKU_EXCALIBUR) {
					// undo selection - can NOT set port 2 to this!
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, Port2->Type, 0);
				} else
				if (Type == STD_SUDOKU_EXCALIBUR2 && Port2->Type !=STD_SUDOKU_EXCALIBUR2) {
					SET_STDCONT(Port1, STD_SUDOKU_EXCALIBUR);
					SET_STDCONT(Port2, STD_SUDOKU_EXCALIBUR2);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, STD_SUDOKU_EXCALIBUR, 0);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, STD_SUDOKU_EXCALIBUR2, 0);
				} else
					SET_STDCONT(Port2, Type);
			}
			return TRUE;
		case IDC_CONT_SEXPPORT:
			if (wmEvent == CBN_SELCHANGE) {
				EXPCONT_TYPE Type = (EXPCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SEXPPORT, CB_GETCURSEL, 0, 0);
				SET_EXPCONT(PortExp, Type);
				if (Type ==EXP_FAMI4PLAY) {
					SET_STDCONT(FSPort3, STD_STDCONTROLLER);
					SET_STDCONT(FSPort4, STD_STDCONTROLLER);
				} else
				if (Type ==EXP_HORI4PLAY) {
					STDCONT_TYPE Prev1 = Port1->Type;
					STDCONT_TYPE Prev2 = Port2->Type;
					SET_STDCONT(Port1, STD_UNCONNECTED);
					SET_STDCONT(Port2, STD_UNCONNECTED);
					SET_STDCONT(FSPort1, Prev1);
					SET_STDCONT(FSPort2, Prev2);
					SET_STDCONT(FSPort3, STD_STDCONTROLLER);
					SET_STDCONT(FSPort4, STD_STDCONTROLLER);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, STD_UNCONNECTED, 0);
					SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, STD_UNCONNECTED, 0);
				}
			}
			return TRUE;
		case IDC_CONT_CPORT1:
			Port1->Config(hDlg);
			return TRUE;
		case IDC_CONT_CPORT2:
			Port2->Config(hDlg);
			return TRUE;
		case IDC_CONT_CEXPPORT:
			PortExp->Config(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
void	OpenConfig (void)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_CONTROLLERS), hMainWnd, ControllerProc);
	SetDeviceUsed();
}

BOOL CALLBACK	EnumKeyboardObjectsCallback (LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	int ItemNum = 0;
	if (IsEqualGUID(lpddoi->guidType, GUID_Key))
	{
		ItemNum = lpddoi->dwOfs;
		if ((ItemNum >= 0) && (ItemNum < 256))
			KeyNames[ItemNum] = _tcsdup(lpddoi->tszName);
		else	MessageBox(hMainWnd, _T("Error - encountered invalid keyboard key ID!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
	}
	return DIENUM_CONTINUE;
}

BOOL CALLBACK	EnumMouseObjectsCallback (LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	size_t ItemNum = 0;
	if (IsEqualGUID(lpddoi->guidType, GUID_XAxis))
	{
		AxisFlags[1] |= 0x01;
		AxisNames[1][AXIS_X] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_YAxis))
	{
		AxisFlags[1] |= 0x02;
		AxisNames[1][AXIS_Y] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_ZAxis))
	{
		AxisFlags[1] |= 0x04;
		AxisNames[1][AXIS_Z] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_Button))
	{
		ItemNum = (lpddoi->dwOfs - ((BYTE *)&MouseState.rgbButtons - (BYTE *)&MouseState)) / sizeof(MouseState.rgbButtons[0]);
		if ((ItemNum >= 0) && (ItemNum < 8))
			ButtonNames[1][ItemNum] = _tcsdup(lpddoi->tszName);
		else	MessageBox(hMainWnd, _T("Error - encountered invalid mouse button ID!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
	}

	return DIENUM_CONTINUE;
}

BOOL CALLBACK	EnumJoystickObjectsCallback (LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	int DevNum = (int)pvRef;
	int ItemNum = 0;
	if (IsEqualGUID(lpddoi->guidType, GUID_XAxis))
	{
		AxisFlags[DevNum] |= 0x01;
		AxisNames[DevNum][AXIS_X] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_YAxis))
	{
		AxisFlags[DevNum] |= 0x02;
		AxisNames[DevNum][AXIS_Y] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_ZAxis))
	{
		AxisFlags[DevNum] |= 0x04;
		AxisNames[DevNum][AXIS_Z] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_RxAxis))
	{
		AxisFlags[DevNum] |= 0x08;
		AxisNames[DevNum][AXIS_RX] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_RyAxis))
	{
		AxisFlags[DevNum] |= 0x10;
		AxisNames[DevNum][AXIS_RY] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_RzAxis))
	{
		AxisFlags[DevNum] |= 0x20;
		AxisNames[DevNum][AXIS_RZ] = _tcsdup(lpddoi->tszName);
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_Slider))
	{
		// ItemNum = DIDFT_GETINSTANCE(lpddoi->dwType);
		// Sliders are enumerated as axes, and this index
		// starts where the other axes left off
		// Thus, we need to ignore it and assign them incrementally
		// Hopefully, this actually works reliably
		if (!(AxisFlags[DevNum] & 0x40))
			ItemNum = 0;
		else if (!(AxisFlags[DevNum] & 0x80))
			ItemNum = 1;
		else	ItemNum = 2;
		if (ItemNum == 0)
		{
			AxisFlags[DevNum] |= 0x40;
			AxisNames[DevNum][AXIS_S0] = _tcsdup(lpddoi->tszName);
		}
		else if (ItemNum == 1)
		{
			AxisFlags[DevNum] |= 0x80;
			AxisNames[DevNum][AXIS_S1] = _tcsdup(lpddoi->tszName);
		}
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_POV))
	{
		ItemNum = DIDFT_GETINSTANCE(lpddoi->dwType);
		if ((ItemNum >= 0) && (ItemNum < 4))
		{
			POVFlags[DevNum] |= 0x01 << ItemNum;
			POVNames[DevNum][ItemNum] = _tcsdup(lpddoi->tszName);
		}
	}
	if (IsEqualGUID(lpddoi->guidType, GUID_Button))
	{
		ItemNum = DIDFT_GETINSTANCE(lpddoi->dwType);
		if ((ItemNum >= 0) && (ItemNum < 128))
			ButtonNames[DevNum][ItemNum] = _tcsdup(lpddoi->tszName);
	}
	return DIENUM_CONTINUE;
}

BOOL CALLBACK	EnumJoysticksCallback (LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	HRESULT hr;
	int DevNum = NumDevices;
	do
	{
		if (SUCCEEDED(DirectInput->CreateDevice(lpddi->guidInstance, &DIDevices[DevNum], NULL)))
		{
			DIDEVCAPS caps;
			if (FAILED(hr = DIDevices[DevNum]->SetDataFormat(&c_dfDIJoystick2)))
				break;
			if (FAILED(hr = DIDevices[DevNum]->SetCooperativeLevel(hMainWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
				break;
			caps.dwSize = sizeof(DIDEVCAPS);
			if (FAILED(hr = DIDevices[DevNum]->GetCapabilities(&caps)))
				break;

			NumButtons[DevNum] = caps.dwButtons;
			AxisFlags[DevNum] = 0;
			POVFlags[DevNum] = 0;
			DeviceName[DevNum] = _tcsdup(lpddi->tszProductName);
			DeviceGUID[DevNum] = lpddi->guidInstance;

			DIDevices[DevNum]->EnumObjects(EnumJoystickObjectsCallback, (LPVOID)DevNum, DIDFT_ALL);
			//EI.DbgOut(_T("Added input device '%s' with %i buttons, %i axes, %i POVs"), lpddi->tszProductName, caps.dwButtons, caps.dwAxes, caps.dwPOVs);
			NumDevices++;
		}
		return DIENUM_CONTINUE;
	} while (0);

	DIDevices[DevNum]->Release();
	DIDevices[DevNum] = NULL;
	return hr;
}

BOOL	InitKeyboard (void)
{
	DIDEVICEINSTANCE inst;
	DIDEVCAPS caps;
	do
	{
		if (FAILED(DirectInput->CreateDevice(GUID_SysKeyboard, &DIDevices[0], NULL)))
			return FALSE;
		if (FAILED(DIDevices[0]->SetDataFormat(&c_dfDIKeyboard)))
			break;
		if (FAILED(DIDevices[0]->SetCooperativeLevel(hMainWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
			break;

		caps.dwSize = sizeof(DIDEVCAPS);
		if (FAILED(DIDevices[0]->GetCapabilities(&caps)))
			break;

		inst.dwSize = sizeof(DIDEVICEINSTANCE);
		if (FAILED(DIDevices[0]->GetDeviceInfo(&inst)))
			break;

		NumButtons[0] = 256;	// normally, I would use caps.dwButtons
		AxisFlags[0] = 0;	// no axes
		POVFlags[0] = 0;	// no POV hats
		DeviceName[0] = _tcsdup(inst.tszProductName);
		DeviceGUID[0] = GUID_SysKeyboard;

		DIDevices[0]->EnumObjects(EnumKeyboardObjectsCallback, NULL, DIDFT_ALL);
		//EI.DbgOut(_T("Added input device '%s' with %i buttons, %i axes, %i POVs"), inst.tszProductName, caps.dwButtons, caps.dwAxes, caps.dwPOVs);
		return TRUE;
	} while (0);

	DIDevices[0]->Release();
	DIDevices[0] = NULL;
	return FALSE;
}

BOOL	InitMouse (void)
{
	DIDEVICEINSTANCE inst;
	DIDEVCAPS caps;
	do
	{
		if (FAILED(DirectInput->CreateDevice(GUID_SysMouse, &DIDevices[1], NULL)))
			return FALSE;
		if (FAILED(DIDevices[1]->SetDataFormat(&c_dfDIMouse2)))
			break;
		if (FAILED(DIDevices[1]->SetCooperativeLevel(hMainWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
			break;
		caps.dwSize = sizeof(DIDEVCAPS);
		if (FAILED(DIDevices[1]->GetCapabilities(&caps)))
			break;

		inst.dwSize = sizeof(DIDEVICEINSTANCE);
		if (FAILED(DIDevices[1]->GetDeviceInfo(&inst)))
			break;

		NumButtons[1] = caps.dwButtons;
		AxisFlags[1] = 0;
		POVFlags[1] = 0;
		DeviceName[1] = _tcsdup(inst.tszProductName);
		DeviceGUID[1] = GUID_SysMouse;

		DIDevices[1]->EnumObjects(EnumMouseObjectsCallback, NULL, DIDFT_ALL);
		//EI.DbgOut(_T("Added input device '%s' with %i buttons, %i axes, %i POVs"), inst.tszProductName, caps.dwButtons, caps.dwAxes, caps.dwPOVs);
		return TRUE;
	} while (0);

	DIDevices[1]->Release();
	DIDevices[1] = NULL;
	return FALSE;
}

void	Init (void)
{
	int i;

	for (i = 0; i < NumDevices; i++)
	{
		DeviceUsed[i] = FALSE;
		DIDevices[i] = NULL;
		DeviceName[i] = NULL;
		ZeroMemory(AxisNames[i], sizeof(AxisNames[i]));
		ZeroMemory(ButtonNames[i], sizeof(ButtonNames[i]));
		ZeroMemory(POVNames[i], sizeof(POVNames[i]));
	}
	ZeroMemory(KeyNames, sizeof(KeyNames));

	StdPort_SetMappings();
	ExpPort_SetMappings();

	ZeroMemory(Port1_Buttons, sizeof(Port1_Buttons));
	ZeroMemory(Port2_Buttons, sizeof(Port2_Buttons));
	ZeroMemory(FSPort1_Buttons, sizeof(FSPort1_Buttons));
	ZeroMemory(FSPort2_Buttons, sizeof(FSPort2_Buttons));
	ZeroMemory(FSPort3_Buttons, sizeof(FSPort3_Buttons));
	ZeroMemory(FSPort4_Buttons, sizeof(FSPort4_Buttons));
	ZeroMemory(PortExp_Buttons, sizeof(PortExp_Buttons));

	Port1 = new StdPort_Unconnected(Port1_Buttons);
	Port2 = new StdPort_Unconnected(Port2_Buttons);
	FSPort1 = new StdPort_Unconnected(FSPort1_Buttons);
	FSPort2 = new StdPort_Unconnected(FSPort2_Buttons);
	FSPort3 = new StdPort_Unconnected(FSPort3_Buttons);
	FSPort4 = new StdPort_Unconnected(FSPort4_Buttons);
	PortExp = new ExpPort_Unconnected(PortExp_Buttons);

	if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&DirectInput, NULL)))
	{
		MessageBox(hMainWnd, _T("Unable to initialize DirectInput!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return;
	}

	if (!InitKeyboard())
		MessageBox(hMainWnd, _T("Failed to initialize keyboard!"), _T("Nintendulator"), MB_OK | MB_ICONWARNING);
	if (!InitMouse())
		MessageBox(hMainWnd, _T("Failed to initialize mouse!"), _T("Nintendulator"), MB_OK | MB_ICONWARNING);

	NumDevices = 2;	// joysticks start at slot 2
	if (FAILED(DirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ALLDEVICES)))
		MessageBox(hMainWnd, _T("Failed to initialize joysticks!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);

	Movie::Mode = 0;
}

void	Destroy (void)
{
	int i, j;
	delete Port1;	Port1 = NULL;
	delete Port2;	Port2 = NULL;
	delete FSPort1;	FSPort1 = NULL;
	delete FSPort2;	FSPort2 = NULL;
	delete FSPort3;	FSPort3 = NULL;
	delete FSPort4;	FSPort4 = NULL;
	delete PortExp;	PortExp = NULL;
	for (i = 0; i < NumDevices; i++)
	{
		if (DIDevices[i])
		{
			DIDevices[i]->Release();
			DIDevices[i] = NULL;
		}
		// Allocated using _tcsdup()
		free(DeviceName[i]);
		DeviceName[i] = NULL;
		for (j = 0; j < 128; j++)
		{
			// Allocated using _tcsdup()
			free(ButtonNames[i][j]);
			ButtonNames[i][j] = NULL;
		}
		for (j = 0; j < 8; j++)
		{
			// Allocated using _tcsdup()
			free(AxisNames[i][j]);
			AxisNames[i][j] = NULL;
		}
		for (j = 0; j < 4; j++)
		{
			// Allocated using _tcsdup()
			free(POVNames[i][j]);
			POVNames[i][j] = NULL;
		}
	}
	for (j = 0; j < 256; j++)
	{
		// Allocated using _tcsdup()
		free(KeyNames[j]);
		KeyNames[j] = NULL;
	}

	DirectInput->Release();
	DirectInput = NULL;
}

void	Write (unsigned char Val) {
	Port1->Write(Val);
	Port2->Write(Val);
	PortExp->Write(Val);
}

int	Save (FILE *out)
{
	int clen = 0;
	unsigned char type;

	type = (unsigned char)Port1->Type;	writeByte(type);
	clen += Port1->Save(out);

	type = (unsigned char)Port2->Type;	writeByte(type);
	clen += Port2->Save(out);

	type = (unsigned char)PortExp->Type;	writeByte(type);
	clen += PortExp->Save(out);

	type = (unsigned char)FSPort1->Type;	writeByte(type);
	clen += FSPort1->Save(out);

	type = (unsigned char)FSPort2->Type;	writeByte(type);
	clen += FSPort2->Save(out);

	type = (unsigned char)FSPort3->Type;	writeByte(type);
	clen += FSPort3->Save(out);

	type = (unsigned char)FSPort4->Type;	writeByte(type);
	clen += FSPort4->Save(out);

	writeBool(!!RI.dipValueSet);
	if (RI.dipValueSet) writeLong(RI.dipValue);

	return clen;
}

int	Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned char type;
	Movie::ControllerTypes[3] = 1;	// denotes that controller state has been loaded
					// if we're playing a movie, this means we should
					// SKIP the controller info in the movie block

	readByte(type);
	SET_STDCONT(Port1, (STDCONT_TYPE)type);
	clen += Port1->Load(in, version_id);

	readByte(type);
	// if there's a Four Score on the first port, then assume the same for port 2
	if (Port1->Type == STD_FOURSCORE)
		SET_STDCONT(Port2, STD_FOURSCORE2);
	else // If not, however, make sure that the right half of a four-score doesn't end up here
	if ((STDCONT_TYPE)type == STD_FOURSCORE2)
		SET_STDCONT(Port2, STD_UNCONNECTED);
	else
		SET_STDCONT(Port2, (STDCONT_TYPE)type);

	// the same for Sudoku Excalibur
	if (Port1->Type == STD_SUDOKU_EXCALIBUR)
		SET_STDCONT(Port2, STD_SUDOKU_EXCALIBUR2);
	else
	if ((STDCONT_TYPE)type == STD_SUDOKU_EXCALIBUR2)
		SET_STDCONT(Port2, STD_UNCONNECTED);
	else
		SET_STDCONT(Port2, (STDCONT_TYPE)type);

	clen += Port2->Load(in, version_id);

	readByte(type);
	SET_EXPCONT(PortExp, (EXPCONT_TYPE)type);
	clen += PortExp->Load(in, version_id);

	readByte(type);
	SET_STDCONT(FSPort1, (STDCONT_TYPE)type);
	clen += FSPort1->Load(in, version_id);

	readByte(type);
	SET_STDCONT(FSPort2, (STDCONT_TYPE)type);
	clen += FSPort2->Load(in, version_id);

	readByte(type);
	SET_STDCONT(FSPort3, (STDCONT_TYPE)type);
	clen += FSPort3->Load(in, version_id);

	readByte(type);
	SET_STDCONT(FSPort4, (STDCONT_TYPE)type);
	clen += FSPort4->Load(in, version_id);

	readBool(RI.dipValueSet);
	if (RI.dipValueSet) readLong(RI.dipValue);

	return clen;
}

StdPort** const stdControllerPorts[6] = { &Port1, &Port2, &FSPort1, &FSPort2, &FSPort3, &FSPort4 };
DWORD* const stdPortButtons[6] = { Port1_Buttons, Port2_Buttons, FSPort1_Buttons, FSPort2_Buttons, FSPort3_Buttons, FSPort4_Buttons };

const static TCHAR *stdControllerRegKeys[STD_MAX] = {
	_T("Unconnected"),	// 0  STD_UNCONNECTED,
	_T("StdController"),    // 1  STD_STDCONTROLLER,
	_T("Zapper"),           // 2  STD_ZAPPER,
	_T("ArkanoidPaddle"),   // 3  STD_ARKANOIDPADDLE,
	_T("PowerPadA"),        // 4  STD_POWERPADA,
	_T("PowerPadB"),        // 5  STD_POWERPADB,
	_T("FourScore"),        // 6  STD_FOURSCORE,
	_T("SnesController"),   // 7  STD_SNESCONTROLLER,
	_T("Zapper"),           // 2  STD_VSZAPPER,
	_T("SnesMouse"),        // 8  STD_SNESMOUSE,
	_T("SnesMouse"),        // 8  STD_SUBORMOUSE,
	_T("FourScore2"),       // 6  STD_FOURSCORE2,
	_T("SudokuExcalibur"),  // 14 STD_SUDOKU_EXCALIBUR,
	_T("SudokuExcalibur2")  // 14 STD_SUDOKU_EXCALIBUR2
};
const static TCHAR *expControllerRegKeys[EXP_MAX] = {
	_T("Unconnected"),		// 0  EXP_UNCONNECTED,
	_T("Fami4Play"),                // 6  EXP_FAMI4PLAY,
	_T("Zapper"),                   // 2  EXP_ZAPPER,
	_T("ArkanoidPaddle"),           // 3  EXP_ARKANOIDPADDLE,
	_T("FamilyBASICKeyboard"),      // 0  EXP_FAMILYBASICKEYBOARD,
	_T("SuborKeyboard"),            // 0  EXP_SUBORKEYBOARD,
	_T("PowerPadB"),                // 5  EXP_FAMTRAINERA,
	_T("PowerPadA"),                // 4  EXP_FAMTRAINERB,
	_T("Tablet"),                   // 9  EXP_TABLET,
	_T("BandaiHyperShot"),          // 10 EXP_BANDAIHYPERSHOT,
	_T("PEC586Keyboard"),           // 0  EXP_PEC586KEYBOARD,
	_T("TurboFile"),                // 0  EXP_TURBOFILE,
	_T("Bit79Keyboard"),            // 0  EXP_BIT79KEYBOARD,
	_T("Zapper"),                   // 2  EXP_CITYPATROLMAN,
	_T("Moguraa"),                  // 11 EXP_MOGURAA,
	_T("KonamiHyperShot"),          // 12 EXP_KONAMIHYPERSHOT,
	_T("SharpC1Cassette"),          // 13 EXP_SHARPC1CASSETTE,
	_T("GoldenNuggetCasino"),       // 15 EXP_GOLDENNUGGETCASINO,
	_T("GoldenKeyboard"),           // 0  EXP_GOLDENNUGGETCASINO,
	_T("Hori4Play"),                // 6  EXP_HORI4PLAY,
};
const static int std2universal[STD_MAX] = { 0, 1, 2, 3, 4, 5, 6, 7, 2, 8, 8, 6, 14, 14 };
const static int exp2universal[EXP_MAX] = { 0, 6, 2, 3, 0, 0, 5, 4, 9,10, 0, 0, 0, 2, 11, 12, 13, 15, 6, 0 };

void	SaveButtons (HKEY SettingsBase) {
	TCHAR RegKey[64];
	int controllerUsed[STD_MAX +EXP_MAX];
	memset(controllerUsed, 0, sizeof(controllerUsed));

	for (int controllerType =STD_UNCONNECTED; controllerType <STD_MAX; controllerType++) {
		if (controllerType ==STD_UNCONNECTED || controllerType ==STD_FOURSCORE || controllerType ==STD_FOURSCORE2 || controllerType ==STD_SUDOKU_EXCALIBUR2) continue;
		for (int portNum =0; portNum <6; portNum++) {
			const StdPort *port =*stdControllerPorts[portNum];
			if (port ==NULL) continue;
			if (port->Type !=controllerType) continue;
			swprintf_s(RegKey, 64, _T("Buttons%s%d"), stdControllerRegKeys[controllerType], ++controllerUsed[std2universal[controllerType]]);
			if (port->NumButtons >0)
				RegSetValueEx(SettingsBase, RegKey, 0, REG_BINARY, (LPBYTE) stdPortButtons[portNum], port->NumButtons * sizeof(DWORD));
		}
	}
	for (int controllerType =EXP_UNCONNECTED; controllerType <EXP_MAX; controllerType++) {
		if (controllerType ==EXP_UNCONNECTED) continue;
		const ExpPort *port =PortExp;
		if (port ==NULL) continue;
		if (port->Type !=controllerType) continue;
		swprintf_s(RegKey, 64, _T("Buttons%s%d"), expControllerRegKeys[controllerType], ++controllerUsed[exp2universal[controllerType]]);
		if (port->NumButtons >0)
				RegSetValueEx(SettingsBase, RegKey, 0, REG_BINARY, (LPBYTE)PortExp_Buttons, PortExp->NumButtons * sizeof(DWORD));
	}
}

void	LoadButtons (HKEY SettingsBase) {
	unsigned long Size;
	TCHAR RegKey[64];

	int controllerUsed[STD_MAX +EXP_MAX];
	memset(controllerUsed, 0, sizeof(controllerUsed));
	for (int controllerType =STD_UNCONNECTED; controllerType <STD_MAX; controllerType++) {
		if (controllerType ==STD_UNCONNECTED || controllerType ==STD_FOURSCORE || controllerType ==STD_FOURSCORE2 || controllerType ==STD_SUDOKU_EXCALIBUR2) continue;
		for (int portNum =0; portNum <6; portNum++) {
			const StdPort *port =*stdControllerPorts[portNum];
			if (port ==NULL) continue;
			if (port->Type !=controllerType) continue;
			swprintf_s(RegKey, 64, _T("Buttons%s%d"), stdControllerRegKeys[controllerType], ++controllerUsed[std2universal[controllerType]]);
			if (port->NumButtons >0) {
				Size = port->NumButtons * sizeof(DWORD);
				RegQueryValueEx(SettingsBase, RegKey, 0, NULL, (LPBYTE)stdPortButtons[portNum], &Size);
			}
		}
	}
	for (int controllerType =EXP_UNCONNECTED; controllerType <EXP_MAX; controllerType++) {
		if (controllerType ==EXP_UNCONNECTED) continue;
		const ExpPort *port =PortExp;
		if (port ==NULL) continue;
		if (port->Type !=controllerType) continue;
		swprintf_s(RegKey, 64, _T("Buttons%s%d"), expControllerRegKeys[controllerType], ++controllerUsed[exp2universal[controllerType]]);
		if (port->NumButtons >0) {
				Size = PortExp->NumButtons * sizeof(DWORD);
				RegQueryValueEx(SettingsBase, RegKey, 0, NULL, (LPBYTE)PortExp_Buttons, &Size);
		}
	}
}

void	SaveSettings (HKEY SettingsBase) {
	DWORD numDevsUsed = 0;
	for (int i = 0; i < MAX_CONTROLLERS; i++)
		if (DeviceUsed[i])
			numDevsUsed++;

	int buflen = sizeof(DWORD) + numDevsUsed * (sizeof(GUID) + sizeof(DWORD));
	BYTE *buf = new BYTE[buflen], *b = buf;

	memcpy(b, &numDevsUsed, sizeof(DWORD)); b += sizeof(DWORD);
	for (DWORD i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (!DeviceUsed[i])
			continue;
		memcpy(b, &i, sizeof(DWORD)); b += sizeof(DWORD);
		memcpy(b, &DeviceGUID[i], sizeof(GUID)); b += sizeof(GUID);
	}

	RegSetValueEx(SettingsBase, _T("UDLR")    , 0, REG_DWORD, (LPBYTE)&EnableOpposites, sizeof(BOOL));

	RegSetValueEx(SettingsBase, _T("Port1T")  , 0, REG_DWORD, (LPBYTE)&Port1->Type  , sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("Port2T")  , 0, REG_DWORD, (LPBYTE)&Port2->Type  , sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("FSPort1T"), 0, REG_DWORD, (LPBYTE)&FSPort1->Type, sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("FSPort2T"), 0, REG_DWORD, (LPBYTE)&FSPort2->Type, sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("FSPort3T"), 0, REG_DWORD, (LPBYTE)&FSPort3->Type, sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("FSPort4T"), 0, REG_DWORD, (LPBYTE)&FSPort4->Type, sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("ExpPortT"), 0, REG_DWORD, (LPBYTE)&PortExp->Type, sizeof(DWORD));
	RegSetValueEx(SettingsBase, _T("PortM")   , 0, REG_BINARY, buf, buflen);
	SaveButtons(SettingsBase);

	delete[] buf;
}

void	LoadSettings (HKEY SettingsBase) {
	unsigned long Size;
	DWORD Port1T = 0, Port2T = 0, FSPort1T = 0, FSPort2T = 0, FSPort3T = 0, FSPort4T = 0, ExpPortT = 0;
	Size = sizeof(BOOL);	RegQueryValueEx(SettingsBase, _T("UDLR")    , 0, NULL, (LPBYTE)&EnableOpposites, &Size);

	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("Port1T")  , 0, NULL, (LPBYTE)&Port1T  , &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("Port2T")  , 0, NULL, (LPBYTE)&Port2T  , &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("FSPort1T"), 0, NULL, (LPBYTE)&FSPort1T, &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("FSPort2T"), 0, NULL, (LPBYTE)&FSPort2T, &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("FSPort3T"), 0, NULL, (LPBYTE)&FSPort3T, &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("FSPort4T"), 0, NULL, (LPBYTE)&FSPort4T, &Size);
	Size = sizeof(DWORD);	RegQueryValueEx(SettingsBase, _T("ExpPortT"), 0, NULL, (LPBYTE)&ExpPortT, &Size);

	if (ExpPortT == EXP_TURBOFILE) ExpPortT = EXP_UNCONNECTED; // Do not connect the Turbo File before a game has been loaded to prevent data loss
	if (Port1T == STD_FOURSCORE) {
		SET_STDCONT(Port1, STD_FOURSCORE);
		SET_STDCONT(Port1, STD_FOURSCORE2);
	} else
	if (Port1T == STD_SUDOKU_EXCALIBUR) {
		SET_STDCONT(Port1, STD_SUDOKU_EXCALIBUR);
		SET_STDCONT(Port1, STD_SUDOKU_EXCALIBUR2);
	} else {
		SET_STDCONT(Port1, (STDCONT_TYPE)Port1T);
		if (Port2T == STD_FOURSCORE || Port2T == STD_FOURSCORE2 || Port2T ==STD_SUDOKU_EXCALIBUR || Port2T ==STD_SUDOKU_EXCALIBUR2)
			Port2T = STD_UNCONNECTED;
		SET_STDCONT(Port2, (STDCONT_TYPE)Port2T);
	}
	SET_STDCONT(FSPort1, (STDCONT_TYPE)FSPort1T);
	SET_STDCONT(FSPort2, (STDCONT_TYPE)FSPort2T);
	SET_STDCONT(FSPort3, (STDCONT_TYPE)FSPort3T);
	SET_STDCONT(FSPort4, (STDCONT_TYPE)FSPort4T);
	SET_EXPCONT(PortExp, (EXPCONT_TYPE)ExpPortT);

	LoadButtons(SettingsBase);

	if (RegQueryValueEx(SettingsBase, _T("PortM"), 0, NULL, NULL, &Size) == ERROR_SUCCESS)
	{
		byte* buf = new byte[Size], *b = buf;
		RegQueryValueEx(SettingsBase, _T("PortM"), 0, NULL, buf, &Size);

		DWORD mapLen;
		memcpy(&mapLen, buf, sizeof(DWORD)); b += sizeof(DWORD);
		DWORD *map_nums = new DWORD[mapLen];
		GUID *map_guids = new GUID[mapLen];
		for (size_t i = 0; i < mapLen; i++)
		{
			memcpy(&map_nums[i], b, sizeof(DWORD)); b += sizeof(DWORD);
			memcpy(&map_guids[i], b, sizeof(GUID)); b += sizeof(GUID);
		}

		int Lens[7] = { Port1->NumButtons, Port2->NumButtons, FSPort1->NumButtons, FSPort2->NumButtons, FSPort3->NumButtons, FSPort4->NumButtons, PortExp->NumButtons };
		DWORD *Datas[7] = { Port1_Buttons, Port2_Buttons, FSPort1_Buttons, FSPort2_Buttons, FSPort3_Buttons, FSPort4_Buttons, PortExp_Buttons };
		TCHAR *Descs[7] = { _T("Port 1"), _T("Port 2"), _T("Four-score Port 1"), _T("Four-score Port 2"), _T("Four-score Port 3"), _T("Four-score Port 4"), _T("Expansion Port") };

		// Resolve stored device IDs into current device index values via GUID matching
		// This isn't the most efficient way of doing it, but it's still fast so nobody cares
		for (int i = 0; i < 7; i++)
		{
			for (int j = 0; j < Lens[i]; j++)
			{
				DWORD &Button = Datas[i][j];
				DWORD DevNum = (Button & 0xFFFF0000) >> 16;
				int found = 0;
				for (size_t k = 0; k < mapLen; k++)
				{
					if (DevNum == map_nums[k])
					{
						found = 1;
						for (int m = 0; m < NumDevices; m++)
						{
							if (IsEqualGUID(map_guids[k], DeviceGUID[m]))
							{
								found = 2;
								Button = (m << 16) | (Button & 0xFFFF);
							}
						}
					}
				}
				if (found < 2)
				{
					EI.DbgOut(_T("%s controller button %i failed to find device, unmapping."), Descs[i], j);
					Button = 0;
				}
			}
		}

		delete[] map_nums;
		delete[] map_guids;
		delete[] buf;
	}

	SetDeviceUsed();
}

void	SetDeviceUsed (void)
{
	int i;

	for (i = 0; i < NumDevices; i++)
		DeviceUsed[i] = FALSE;

	for (i = 0; i < CONTROLLERS_MAXBUTTONS; i++)
	{
		//if (Port1->Type == STD_FOURSCORE || PortExp->Type == EXP_BANDAIHYPERSHOT || PortExp->Type == EXP_FAMI4PLAY)
		//{
			if (FSPort1->Type != STD_UNCONNECTED && i < FSPort1->NumButtons)
				DeviceUsed[FSPort1->Buttons[i] >> 16] = TRUE;
			if (FSPort2->Type != STD_UNCONNECTED && i < FSPort2->NumButtons)
				DeviceUsed[FSPort2->Buttons[i] >> 16] = TRUE;
			if (FSPort3->Type != STD_UNCONNECTED && i < FSPort3->NumButtons)
				DeviceUsed[FSPort3->Buttons[i] >> 16] = TRUE;
			if (FSPort4->Type != STD_UNCONNECTED && i < FSPort4->NumButtons)
				DeviceUsed[FSPort4->Buttons[i] >> 16] = TRUE;
		//}
		//else
		//{
			if (Port1->Type != STD_UNCONNECTED && i < Port1->NumButtons)
				DeviceUsed[Port1->Buttons[i] >> 16] = TRUE;
			if (Port2->Type != STD_UNCONNECTED && i < Port2->NumButtons)
				DeviceUsed[Port2->Buttons[i] >> 16] = TRUE;
		//}
		if (PortExp->Type != STD_UNCONNECTED && i < PortExp->NumButtons)
			DeviceUsed[PortExp->Buttons[i] >> 16] = TRUE;
	}
	if ((Port1->Type == STD_ARKANOIDPADDLE) || (Port2->Type == STD_ARKANOIDPADDLE) || (PortExp->Type == EXP_ARKANOIDPADDLE))
		DeviceUsed[1] = TRUE;

	if (PortExp->Type == EXP_FAMILYBASICKEYBOARD || PortExp->Type == EXP_SUBORKEYBOARD || PortExp->Type == EXP_PEC586KEYBOARD || PortExp->Type == EXP_BIT79KEYBOARD || PortExp->Type == EXP_GOLDENKEYBOARD)
		DeviceUsed[0] = TRUE;
}

void	Acquire (void)
{
	int i;
	for (i = 0; i < NumDevices; i++)
		if (DeviceUsed[i])
			DIDevices[i]->Acquire();
	Port1->SetMasks();
	Port2->SetMasks();
	PortExp->SetMasks();

	if (MaskMouse && Controllers::scrollLock)
	{
		RECT rect;
		POINT point = {0, 0};

		GetClientRect(hMainWnd, &rect);
		ClientToScreen(hMainWnd, &point);

		rect.left += point.x;
		rect.right += point.x;
		rect.top += point.y;
		rect.bottom += point.y;

		ClipCursor(&rect);
		while(ShowCursor(FALSE) >=0);

		/*
		// do not allow both keyboard and mouse to be masked at the same time!
		// NRS:: Yes, do allow it.
		MaskKeyboard = FALSE;
		*/
	}
}
void	UnAcquire (void)
{
	int i;
	for (i = 0; i < NumDevices; i++)
		if (DeviceUsed[i])
			DIDevices[i]->Unacquire();
	if (MaskMouse && Controllers::scrollLock) {
		ClipCursor(NULL);
		while(ShowCursor(TRUE) <0);
	}
	MaskKeyboard = FALSE;
	MaskMouse = FALSE;
}

void	ClearKeyState (void)
{
	ZeroMemory(KeyState, sizeof(KeyState));
}

void	ClearMouseState (void)
{
	ZeroMemory(&MouseState, sizeof(MouseState));
}

void	ClearJoyState (int dev)
{
	ZeroMemory(&JoyState[dev], sizeof(DIJOYSTATE2));
	// axes need to be initialized to 0x8000
	JoyState[dev].lX = JoyState[dev].lY = JoyState[dev].lZ = 0x8000;
	JoyState[dev].lRx = JoyState[dev].lRy = JoyState[dev].lRz = 0x8000;
	JoyState[dev].rglSlider[0] = JoyState[dev].rglSlider[1] = 0x8000;
	// and POV hats need to be initialized to -1
	JoyState[dev].rgdwPOV[0] = JoyState[dev].rgdwPOV[1] = JoyState[dev].rgdwPOV[2] = JoyState[dev].rgdwPOV[3] = (DWORD)-1;
}

// We need to access NewBits directly to reverse the input of controllers 1 and 2 for some Vs. Games
struct StdPort_StdController_State {
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char NewBits;
};

/* Only update once per frame */
void	UpdateInput (void) {
	HRESULT hr;
	int i;
	unsigned char Cmd = 0;
	
	if (DeviceUsed[0])
	{
		hr = DIDevices[0]->GetDeviceState(256, KeyState);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			ClearKeyState();
			DIDevices[0]->Acquire();
		}
	}
	if (DeviceUsed[1])
	{
		hr = DIDevices[1]->GetDeviceState(sizeof(DIMOUSESTATE2), &MouseState);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			ClearMouseState();
			DIDevices[1]->Acquire();
		}
	}
	for (i = 2; i < NumDevices; i++)
		if (DeviceUsed[i])
		{
			hr = DIDevices[i]->Poll();
			if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			{
				ClearJoyState(i);
				DIDevices[i]->Acquire();
				continue;
			}
			hr = DIDevices[i]->GetDeviceState(sizeof(DIJOYSTATE2), &JoyState[i]);
			if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				ClearJoyState(i);
		}

	if (Movie::Mode & MOV_PLAY)
		Cmd = Movie::LoadInput();
	else
	{
		if ((MI) && (MI->Config))
			Cmd = MI->Config(CFG_QUERY, 0);
	}
	Port1->Frame(Movie::Mode);
	Port2->Frame(Movie::Mode);
	PortExp->Frame(Movie::Mode);
	/*if (VSDUAL)
		EI.DbgOut(L"%02x %02X %02X %02X",
			dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits,
			dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits,
			dynamic_cast<StdPort_StdController*>(FSPort3)->State->NewBits,
			dynamic_cast<StdPort_StdController*>(FSPort4)->State->NewBits
		);
	else
		EI.DbgOut(L"%02x %02X",
			dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits,
			dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits
		);*/
	if (RI.ConsoleType ==CONSOLE_VS) {
		// Reverse START and SELECT
		if (VSDUAL) {
			int bits1 = dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits;
			int bits2 = dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits = (bits1 &~0x0C) | ((bits1 &0x04)? 0x08: 0x00) | ((bits1 &0x08)? 0x04: 0x00);
			dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits = (bits2 &~0x0C) | ((bits2 &0x04)? 0x08: 0x00) | ((bits2 &0x08)? 0x04: 0x00);
			bits1 = dynamic_cast<StdPort_StdController*>(FSPort3)->State->NewBits;
			bits2 = dynamic_cast<StdPort_StdController*>(FSPort4)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(FSPort3)->State->NewBits = (bits1 &~0x0C) | ((bits1 &0x04)? 0x08: 0x00) | ((bits1 &0x08)? 0x04: 0x00);
			dynamic_cast<StdPort_StdController*>(FSPort4)->State->NewBits = (bits2 &~0x0C) | ((bits2 &0x04)? 0x08: 0x00) | ((bits2 &0x08)? 0x04: 0x00);
		} else if (RI.InputType !=INPUT_VS_ZAPPER) {
			int bits1 = dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits;
			int bits2 = dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits = (bits1 &~0x0C) | ((bits1 &0x04)? 0x08: 0x00) | ((bits1 &0x08)? 0x04: 0x00);
			dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits = (bits2 &~0x0C) | ((bits2 &0x04)? 0x08: 0x00) | ((bits2 &0x08)? 0x04: 0x00);
		}

		if (RI.InputType ==INPUT_VS_PINBALL) {
			int bits1 = dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits;
			int bits2 = dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits =(bits1 &~0x03) | ((bits1 &0x01)? 0x01: 0x00);
			dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits =(bits2 &~0x03) | ((bits1 &0x02)? 0x01: 0x00);
		} else
		if (VSDUAL && RI.InputType ==INPUT_VS_REVERSED) {
			int bits1 = dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits;
			int bits2 = dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits =(bits2 &~0x0C) | (bits1 &0x0C);
			dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits =(bits1 &~0x0C) | (bits2 &0x0C);
			bits1 = dynamic_cast<StdPort_StdController*>(FSPort3)->State->NewBits;
			bits2 = dynamic_cast<StdPort_StdController*>(FSPort4)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(FSPort3)->State->NewBits =(bits2 &~0x0C) | (bits1 &0x0C);
			dynamic_cast<StdPort_StdController*>(FSPort4)->State->NewBits =(bits1 &~0x0C) | (bits2 &0x0C);
		} else if (RI.InputType ==INPUT_VS_REVERSED) {
			int bits1 = dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits;
			int bits2 = dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits;
			dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits =(bits2 &~0x0C) | (bits1 &0x0C);
			dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits =(bits1 &~0x0C) | (bits2 &0x0C);
		}
		// Vs. Ice Climber / Vs. Raid on Bungeling Bay Protection
		if (RI.INES2_VSFlags ==VS_ICECLIMBERJ) {
			dynamic_cast<StdPort_StdController*>(Port1)->State->NewBits |=0x08;
			dynamic_cast<StdPort_StdController*>(Port2)->State->NewBits |=0x08;
		}
		if (RI.INES2_VSFlags ==VS_BUNGELING) {
			dynamic_cast<StdPort_StdController*>(FSPort1)->State->NewBits |=0x08;
			dynamic_cast<StdPort_StdController*>(FSPort2)->State->NewBits |=0x08;
		}
	}

	capsLock =!!(GetKeyState(VK_CAPITAL) &1);
	bool newScrollLock =!!(GetKeyState(VK_SCROLL) &1);
	if (newScrollLock != scrollLock) {
		UnAcquire();
		scrollLock =newScrollLock; // Order is important here!
		Acquire();
	}

	if (Cmd && MI && MI->Config) MI->Config(CFG_CMD, Cmd);
	if (Movie::Mode & MOV_RECORD) Movie::SaveInput(Cmd);
}

int	GetConfigButton (HWND hWnd, int DevNum, BOOL AxesOnly = FALSE){
	LPDIRECTINPUTDEVICE8 dev = DIDevices[DevNum];
	HRESULT hr;
	int i;
	int Key = -1;
	int FirstAxis, LastAxis;
	int FirstPOV, LastPOV;
	if (DevNum == 0)
	{
		if (AxesOnly)
			return 0;
		FirstAxis = LastAxis = 0;
		FirstPOV = LastPOV = 0;
	}
	else if (DevNum == 1)
	{
		FirstAxis = 0x08;
		LastAxis = 0x0E;
		FirstPOV = LastPOV = 0;
	}
	else
	{
		FirstAxis = 0x80;
		LastAxis = 0x90;
		if (POVAxis)
		{
			FirstPOV = 0xE0;
			LastPOV = 0xF0;
		}
		else
		{
			FirstPOV = 0xC0;
			LastPOV = 0xE0;
		}
	}

	if (FAILED(dev->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		MessageBox(hMainWnd, _T("Unable to modify device input cooperative level!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return Key;
	}

	dev->Acquire();
	DWORD ticks;
	ticks = GetTickCount();
	while (Key == -1)
	{
		ProcessMessages();	// Wine workaround
		// abort after 5 seconds
		if (GetTickCount() - ticks > 5000)
			break;
		if (DevNum == 0)
			hr = dev->GetDeviceState(256, KeyState);
		else if (DevNum == 1)
			hr = dev->GetDeviceState(sizeof(DIMOUSESTATE2), &MouseState);
		else
		{
			hr = dev->Poll();
			hr = dev->GetDeviceState(sizeof(DIJOYSTATE2), &JoyState[DevNum]);
		}
		if (!AxesOnly)
		{
			for (i = 0; i < NumButtons[DevNum]; i++)
			{
				if (IsPressed((DevNum << 16) | i))
				{
					Key = i;
					break;
				}
			}
			if (Key != -1)	// if we got a button, don't check for an axis
				break;
		}
		for (i = FirstAxis; i < LastAxis; i++)
		{
			if (IsPressed((DevNum << 16) | i))
			{
				Key = i;
				break;
			}
		}
		if (!AxesOnly)
		{
			if (Key != -1)	// if we got an axis, don't check for a POV hat
				break;
			for (i = FirstPOV; i < LastPOV; i++)
			{
				if (IsPressed((DevNum << 16) | i))
				{
					Key = i;
					break;
				}
			}
		}
	}

	ticks = GetTickCount();
	while (1)
	{
		// if we timed out above, then skip this
		if (Key == -1)
			break;
		ProcessMessages();	// Wine workaround
		// wait 3 seconds to release key
		if (GetTickCount() - ticks > 3000)
			break;

		bool held = false;
		if (DevNum == 0)
			hr = dev->GetDeviceState(256, KeyState);
		else if (DevNum == 1)
			hr = dev->GetDeviceState(sizeof(DIMOUSESTATE2), &MouseState);
		else
		{
			hr = dev->Poll();
			hr = dev->GetDeviceState(sizeof(DIJOYSTATE2), &JoyState[DevNum]);
		}
		// don't need to reset FirstAxis/LastAxis or FirstPOV/LastPOV, since they were set in the previous loop
		if (!AxesOnly)
		{
			for (i = 0; i < NumButtons[DevNum]; i++)
				if (IsPressed((DevNum << 16) | i))
				{
					held = true;
					break;
				}
			if (held)
				continue;
		}
		for (i = FirstAxis; i < LastAxis; i++)
			if (IsPressed((DevNum << 16) | i))
			{
				held = true;
				break;
			}
		if (held)
			continue;
		if (!AxesOnly)
		{
			for (i = FirstPOV; i < LastPOV; i++)
				if (IsPressed((DevNum << 16) | i))
				{
					held = true;
					break;
				}
			if (held)
				continue;
		}
		break;
	}
	dev->Unacquire();
	if (FAILED(dev->SetCooperativeLevel(hMainWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		MessageBox(hMainWnd, _T("Unable to restore device input cooperative level!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return Key;
	}
	return Key;
}

const TCHAR *	GetButtonLabel (int DevNum, int Button, BOOL AxesOnly = FALSE)
{
	static TCHAR str[256];
	_tcscpy(str, _T("???"));
	if (AxesOnly && (!DevNum || !Button))
		return str;
	if (DevNum == 0)
	{
		if (KeyNames[Button])
			_tcscpy(str, KeyNames[Button]);
		return str;
	}
	else if (DevNum == 1)
	{
		if (Button & 0x08)
		{
			Button &= 0x07;
			if (AxisNames[1][Button >> 1])
				_stprintf(str, _T("%s %s"), AxisNames[1][Button >> 1], (Button & 1) ? _T("(+)") : _T("(-)"));
			return str;
		}
		else
		{
			if (ButtonNames[1][Button])
				_tcscpy(str, ButtonNames[1][Button]);
			return str;
		}
	}
	else
	{
		if ((Button & 0xE0) == 0x80)
		{
			Button &= 0x0F;
			if (AxisNames[DevNum][Button >> 1])
				_stprintf(str, _T("%s %s"), AxisNames[DevNum][Button >> 1], (Button & 1) ? _T("(+)") : _T("(-)"));
			return str;
		}
		else if ((Button & 0xE0) == 0xC0)
		{
			const TCHAR *POVs[8] = {_T("(N)"), _T("(NE)"), _T("(E)"), _T("(SE)"), _T("(S)"), _T("(SW)"), _T("(W)"), _T("(NW)") };
			Button &= 0x1F;
			if (POVNames[DevNum][Button >> 3])
				_stprintf(str, _T("%s %s"), POVNames[DevNum][Button >> 3], POVs[Button & 0x7]);
			return str;
		}
		else if ((Button & 0xE0) == 0xE0)
		{
			const TCHAR *POVs[4] = {_T("Y (-)"), _T("X (+)"), _T("Y (+)"), _T("X (-)") };
			Button &= 0xF;
			if (POVNames[DevNum][Button >> 2])
				_stprintf(str, _T("%s %s"), POVNames[DevNum][Button >> 2], POVs[Button & 0x3]);
			return str;
		}
		else
		{
			if (ButtonNames[DevNum][Button])
				_tcscpy(str, ButtonNames[DevNum][Button]);
			return str;
		}
	}
}

void	ConfigButton (DWORD *Button, int Device, HWND hDlg, BOOL getKey, BOOL AxesOnly)
{
	*Button &= 0xFFFF;
	if (getKey)	// this way, we can just re-label the button
	{
		key = CreateDialog(hInst, AxesOnly ? MAKEINTRESOURCE(IDD_AXISCONFIG) : MAKEINTRESOURCE(IDD_KEYCONFIG), hDlg, NULL);
		ShowWindow(key, TRUE);	// FIXME - center this window properly
		ProcessMessages();	// let the "Press a key..." dialog display itself
		int newKey = GetConfigButton(key, Device, AxesOnly);
		if (newKey != -1)
			*Button = newKey;
		ProcessMessages();	// flush all keypresses - don't want them going back to the parent dialog
		DestroyWindow(key);	// close the little window
		key = NULL;
	}
	SetWindowText(hDlg, GetButtonLabel(Device, *Button, AxesOnly));
	*Button |= Device << 16;	// add the device ID
}

BOOL	IsPressed (int Button)
{
	int DevNum = (Button & 0xFFFF0000) >> 16;
	if (DevNum == 0)
		return (KeyState[Button & 0xFF] & 0x80) ? TRUE : FALSE;
	else if (DevNum == 1)
	{
		if (Button & 0x8)	// axis selected
		{
			switch (Button & 0x7)
			{
			case 0x0:	return ((AxisFlags[1] & (1 << AXIS_X)) && (MouseState.lX < -1)) ? TRUE : FALSE;	break;
			case 0x1:	return ((AxisFlags[1] & (1 << AXIS_X)) && (MouseState.lX > +1)) ? TRUE : FALSE;	break;
			case 0x2:	return ((AxisFlags[1] & (1 << AXIS_Y)) && (MouseState.lY < -1)) ? TRUE : FALSE;	break;
			case 0x3:	return ((AxisFlags[1] & (1 << AXIS_Y)) && (MouseState.lY > +1)) ? TRUE : FALSE;	break;
			case 0x4:	return ((AxisFlags[1] & (1 << AXIS_Z)) && (MouseState.lZ < -1)) ? TRUE : FALSE;	break;
			case 0x5:	return ((AxisFlags[1] & (1 << AXIS_Z)) && (MouseState.lZ > +1)) ? TRUE : FALSE;	break;
			}
		}
		else	return (MouseState.rgbButtons[Button & 0x7] & 0x80) ? TRUE : FALSE;
	}
	else
	{
		if ((Button & 0xE0) == 0x80)
		{	// axis
			switch (Button & 0xF)
			{
			case 0x0:	return ((AxisFlags[DevNum] & (1 << AXIS_X)) && (JoyState[DevNum].lX < 0x4000)) ? TRUE : FALSE;	break;
			case 0x1:	return ((AxisFlags[DevNum] & (1 << AXIS_X)) && (JoyState[DevNum].lX > 0xC000)) ? TRUE : FALSE;	break;
			case 0x2:	return ((AxisFlags[DevNum] & (1 << AXIS_Y)) && (JoyState[DevNum].lY < 0x4000)) ? TRUE : FALSE;	break;
			case 0x3:	return ((AxisFlags[DevNum] & (1 << AXIS_Y)) && (JoyState[DevNum].lY > 0xC000)) ? TRUE : FALSE;	break;
			case 0x4:	return ((AxisFlags[DevNum] & (1 << AXIS_Z)) && (JoyState[DevNum].lZ < 0x4000)) ? TRUE : FALSE;	break;
			case 0x5:	return ((AxisFlags[DevNum] & (1 << AXIS_Z)) && (JoyState[DevNum].lZ > 0xC000)) ? TRUE : FALSE;	break;
			case 0x6:	return ((AxisFlags[DevNum] & (1 << AXIS_RX)) && (JoyState[DevNum].lRx < 0x4000)) ? TRUE : FALSE;	break;
			case 0x7:	return ((AxisFlags[DevNum] & (1 << AXIS_RX)) && (JoyState[DevNum].lRx > 0xC000)) ? TRUE : FALSE;	break;
			case 0x8:	return ((AxisFlags[DevNum] & (1 << AXIS_RY)) && (JoyState[DevNum].lRy < 0x4000)) ? TRUE : FALSE;	break;
			case 0x9:	return ((AxisFlags[DevNum] & (1 << AXIS_RY)) && (JoyState[DevNum].lRy > 0xC000)) ? TRUE : FALSE;	break;
			case 0xA:	return ((AxisFlags[DevNum] & (1 << AXIS_RZ)) && (JoyState[DevNum].lRz < 0x4000)) ? TRUE : FALSE;	break;
			case 0xB:	return ((AxisFlags[DevNum] & (1 << AXIS_RZ)) && (JoyState[DevNum].lRz > 0xC000)) ? TRUE : FALSE;	break;
			case 0xC:	return ((AxisFlags[DevNum] & (1 << AXIS_S0)) && (JoyState[DevNum].rglSlider[0] < 0x4000)) ? TRUE : FALSE;	break;
			case 0xD:	return ((AxisFlags[DevNum] & (1 << AXIS_S0)) && (JoyState[DevNum].rglSlider[0] > 0xC000)) ? TRUE : FALSE;	break;
			case 0xE:	return ((AxisFlags[DevNum] & (1 << AXIS_S1)) && (JoyState[DevNum].rglSlider[1] < 0x4000)) ? TRUE : FALSE;	break;
			case 0xF:	return ((AxisFlags[DevNum] & (1 << AXIS_S1)) && (JoyState[DevNum].rglSlider[1] > 0xC000)) ? TRUE : FALSE;	break;
			}
		}
		else if ((Button & 0xE0) == 0xC0)
		{	// POV trigger (8-button mode)
			int povNum = (Button >> 3) & 0x3;
			if (JoyState[DevNum].rgdwPOV[povNum] == -1)
				return FALSE;
			switch (Button & 0x7)
			{
			case 0x00:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 33750) || (JoyState[DevNum].rgdwPOV[povNum] <  2250))) ? TRUE : FALSE;	break;
			case 0x01:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] >  2250) && (JoyState[DevNum].rgdwPOV[povNum] <  6750))) ? TRUE : FALSE;	break;
			case 0x02:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] >  6750) && (JoyState[DevNum].rgdwPOV[povNum] < 11250))) ? TRUE : FALSE;	break;
			case 0x03:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 11250) && (JoyState[DevNum].rgdwPOV[povNum] < 15750))) ? TRUE : FALSE;	break;
			case 0x04:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 15750) && (JoyState[DevNum].rgdwPOV[povNum] < 20250))) ? TRUE : FALSE;	break;
			case 0x05:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 20250) && (JoyState[DevNum].rgdwPOV[povNum] < 24750))) ? TRUE : FALSE;	break;
			case 0x06:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 24750) && (JoyState[DevNum].rgdwPOV[povNum] < 29250))) ? TRUE : FALSE;	break;
			case 0x07:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 29250) && (JoyState[DevNum].rgdwPOV[povNum] < 33750))) ? TRUE : FALSE;	break;
			}
		}
		else if ((Button & 0xE0) == 0xE0)
		{	// POV trigger (axis mode)
			int povNum = (Button >> 2) & 0x3;
			if (JoyState[DevNum].rgdwPOV[povNum] == -1)
				return FALSE;
			switch (Button & 0x03)
			{
			case 0x0:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 29250) || (JoyState[DevNum].rgdwPOV[povNum] <  6750))) ? TRUE : FALSE;	break;
			case 0x1:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] >  2250) && (JoyState[DevNum].rgdwPOV[povNum] < 15750))) ? TRUE : FALSE;	break;
			case 0x2:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 11250) && (JoyState[DevNum].rgdwPOV[povNum] < 24750))) ? TRUE : FALSE;	break;
			case 0x3:	return ((POVFlags[DevNum] & (1 << povNum)) && ((JoyState[DevNum].rgdwPOV[povNum] > 20250) && (JoyState[DevNum].rgdwPOV[povNum] < 33750))) ? TRUE : FALSE;	break;
			}
		}
		else	return (JoyState[DevNum].rgbButtons[Button & 0x7F] & 0x80) ? TRUE : FALSE;
	}
	// should never actually reach this point - this is just to make the compiler stop whining
	return FALSE;
}

int	GetDelta (int Button)
{
	int DevNum = (Button & 0xFFFF0000) >> 16;
	if (DevNum == 0)
		return 0;	// Keyboard not supported
	else if (DevNum == 1)
	{
		if (Button & 0x8)	// axis selected
		{
			switch (Button & 0x7)
			{
			case 0x0:	return (AxisFlags[1] & (1 << AXIS_X)) ? -MouseState.lX : 0;	break;
			case 0x1:	return (AxisFlags[1] & (1 << AXIS_X)) ?  MouseState.lX : 0;	break;
			case 0x2:	return (AxisFlags[1] & (1 << AXIS_Y)) ? -MouseState.lY : 0;	break;
			case 0x3:	return (AxisFlags[1] & (1 << AXIS_Y)) ?  MouseState.lY : 0;	break;
			case 0x4:	return (AxisFlags[1] & (1 << AXIS_Z)) ? -MouseState.lZ : 0;	break;
			case 0x5:	return (AxisFlags[1] & (1 << AXIS_Z)) ?  MouseState.lZ : 0;	break;
			}
		}
		else	return 0;
	}
	else
	{
		if ((Button & 0xE0) == 0x80)
		{	// axis
			switch (Button & 0xF)
			{
			case 0x0:	return (AxisFlags[DevNum] & (1 << AXIS_X)) ? -(0x8000 - JoyState[DevNum].lX) / 0x400 : 0;	break;
			case 0x1:	return (AxisFlags[DevNum] & (1 << AXIS_X)) ?  (0x8000 - JoyState[DevNum].lX) / 0x400 : 0;	break;
			case 0x2:	return (AxisFlags[DevNum] & (1 << AXIS_Y)) ? -(0x8000 - JoyState[DevNum].lY) / 0x400 : 0;	break;
			case 0x3:	return (AxisFlags[DevNum] & (1 << AXIS_Y)) ?  (0x8000 - JoyState[DevNum].lY) / 0x400 : 0;	break;
			case 0x4:	return (AxisFlags[DevNum] & (1 << AXIS_Z)) ? -(0x8000 - JoyState[DevNum].lZ) / 0x400 : 0;	break;
			case 0x5:	return (AxisFlags[DevNum] & (1 << AXIS_Z)) ?  (0x8000 - JoyState[DevNum].lZ) / 0x400 : 0;	break;
			case 0x6:	return (AxisFlags[DevNum] & (1 << AXIS_RX)) ? -(0x8000 - JoyState[DevNum].lRx) / 0x400 : 0;	break;
			case 0x7:	return (AxisFlags[DevNum] & (1 << AXIS_RX)) ?  (0x8000 - JoyState[DevNum].lRx) / 0x400 : 0;	break;
			case 0x8:	return (AxisFlags[DevNum] & (1 << AXIS_RY)) ? -(0x8000 - JoyState[DevNum].lRy) / 0x400 : 0;	break;
			case 0x9:	return (AxisFlags[DevNum] & (1 << AXIS_RY)) ?  (0x8000 - JoyState[DevNum].lRy) / 0x400 : 0;	break;
			case 0xA:	return (AxisFlags[DevNum] & (1 << AXIS_RZ)) ? -(0x8000 - JoyState[DevNum].lRz) / 0x400 : 0;	break;
			case 0xB:	return (AxisFlags[DevNum] & (1 << AXIS_RZ)) ?  (0x8000 - JoyState[DevNum].lRz) / 0x400 : 0;	break;
			case 0xC:	return (AxisFlags[DevNum] & (1 << AXIS_S0)) ? -(0x8000 - JoyState[DevNum].rglSlider[0]) / 0x400 : 0;	break;
			case 0xD:	return (AxisFlags[DevNum] & (1 << AXIS_S0)) ?  (0x8000 - JoyState[DevNum].rglSlider[0]) / 0x400 : 0;	break;
			case 0xE:	return (AxisFlags[DevNum] & (1 << AXIS_S1)) ? -(0x8000 - JoyState[DevNum].rglSlider[1]) / 0x400 : 0;	break;
			case 0xF:	return (AxisFlags[DevNum] & (1 << AXIS_S1)) ?  (0x8000 - JoyState[DevNum].rglSlider[1]) / 0x400 : 0;	break;
			}
		}
		else	return 0;	// buttons and POV axes not supported
	}
	// should never actually reach this point - this is just to make the compiler stop whining
	return 0;
}

INT_PTR	ParseConfigMessages (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, int numButtons, int numAxes, const int *dlgDevices, const int *dlgButtons, DWORD *Buttons)
{
	int wmId = LOWORD(wParam);
	int wmEvent = HIWORD(wParam);
	int i;
	if (key)
		return FALSE;
	if (uMsg == WM_INITDIALOG)
	{
		for (i = 0; i < numButtons + numAxes; i++)
		{
			int j;
			SendDlgItemMessage(hDlg, dlgDevices[i], CB_RESETCONTENT, 0, 0);		// clear the listbox
			SendDlgItemMessage(hDlg, dlgDevices[i], CB_ADDSTRING, 0, i < numButtons ? (LPARAM)DeviceName[0] : (LPARAM)_T("Select a device..."));
			for (j = 1; j < NumDevices; j++)
				SendDlgItemMessage(hDlg, dlgDevices[i], CB_ADDSTRING, 0, (LPARAM)DeviceName[j]);	// add each device
			SendDlgItemMessage(hDlg, dlgDevices[i], CB_SETCURSEL, Buttons[i] >> 16, 0);	// select the one we want
			ConfigButton(&Buttons[i], Buttons[i] >> 16, GetDlgItem(hDlg, dlgButtons[i]), FALSE, i >= numButtons);	// and label the corresponding button
		}
	}
	if (uMsg != WM_COMMAND)
		return FALSE;
	if (wmId == IDOK)
	{
		EndDialog(hDlg, 1);
		return TRUE;
	}
	for (i = 0; i < numButtons + numAxes; i++)
	{
		if (wmId == dlgDevices[i])
		{
			if (wmEvent != CBN_SELCHANGE)
				break;
			Buttons[i] = 0;
			ConfigButton(&Buttons[i], (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), GetDlgItem(hDlg, dlgButtons[i]), FALSE, i >= numButtons);
			return TRUE;
		}
		if (wmId == dlgButtons[i])
		{
			ConfigButton(&Buttons[i], Buttons[i] >> 16, GetDlgItem(hDlg, dlgButtons[i]), TRUE, i >= numButtons);
			return TRUE;
		}
	}
	return FALSE;
}

void	SetDefaultInput(uint8_t value) {
	EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_MISC_STOPTAPE, MF_GRAYED);
	// Disconnect everything
	StdPort_SetControllerType(FSPort4, STD_UNCONNECTED, FSPort4_Buttons);
	StdPort_SetControllerType(FSPort3, STD_UNCONNECTED, FSPort3_Buttons);
	StdPort_SetControllerType(FSPort2, STD_UNCONNECTED, FSPort2_Buttons);
	StdPort_SetControllerType(FSPort1, STD_UNCONNECTED, FSPort1_Buttons);
	ExpPort_SetControllerType(PortExp, EXP_UNCONNECTED, PortExp_Buttons);
	StdPort_SetControllerType(Port2, STD_UNCONNECTED, Port2_Buttons);
	StdPort_SetControllerType(Port1, STD_UNCONNECTED, Port1_Buttons);
	if (VSDUAL) {
		StdPort_SetControllerType(Port1, STD_FOURSCORE, Port1_Buttons);
		StdPort_SetControllerType(Port2, STD_FOURSCORE2, Port2_Buttons);
		StdPort_SetControllerType(FSPort1, STD_STDCONTROLLER, FSPort1_Buttons);
		StdPort_SetControllerType(FSPort2, STD_STDCONTROLLER, FSPort2_Buttons);
		StdPort_SetControllerType(FSPort3, STD_STDCONTROLLER, FSPort3_Buttons);
		StdPort_SetControllerType(FSPort4, STD_STDCONTROLLER, FSPort4_Buttons);
	} else
	switch(value) {
		case INPUT_DOUBLE_FISTED:
		case INPUT_FOURSCORE:
			StdPort_SetControllerType(Port1, STD_FOURSCORE, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_FOURSCORE2, Port2_Buttons);
			StdPort_SetControllerType(FSPort1, STD_STDCONTROLLER, FSPort1_Buttons);
			StdPort_SetControllerType(FSPort2, STD_STDCONTROLLER, FSPort2_Buttons);
			StdPort_SetControllerType(FSPort3, STD_STDCONTROLLER, FSPort3_Buttons);
			StdPort_SetControllerType(FSPort4, STD_STDCONTROLLER, FSPort4_Buttons);
			break;
		case INPUT_FAMI4PLAY:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			StdPort_SetControllerType(FSPort3, STD_STDCONTROLLER, FSPort3_Buttons);
			StdPort_SetControllerType(FSPort4, STD_STDCONTROLLER, FSPort4_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_FAMI4PLAY, PortExp_Buttons);
			break;
		case INPUT_VS_ZAPPER:
			StdPort_SetControllerType(Port1, STD_VSZAPPER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			break;
		case INPUT_ZAPPER: case INPUT_TWO_ZAPPERS:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_ZAPPER, PortExp_Buttons);
			break;
		case INPUT_BANDAI_HYPER_SHOT:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_BANDAIHYPERSHOT, PortExp_Buttons);
			break;
		case INPUT_POWER_PAD_A:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_POWERPADA, Port2_Buttons);
			break;
		case INPUT_POWER_PAD_B:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_POWERPADB, Port2_Buttons);
			break;
		case INPUT_FAMILY_TRAINER_A:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_FAMTRAINERA, PortExp_Buttons);
			break;
		case INPUT_FAMILY_TRAINER_B:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_FAMTRAINERB, PortExp_Buttons);
			break;
		case INPUT_VAUS_NES:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_ARKANOIDPADDLE, Port2_Buttons);
			break;
		case INPUT_VAUS_FAMICOM:
		case INPUT_TWO_VAUS:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_ARKANOIDPADDLE, PortExp_Buttons);
			break;
		case INPUT_OEKA_KIDS_TABLET:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_TABLET, PortExp_Buttons);
			break;
		case INPUT_PEC_586:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_PEC586KEYBOARD, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		case INPUT_BIT_79:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_BIT79KEYBOARD, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		case INPUT_FAMILY_BASIC:
		case INPUT_DATA_RECORDER:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_FAMILYBASICKEYBOARD, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		case INPUT_TURBO_FILE:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_TURBOFILE, PortExp_Buttons);
			break;
		case INPUT_SUBOR_KEYBOARD:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_SUBORKEYBOARD, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		case INPUT_SUBOR_KBDMOUSE8:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_SUBORMOUSE, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_SUBORKEYBOARD, PortExp_Buttons);
			break;
		case INPUT_SUBOR_KBDMOUSE24_1P:
			StdPort_SetControllerType(Port1, STD_SUBORMOUSE, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_SUBORKEYBOARD, PortExp_Buttons);
			break;
		case INPUT_SUBOR_KBDMOUSE24_2P:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_SUBORMOUSE, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_SUBORKEYBOARD, PortExp_Buttons);
			break;
		case INPUT_SNES_CONTROLLER:
			StdPort_SetControllerType(Port1, STD_SNESCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_SNESCONTROLLER, Port2_Buttons);
			break;
		case INPUT_SNES_MOUSE:
			StdPort_SetControllerType(Port1, STD_SNESMOUSE, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			break;
		case INPUT_CITY_PATROLMAN:
			ExpPort_SetControllerType(PortExp, EXP_CITYPATROLMAN, PortExp_Buttons);
			break;
		case INPUT_POKKUN_MOGURAA:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_MOGURAA, PortExp_Buttons);
			break;
		case INPUT_KONAMI_HYPER_SHOT:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_KONAMIHYPERSHOT, PortExp_Buttons);
			break;
		case INPUT_SHARP_C1_CASSETTE:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_SHARPC1CASSETTE, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		case INPUT_SUDOKU_EXCALIBUR:
			StdPort_SetControllerType(Port1, STD_SUDOKU_EXCALIBUR, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_SUDOKU_EXCALIBUR2, Port2_Buttons);
			break;
		case INPUT_GOLDEN_NUGGET_CASINO:
			StdPort_SetControllerType(Port1, STD_UNCONNECTED, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_UNCONNECTED, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_GOLDENNUGGETCASINO, PortExp_Buttons);
			break;
		case INPUT_GOLDEN_KEYBOARD:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			ExpPort_SetControllerType(PortExp, EXP_GOLDENKEYBOARD, PortExp_Buttons);
			EnableMenuItem(hMenu, ID_MISC_PLAYTAPE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_MISC_RECORDTAPE, MF_ENABLED);
			break;
		default:
			StdPort_SetControllerType(Port1, STD_STDCONTROLLER, Port1_Buttons);
			StdPort_SetControllerType(Port2, STD_STDCONTROLLER, Port2_Buttons);
			break;
	}
	SetDeviceUsed();
	UpdateInput();
	SetCursor(*CurrentCursor);
}

struct GenericMulticart_ControllerStrings {
	uint8_t InputType;
	uint16_t CompareAddress;
	size_t CompareSize;
	const char *CompareData;
} GenericMulticart_ControllerStrings[] = {
	{ INPUT_ZAPPER,            0xD131, 16, "\xAD\x17\x40\x29\x10\xC5\xB7\xF0\x07\x85\xB7\xA9\x00\x85\xB8\x60" },         // Duck Hunt
	{ INPUT_ZAPPER,            0xCF3E, 16, "\xAD\x17\x40\x29\x10\xC5\x95\xF0\x07\x85\x95\xA9\x00\x85\x96\x60" },         // Hogan's Alley
	{ INPUT_ZAPPER,            0xD01C, 16, "\xAD\x17\x40\x29\x10\xC5\xA0\xF0\x07\x85\xA0\xA9\x00\x85\xA1\x60" },         // Wild Gunman (PRG0)
	{ INPUT_ZAPPER,            0xD02B, 16, "\xAD\x17\x40\x29\x10\xC5\xA0\xF0\x07\x85\xA0\xA9\x00\x85\xA1\x60" },         // Wild Gunman (PRG1)
	{ INPUT_ZAPPER,            0xE090, 15, "\xAD\x17\x40\x85\x07\x4A\x05\x07\x4A\x26\x05\x88\xD0\xE7\x60" },             // Mad City
	{ INPUT_TWO_ZAPPERS,       0xE1C3,  7, "\xAD\x17\x40\x29\x10\x85\x45" },                                             // Hit Marmot (Caltron Ind. Inc.)
	{ INPUT_TWO_ZAPPERS,       0x832F,  7, "\xAD\x17\x40\x29\x10\xC5\x10" },                                             // Fantasy of Gun
	{ INPUT_DATA_RECORDER,     0xC65C, 16, "\x8D\x16\x40\xC6\x07\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x0C\xD0\xFC" },         // ExciteBike (NTSC)
	{ INPUT_DATA_RECORDER,     0xAAF5, 16, "\x8D\x16\x40\xC6\x08\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x09\xD0\xFC" },         // Mach Rider
	{ INPUT_DATA_RECORDER,     0xB91C, 16, "\x8D\x16\x40\xC6\x01\xD0\xFC\xA9\xFF\x8D\x16\x40\xC6\x02\xD0\xFC" },         // Wrecking Crew
	{ INPUT_DATA_RECORDER,     0xE9B8, 18, "\xA9\x05\x8D\x16\x40\xEA\xEA\xEA\xEA\xEA\xEA\xA2\x00\xA9\x04\x8D\x16\x40" }, // Lode Runner (U.S. and Japan)
	{ INPUT_POWER_PAD_B,       0xD166,  7, "\xAD\x17\x40\x48\x29\x08\x20" },                                             // World Class Track Meet (PRG0 and PRG1)
	{ INPUT_TURBO_FILE,        0x8201,  9, "\xAD\x17\x40\x4A\x4A\x29\x01\x09\x06" },                                     // Castle Excellent
	{ INPUT_TURBO_FILE,        0xFD8E,  9, "\xAD\x16\x40\x4A\x26\xEE\x4A\x26\x04" },                                     // Downtown熱血物語
	{ INPUT_VAUS_FAMICOM,      0x8C5E,  9, "\xAD\x17\x40\x6A\x26\x02\x6A\x26\x08" },                                     // Arkanoid (Famicom)
	{ INPUT_VAUS_FAMICOM,      0xC4FE,  9, "\xAD\x17\x40\x6A\x26\x02\xEA\xEA\x6A" },                                     // Block Shock (UM6578)
	{ INPUT_VAUS_FAMICOM,      0xC869,  9, "\xBD\x16\x40\x4A\x36\x20\x4A\x36\x6E" },                                     // Taito Chase H.Q.
	{ INPUT_VAUS_NES,          0x8C09,  9, "\xAD\x17\x40\x6A\x26\x02\x6A\x6A\x6A" },                                     // Arkanoid (NES)
	{ INPUT_TWO_VAUS,          0xDA6E,  9, "\xAD\x17\x40\x6A\x26\x00\x6A\x26\x01" },                                     // Arkanoid II
	{ INPUT_KONAMI_HYPER_SHOT, 0xC1F9,  9, "\xAD\x17\x40\xA8\x29\x06\x4A\xAA\xA5" },                                     // Hyper Olympic
	{ INPUT_KONAMI_HYPER_SHOT, 0x8242,  9, "\xAD\x17\x40\xA8\x29\x06\x4A\xAA\xA5" },                                     // Hyper Olympic 限定版!
	{ INPUT_KONAMI_HYPER_SHOT, 0xC1A2,  9, "\xAD\x17\x40\xA8\x29\x06\x4A\xAA\xA5" },                                     // Hyper Sports (PRG0/1)
	{ INPUT_FAMI4PLAY,         0xC5FA,  9, "\xAD\x16\x40\x4A\x26\x04\x4A\x26\x06" },                                     // もえろ TwinBee (Re-release)
	{ INPUT_FAMI4PLAY,         0xD9F4, 10, "\xAD\x16\x40\x4A\x2E\x0C\x03\x4A\x26\x00" },                                 // Super Dodgeball
	{ INPUT_FAMI4PLAY,         0xFD2C, 10, "\xBD\x16\x40\x4A\x36\x06\x4A\x36\x04\x88" },                                 // Downtown 熱血行進曲 ~それゆけ大運動会~
	{ INPUT_FAMI4PLAY,         0xF39C,  9, "\xAD\x16\x40\x4A\x26\x04\x4A\x26\x08" },                                     // いけいけ熱血 Hockey部꞉ すべってころんで大乱闘
	{ INPUT_FAMI4PLAY,         0xF9AB,  9, "\xAD\x16\x40\x4A\x26\x04\x4A\x26\x08" },                                     // 熱血格闘伝説
	{ INPUT_FAMI4PLAY,         0xEE12,  9, "\xAD\x16\x40\x4A\x26\x08\x4A\x26\x0C" },                                     // くにおくんの熱血 Soccer League
	{ INPUT_FAMI4PLAY,         0xF19B,  9, "\xBD\x16\x40\x4A\x36\x0E\x4A\x36\x0C" },                                     // U.S. Championship V'Ball
	{ INPUT_FOURSCORE,         0x8050,  9, "\xAD\x16\x40\x4A\x36\x08\x4A\x36\x0C" },                                     // Kings of the Beach
	{ INPUT_FOURSCORE,         0xDBE8,  9, "\xBD\x16\x40\x4A\x36\x04\x88\xD0\xF7" },                                     // Monster Truck Rally
	{ INPUT_FOURSCORE,         0xFC95,  9, "\xBD\x16\x40\x29\x03\xC9\x01\x36\x00" },                                     // Nintendo World Cup
	{ INPUT_DOUBLE_FISTED,     0xE1BF,  9, "\xAD\x16\x40\x4A\x2E\x70\x02\xAD\x17" },                                     // Smash TV
	{ INPUT_FAMICOM_3D,        0xC143,  9, "\xA5\x38\x49\x06\x09\x01\x8D\x16\x40" },                                     // Attack Animal 学園
	{ INPUT_FAMICOM_3D,        0xDBA0, 12, "HIGHWAY STAR" },                                                             // Highway Star
};

void	SwitchControllersAutomatically (void) {
	uint8_t NewInputType =INPUT_STANDARD;
	for (auto& theString: GenericMulticart_ControllerStrings) {
		bool match =true;
		for (size_t i =0; i <theString.CompareSize; i++) {
			int Address =theString.CompareAddress +i;
			if ((theString.CompareData[i] &0xFF) != (EI.GetCPUReadHandlerDebug(Address >>12))(Address >>12, Address &0xFFF)) {
				match =false;
				break;
			}
		}
		if (match) {
			NewInputType =theString.InputType;
			break;
		}
	}
	if (RI.InputType !=NewInputType) {
		EI.DbgOut(_T("Auto-switching input to %s"), HeaderEdit::ExpansionDevices[NewInputType]);
		RI.InputType =NewInputType;
		SetDefaultInput(RI.InputType);
		/* The cursor is only redrawn when the position is updated, so do that */
		POINT mousepos;
		GetCursorPos(&mousepos);
		SetCursorPos(mousepos.x, mousepos.y);
	}
}

//#include "GameDB_Controllers.h"
} // namespace Controllers