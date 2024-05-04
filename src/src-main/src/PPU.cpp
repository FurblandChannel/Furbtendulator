#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "APU.h"
#include "PPU.h"
#include "CPU.h"
#include "GFX.h"
#include "Debugger.h"
#include "Settings.h"
#include "Controllers.h"

#define DELAY_VRAM_ADDRESS_CHANGE 0
namespace PPU {
bool	showBG =true;
bool	showOBJ =true;
const	unsigned char	ReverseCHR[256] ={
	0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

const	unsigned long	CHRLoBit[16] ={
	0x00000000,0x00000001,0x00000100,0x00000101,0x00010000,0x00010001,0x00010100,0x00010101,
	0x01000000,0x01000001,0x01000100,0x01000101,0x01010000,0x01010001,0x01010100,0x01010101
};
const	unsigned long	CHRHiBit[16] ={
	0x00000000,0x00000002,0x00000200,0x00000202,0x00020000,0x00020002,0x00020200,0x00020202,
	0x02000000,0x02000002,0x02000200,0x02000202,0x02020000,0x02020002,0x02020200,0x02020202
};

/* const */ unsigned char	OpenBus[0x400] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
	0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

void	MAPINT	NoPPUCycle		(int Addr, int Scanline, int Cycle, int _IsRendering)	{ }

PPU_RP2C02::PPU_RP2C02(int _which):
	which(_which) { 
	memset(Sprite, 0xFF, sizeof(Sprite));
	memset(DrawArray, 0x0F, sizeof(DrawArray));
	Spr0InLine =0;
	SprCount =Spr0Hit =SpritePtr =SprAddr =SprAddrH =sprstate =sprcount =sprzero =sprtmp =0;
	EndSLTicks =341;
	SprBuff =&Sprite[256];	
	
	RenderAddr =VRAMAddr =PatAddr =0;
	memset(RenderData, 0, sizeof(RenderData));
	Clockticks =0;
	UpdateVRAMAddr =0;
	for (int i =0; i <16; i++) {
		ReadHandler[i] =BusRead;
		ReadHandlerDebug[i] =BusRead;
		WriteHandler[i] =(i &8)? BusWriteNT: BusWriteCHR;
	}
}

PPU_RP2C02::PPU_RP2C02():
	PPU_RP2C02(0) { 
}

void	PPU_RP2C02::Reset (void) {
	if (Settings::PPUSoftReset) PowerOn();
	pipeline =0;
	SprAddr =0;
	IsRendering =FALSE;
	OnScreen =FALSE;
	UpdateIsRendering =FALSE;
	IOVal =0;
	IOMode =0;
	Clockticks =0;
	PALsubticks =0;
	SLnum =0;
	SprBuff =&Sprite[256];
	GetHandlers();
	GfxData =DrawArray;
}

void	PPU_RP2C02::GetHandlers (void) {
	if (which ==0 && MI && MI->PPUCycle)
		PPUCycle =MI->PPUCycle;
	else	
		PPUCycle =NoPPUCycle;
	GetGFXPtr();
}

void	PPU_RP2C02::SetRegion (void) {
	switch (NES::CurRegion) {
	case Settings::REGION_NTSC:
		PALRatio =FALSE;
		SkipTick =TRUE;
		SLStartNMI =241;
		SLEndFrame =262;
		if (SLnum >=SLEndFrame -1)	// if we switched from PAL, scanline number could be invalid
			SLnum =SLEndFrame -2;
		break;
	case Settings::REGION_PAL:
		PALRatio =TRUE;
		SkipTick =FALSE;
		SLStartNMI =241;
		SLEndFrame =312;
		break;
	case Settings::REGION_DENDY:
		PALRatio =FALSE;
		SkipTick =FALSE;
		SLStartNMI =291;
		SLEndFrame =312;
		break;
	default:
		EI.DbgOut(_T("Invalid PPU region selected!"));
		break;
	}
	ActualColorEmphasis =(NES::CurRegion ==Settings::REGION_PAL || NES::CurRegion ==Settings::REGION_DENDY && Settings::DendySwapEmphasis)? ((ColorEmphasis &0x80) >>1) | ((ColorEmphasis &0x40) <<1) | (ColorEmphasis &0x100) :ColorEmphasis;
	if (Settings::DisableEmphasis) ActualColorEmphasis =0;
}

void	PPU_RP2C02::ProcessSprites (void) {
	if (Clockticks ==0) {
		// if Sprite address is not zero at the beginning of the pre-render scanline
		// then copy the contents of its 8-byte page into the first 8 bytes. Needed for "Huge Insect"
		if (SLnum ==-1  && (SprAddr &0xF8) !=0)	memcpy(Sprite, Sprite +(SprAddr &0xF8), 8);
		if (SLnum ==-1 && GFX::g_bSan2) memcpy(GFX::CPU_BACKUP, CPU::CPU[which]->RAM, 256);
		SpritePtr =SprAddr =SprAddrH =0;
	} else
	if (Clockticks <=64) {
		if (Clockticks &1)
			sprtmp =Sprite[SpritePtr =0x200]; // dummy byte, always 0xFF
		else {
			Sprite[SpritePtr =0x100 | SprAddrH] =sprtmp;
			if (Settings::NoSpriteLimit) {
				Sprite[SpritePtr +0x20] =sprtmp;
				Sprite[SpritePtr +0x40] =sprtmp;
				Sprite[SpritePtr +0x60] =sprtmp;
				Sprite[SpritePtr +0x80] =sprtmp;
				Sprite[SpritePtr +0xA0] =sprtmp;
				Sprite[SpritePtr +0xC0] =sprtmp;
				Sprite[SpritePtr +0xE0] =sprtmp;
			}
			SprAddrH =(SprAddrH +1) &0x1F;
		}
	} else
	if (Clockticks <=256) {
		if (Clockticks ==65) {
			sprstate =0;
			sprzero =FALSE;
			sprcount =0; // shortcut used for sprite rendering logic later on
		}
		switch (sprstate) {
		case 0:	// evaluate current Y coordinate
			if (Clockticks &1)
				sprtmp =Sprite[SpritePtr =SprAddr] -(Settings::upByOne? 1: 0);
			else {
				Sprite[SpritePtr =0x100 | SprAddrH] =sprtmp;
				if (SLnum >=sprtmp && SLnum <=sprtmp +(Reg2000 &0x20? 0xF: 0x7)) {
					sprcount++;
					// sprite is in range
					if (SprAddr ==0) sprzero =TRUE;
					// start fetching data bytes
					sprstate =1;
					SprAddr =(SprAddr +1) &0xFF;
					SprAddrH =(SprAddrH +1) &0x1F;
				} else {
					// checked all 64 sprites? skip to state 4
					SprAddr =(SprAddr +4) &0xFC;
					// wrapped around? skip to state 4
					if (!SprAddr) sprstate =4;
				}
			}
			break;
		case 1:	// Y-coordinate is in range, copy remaining bytes
			if (Clockticks &1)
				sprtmp =Sprite[SpritePtr =SprAddr];
			else {
				Sprite[SpritePtr =0x100 | SprAddrH] =sprtmp;
				SprAddr =(SprAddr +1) &0xFF;
				SprAddrH =(SprAddrH +1) &0x1F;
				// copied 4 bytes
				if (!(SprAddrH &0x3)) {					
					if (SprAddr <0x4) 	// checked all 64 sprites? skip to state 4
						sprstate =4;					
					else
					if (SprAddrH ==0) {	// found 8 sprites? go to state 2
						sprstate =2;
						if (Settings::NoSpriteLimit) {
							// When allowing more then eight sprites per scanline,
							// Fill up the extra bytes of secondary OAM out-of-sequence
							uint8_t SprAddrExtra =SprAddr &~3;
							uint8_t SprAddrHExtra =SprAddrH +0x20;
							while (SprAddrExtra) {
								uint8_t spriteScanline =Sprite[SprAddrExtra];
								if (SLnum >=spriteScanline && SLnum <=spriteScanline +(Reg2000 &0x20? 0xF: 0x7)) {
									Sprite[0x100 |SprAddrHExtra |0] =Sprite[SprAddrExtra |0];
									Sprite[0x100 |SprAddrHExtra |1] =Sprite[SprAddrExtra |1];
									Sprite[0x100 |SprAddrHExtra |2] =Sprite[SprAddrExtra |2];
									Sprite[0x100 |SprAddrHExtra |3] =Sprite[SprAddrExtra |3];
									sprcount++;
									SprAddrHExtra +=4;
								}
								SprAddrExtra +=4;
							}
							sprstate =4; // Don't do anything else now!
						}
					} else	
						sprstate =0;
				}
			}
			break;
		case 2:	// exactly 8 sprites detected, go through 'weird' evaluation
			if (Clockticks &1)
				sprtmp =Sprite[SpritePtr =SprAddr];
			else {
				SpritePtr =0x100 | SprAddrH;	// write suppressed
				if (SLnum >=sprtmp && SLnum <=sprtmp +(Reg2000 &0x20? 0xF: 0x7)) {
					// 9th sprite found "in range"
					sprstate =3;
					if (!Settings::NoSpriteLimit) Reg2002 |=0x20; // set sprite overflow flag
					SprAddr =(SprAddr +1) &0xFF;
					SprAddrH =1; // this doesn't actually happen, but it's simpler to track this way
				} else {
					if ((SprAddr &0x3) ==0x3)
						SprAddr =(SprAddr +1) &0xFF;
					else	
						SprAddr =(SprAddr +5) &0xFF;
					if (SprAddr <0x4) sprstate =4;
				}
			}
			break;
		case 3:	// 9th sprite detected, fetch next 3 bytes
			if (Clockticks &1)
				sprtmp =Sprite[SpritePtr =SprAddr];
			else {
				SpritePtr =0x100;	// write suppressed
				SprAddr =(SprAddr +1) &0xFF;
				SprAddrH++;
				// fetched 4 bytes
				if (SprAddrH ==4) {
					// if we're misaligned, go back to the beginning of the sprite
					if (SprAddr &0x3)
						SprAddr =(SprAddr &0xFC);
					else	
						SprAddr =(SprAddr +4) &0xFC;
					SprAddrH =0;
					sprstate =4;
				}
			}
			break;
		case 4:	// no more sprites to evaluate, thrash until HBLANK
			if (Clockticks &1)
				sprtmp =Sprite[SpritePtr =SprAddr];
			else {
				SpritePtr =0x100 | SprAddrH;
				SprAddr =(SprAddr +4) &0xFC;
			}
			break;
		}
	} else
	if (Clockticks <=320) {
		if (Clockticks ==257) {
			SprCount =sprcount *4;
			Spr0InLine =sprzero;
			SprAddr =SprAddrH =0;
		}
		SpritePtr =0x100 | SprAddrH;
		if (((Clockticks -1) &4) <4)
			SprAddrH =(SprAddrH +1) &0x1F;
	} else	
		SpritePtr =0x100;
}

void	PPU_RP2C02::GetGFXPtr (void) {
	GfxData =DrawArray;
}

void	PPU_RP2C02::PowerOn() {
	Reg2000 =0;
	Reg2001 =0;
	ColorEmphasis =ActualColorEmphasis =0;
	GrayScale =0xFFFFFFFF;
	VRAMAddr =IntReg =0;
	IntX =0;
	readLatch =0;
	Reg2002 =0;
	buf2007 =0;
	HVTog =TRUE;
	ShortSL =TRUE;
	ZeroMemory(VRAM, sizeof(VRAM));
	memset(Palette, 0x3F, sizeof(Palette));
	memset(Sprite, 0xFF, sizeof(Sprite));
	GetHandlers();
}

int	PPU_RP2C02::Save (FILE *out) {
	int clen =0;
	writeArray(VRAM, 0x1000);	//	NTAR	uint8[0x1000]	4 KB of name/attribute table RAM
	writeArray(Sprite, 0x100);	//	SPRA	uint8[0x100]	256 bytes of sprite RAM. Not more, even with NoSpriteLimit, so that states saved with one setting can be restored with the other setting!
	writeArray(Palette, 0x20);	//	PRAM	uint8[0x20]	32 bytes of palette index RAM
	writeByte(Reg2000);		//	R2000	uint8		Last value written to $2000
	writeByte(Reg2001);		//	R2001	uint8		Last value written to $2001
	writeByte(Reg2002);		//	R2002	uint8		Current contents of $2002
	writeByte(SprAddr);		//	SPADR	uint8		SPR-RAM Address ($2003)

	writeByte(IntX);		//	XOFF	uint8		Tile X-offset.

	writeByte(HVTog);		//	VTOG	uint8		Toggle used by $2005 and $2006.
	writeWord(VRAMAddr)		//	RADD	uint16		VRAM Address
	writeWord(IntReg);		//	TADD	uint16		VRAM Address Latch
	writeByte(buf2007);		//	VBUF	uint8		VRAM Read Buffer
	writeByte(readLatch);		//	PGEN	uint8		PPU "general" latch

	writeWord(Clockticks | (PALsubticks << 12));
					//	TICKS	uint16		Clock Ticks (0..340) with PAL subticks stored in upper 4 bits
	writeWord(SLnum);		//	SLNUM	uint16		Scanline number
	writeByte(ShortSL);		//	SHORT	uint8		Short frame (last scanline 1 clock tick shorter)

	writeByte(IOVal);		//	IOVAL	uint8		External I/O Value
	writeByte(IOMode);		//	IOMOD	uint8		External I/O Mode/Counter

	unsigned char region =0;
	switch (NES::CurRegion) {
	case Settings::REGION_NTSC:	region =0;	break;
	case Settings::REGION_PAL:	region =1;	break;
	case Settings::REGION_DENDY:	region =2;	break;
	}
	writeByte(region);		//	NTSCP	uint8		0 for NTSC, 1 for PAL, 2 for Dendy
	return clen;
}

int	PPU_RP2C02::Load (FILE *in, int version_id) {
	int clen =0;
	unsigned short tps;
	readArray(VRAM, 0x1000);	//	NTAR	uint8[0x1000]	4 KB of name/attribute table RAM
	readArray(Sprite, 0x100);	//	SPRA	uint8[0x100]	256 bytes of sprite RAM
	readArray(Palette, 0x20);	//	PRAM	uint8[0x20]	32 bytes of palette index RAM
	readByte(Reg2000);		//	R2000	uint8		Last value written to $2000
	readByte(Reg2001);		//	R2001	uint8		Last value written to $2001
	readByte(Reg2002);		//	R2002	uint8		Current contents of $2002
	readByte(SprAddr);		//	SPADR	uint8		SPR-RAM Address ($2003)

	readByte(IntX);			//	XOFF	uint8		Tile X-offset.

	readByte(HVTog);		//	VTOG	uint8		Toggle used by $2005 and $2006.
	readWord(VRAMAddr);		//	RADD	uint16		VRAM Address
	readWord(IntReg);		//	TADD	uint16		VRAM Address Latch
	readByte(buf2007);		//	VBUF	uint8		VRAM Read Buffer
	readByte(readLatch);		//	PGEN	uint8		PPU "general" latch.

	readWord(tps);			//	TICKS	uint16		Clock Ticks (0..340) with PAL subticks stored in upper 4 bits
	Clockticks =tps &0xFFF;
	PALsubticks =(unsigned char)(tps >> 12);

	readWord(SLnum);		//	SLNUM	uint16		Scanline number
	readByte(ShortSL);		//	SHORT	uint8		Short frame (last scanline 1 clock tick shorter)

	if (version_id < 1001)
		readWord(tps);		//	IOADD	uint16		External I/O Address
	readByte(IOVal);		//	IOVAL	uint8		External I/O Value
	readByte(IOMode);		//	IOMOD	uint8		External I/O Mode/Counter

	unsigned char region =0;
	readByte(region);		//	NTSCP	uint8		0 for NTSC, 1 for PAL, 2 for Dendy

	IsRendering =OnScreen =FALSE;
	ColorEmphasis =(Reg2001 &0xE0) << 1;
	ActualColorEmphasis =(NES::CurRegion ==Settings::REGION_PAL || NES::CurRegion ==Settings::REGION_DENDY && Settings::DendySwapEmphasis)? ((ColorEmphasis &0x80) >>1) | ((ColorEmphasis &0x40) <<1) | (ColorEmphasis &0x100) :ColorEmphasis;
	if (Settings::DisableEmphasis) ActualColorEmphasis =0;
	GrayScale =(Reg2001 &0x01) ? 0xFFFFFFF0: 0xFFFFFFFF;

	switch (region) {
	case 0: NES::SetRegion(Settings::REGION_NTSC);	break;
	case 1: NES::SetRegion(Settings::REGION_PAL);	break;
	case 2: NES::SetRegion(Settings::REGION_DENDY);	break;
	}
	return clen;
}

inline void PPU_RP2C02::IncrementH () {
	if ((VRAMAddr &0x1F) ==0x1F)
		VRAMAddr ^=0x41F;
	else	
		VRAMAddr++;
}

inline void PPU_RP2C02::IncrementV () {
	if ((VRAMAddr &0x7000) ==0x7000) {
		register int YScroll =VRAMAddr &0x3E0;
		VRAMAddr &=0xFFF;
		if (YScroll ==0x3A0)
			VRAMAddr ^=0xBA0;
		else if (YScroll ==0x3E0)
			VRAMAddr ^=0x3E0;
		else	VRAMAddr +=0x20;
	}
	else	
		VRAMAddr +=0x1000;
}
inline void PPU_RP2C02::IncrementAddr () {
	if (Reg2000 &0x04)
		VRAMAddr +=32;
	else	
		VRAMAddr++;
	VRAMAddr &=0x7FFF;
}

__inline void	PPU_RP2C02::RunNoSkip (int NumTicks) {
	register unsigned long TL;
	register unsigned char TC;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i, y;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;

		if (Clockticks ==256) {
			if (SLnum <240) ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 && Clockticks <=303) {
			if (IsRendering && SLnum ==-1) {
				VRAMAddr &=~0x7BE0;
				VRAMAddr |=IntReg &0x7BE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick )
					EndSLTicks =340;
				else	
					EndSLTicks =341;
			} else	
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			}
			if (SLnum ==SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[which]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				/*
				if (RI.INES_MapperNum ==400) {
					DrawArray[16 +0] =RI.dipValue &0x01? 0x16: 0x0F;
					DrawArray[16 +1] =RI.dipValue &0x01? 0x16: 0x0F;
					DrawArray[16 +2] =RI.dipValue &0x02? 0x2A: 0x0F;
					DrawArray[16 +3] =RI.dipValue &0x02? 0x2A: 0x0F;
					DrawArray[16 +4] =RI.dipValue &0x04? 0x28: 0x0F;
					DrawArray[16 +5] =RI.dipValue &0x04? 0x28: 0x0F;
					DrawArray[16 +6] =RI.dipValue &0x08? 0x12: 0x0F;
					DrawArray[16 +7] =RI.dipValue &0x08? 0x12: 0x0F;
					DrawArray[16 +8] =RI.dipValue &0x10? 0x16: 0x0F;
					DrawArray[16 +9] =RI.dipValue &0x10? 0x16: 0x0F;
					DrawArray[16 +10]=RI.dipValue &0x20? 0x2A: 0x0F;
					DrawArray[16 +11]=RI.dipValue &0x20? 0x2A: 0x0F;
					DrawArray[16 +12]=RI.dipValue &0x40? 0x28: 0x0F;
					DrawArray[16 +13]=RI.dipValue &0x40? 0x28: 0x0F;
					DrawArray[16 +14]=RI.dipValue &0x80? 0x12: 0x0F;
					DrawArray[16 +15]=RI.dipValue &0x80? 0x12: 0x0F;
				}
				*/
				if (which ==NES::WhichScreenToShow) GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (RI.ConsoleType ==CONSOLE_VS || RI.ConsoleType ==CONSOLE_PC10) ShortSL =0;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline
		
		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[(Clockticks >> 1) &3] =0;	// seems to force black in this case
					if (IOMode ==2)
						WriteHandler[RenderAddr >>10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[(Clockticks >> 1) &3]);
				} else
				if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData[(Clockticks >> 1) &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				else	
					RenderData[(Clockticks >> 1) &3] =(unsigned char)ReadHandler[RenderAddr >>10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =(RenderData[0] << 4) | (VRAMAddr >> 12) | ((Reg2000 &0x10) << 8);
				break;
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr =0x23C0 | (VRAMAddr &0xC00) | ((VRAMAddr &0x380) >> 4) | ((VRAMAddr &0x1C) >> 2);
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
				CurTileData =&TileData[Clockticks +13];
				TL =((RenderData[1] >> (((VRAMAddr &0x40) >> 4) | (VRAMAddr &0x2))) &3) * 0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case 323:	case 331:
				CurTileData =&TileData[Clockticks -323];
				TL =((RenderData[1] >> (((VRAMAddr &0x40) >> 4) | (VRAMAddr &0x2))) &3) * 0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				RenderAddr =PatAddr;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				TC =ReverseCHR[RenderData[2]];
				CurTileData =&TileData[Clockticks +11];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >> 4];
				break;
			case 325:	case 333:
				TC =ReverseCHR[RenderData[2]];
				CurTileData =&TileData[Clockticks -325];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >> 4];
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr =PatAddr | 8;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				TC =ReverseCHR[RenderData[3]];
				CurTileData =&TileData[Clockticks +9];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
				IncrementH();
				if (Clockticks ==255)
					IncrementV();
				break;
			case 327:	case 335:
				TC =ReverseCHR[RenderData[3]];
				CurTileData =&TileData[Clockticks -327];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				// Fall-through
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum =(Clockticks >> 1) &0x1C;
				TC =SprBuff[SprNum | 1];
				SprSL =(unsigned char)(SLnum -SprBuff[SprNum]);
 				if (Reg2000 &0x20) // 8x16 sprites
					PatAddr =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1));
				else	
					PatAddr =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9);
				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr =PatAddr;
				break;
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				SprNum =(Clockticks >> 1) &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[2];
				else	
					TC =ReverseCHR[RenderData[2]];
				TL =(SprBuff[SprNum] &0x3) * 0x04040404;
				CurTileData =SprData[SprNum >> 2];
				((unsigned long *)CurTileData)[0] =CHRLoBit[TC &0xF] | TL;
				((unsigned long *)CurTileData)[1] =CHRLoBit[TC >> 4] | TL;
				
				for (SprNum =(SprNum &~3) +8; SprNum <SprCount; SprNum +=8) {
					TC =SprBuff[SprNum | 1];
					SprSL =(unsigned char)(SLnum -SprBuff[SprNum]);
					unsigned long PatAddr2;
					if (Reg2000 &0x20)
						PatAddr2 =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1));
					else	
						PatAddr2 =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9);
					if (SprBuff[SprNum |2] &0x40)
						TC =CHRPointer[PatAddr2 >>10 &0xF][PatAddr2 &0x3FF];
					else	
						TC =ReverseCHR[CHRPointer[PatAddr2 >>10 &0xF][PatAddr2 &0x3FF]];
					TL =(SprBuff[SprNum |2] &0x3) * 0x04040404;
					CurTileData =SprData[SprNum >> 2];
					((unsigned long *)CurTileData)[0] =CHRLoBit[TC &0xF] | TL;
					((unsigned long *)CurTileData)[1] =CHRLoBit[TC >> 4] | TL;
				}
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr =PatAddr | 8;
				break;
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				SprNum =(Clockticks >> 1) &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[3];
				else	
					TC =ReverseCHR[RenderData[3]];
				CurTileData =SprData[SprNum >> 2];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
				CurTileData[8] =SprBuff[SprNum];
				CurTileData[9] =SprBuff[SprNum | 1];
				for (SprNum =(SprNum &~3) +8; SprNum <SprCount; SprNum +=8) {
					TC =SprBuff[SprNum | 1];
					SprSL =(unsigned char)(SLnum -SprBuff[SprNum]);
					unsigned long PatAddr2;
					if (Reg2000 &0x20)
						PatAddr2 =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1)) |8;
					else	
						PatAddr2 =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9) |8;
					if (SprBuff[SprNum |2] &0x40)
						TC =CHRPointer[PatAddr2 >>10 &0xF][PatAddr2 &0x3FF];
					else	
						TC =ReverseCHR[CHRPointer[PatAddr2 >>10 &0xF][PatAddr2 &0x3FF]];
					CurTileData =SprData[SprNum >> 2];
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
					CurTileData[8] =SprBuff[SprNum |2];
					CurTileData[9] =SprBuff[SprNum |3];
				}
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (!(Clockticks &1)) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2)
					WriteHandler[RenderAddr >> 10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF, RenderAddr &0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr =(unsigned short)(VRAMAddr &0x3FFF);
			if (IOMode >=5 && !IsRendering)
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else if (IOMode ==2) {
				if (!IsRendering) WriteHandler[addr >> 10](which <<6 | addr >>10, addr &0x3FF, IOVal);
			}
			else if (IOMode ==1) {
				IOMode++;
				if (!IsRendering) {
					if (ReadHandler[addr >>10] ==BusRead)
						buf2007 =CHRPointer[addr >>10][addr &0x3FF];
					else	buf2007 =(unsigned char)ReadHandler[addr >>10](which <<6 | addr >>10, addr &0x3FF);
				}
			}
			IOMode -=2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks ==255)) IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!((Clockticks &7) ==7 && !(255 <=Clockticks &&Clockticks <=319))) IncrementH();
				} else
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);
		
		register int PalIndex, DisplayedTC;
		if (Clockticks <256 && OnScreen) {
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 &0x02))
				TC =TileData[Clockticks +IntX];
			else	
				TC =0;
			DisplayedTC =showBG? TC: 0;
			
			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (y =0; y <SprCount; y +=4) {
				register int SprPixel =Clockticks -SprData[y >>2][9];
				register unsigned char SprDat;
				if (SprPixel &~7) continue;
				SprDat =SprData[y >>2][SprPixel];
				if (SprDat &0x3) {
					if (Spr0InLine && y ==0 && TC &0x3 && Clockticks <255) {
						Spr0Hit =1;
						Spr0InLine =FALSE;
					}
					if (showOBJ && !(TC &0x3 && SprData[y >> 2][8] &0x20)) DisplayedTC =SprDat |0x10;
					break;
				}
			}
			PalIndex =GetPalIndex(DisplayedTC);
			if (!IsRendering && (VRAMAddr &0x3F00) ==0x3F00) PalIndex =Palette[VRAMAddr &0x1F];
		} else
		if (SLnum <242) {
			if (Clockticks ==325)
				PalIndex =Palette[!IsRendering && (VRAMAddr &0x3F00)==0x3F00? VRAMAddr &0x1F: 0x00] &0x30;
			else
				PalIndex =Palette[!IsRendering && (VRAMAddr &0x3F00)==0x3F00? VRAMAddr &0x1F: 0x00];
		}
		/*pipeline =pipeline <<8 |PalIndex;
		PalIndex =pipeline >>24 &0xFF;*/
		PalIndex &=GrayScale;
		//if (NES::CurRegion ==Settings::REGION_PAL && (Clockticks <=1 || Clockticks >=254 || SLnum ==0 || SLnum >239)) PalIndex =0x0F;
		PalIndex |=ActualColorEmphasis;
		*GfxData++ =PalIndex;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) *GfxData++ =PalIndex;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

int	PPU_RP2C02::GetPalIndex(int TC) {
	if (!(TC &0x3)) TC =0;
	int PalIndex =Palette[TC &0x1F];
	return PalIndex;
}

int	PPU_VT01STN::GetPalIndex(int TC) {
	if (!(TC &0x3)) TC =0;
	return Palette[TC &0x1F] +PALETTE_VT01;
}

__inline void	PPU_RP2C02::RunSkip (int NumTicks)
{
	register unsigned char TC;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;
		if (Clockticks ==256) {
			if (SLnum <240) ZeroMemory(TileData, sizeof(TileData));if (SLnum <240) if (Spr0InLine)	ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 &&Clockticks <=303) {
			if ((IsRendering) &&(SLnum ==-1)) {
				VRAMAddr &=~0x7BE0;
				VRAMAddr |=IntReg &0x7BE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick )
					EndSLTicks =340;
				else	
					EndSLTicks =341;
			} else
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			} else
			if (SLnum ==SLStartNMI)	{
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[which]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (RI.ConsoleType ==CONSOLE_VS || RI.ConsoleType ==CONSOLE_PC10) ShortSL =0;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		else
			if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline
		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[(Clockticks >> 1) &3] =0;	// seems to force black in this case
					if (IOMode ==2)
						WriteHandler[RenderAddr >> 10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[(Clockticks >> 1) &3]);
				}
				else if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData[(Clockticks >> 1) &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF];
				else	
					RenderData[(Clockticks >> 1) &3] =(unsigned char)ReadHandler[RenderAddr >>10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =(RenderData[0] << 4) | (VRAMAddr >> 12) | ((Reg2000 &0x10) << 8);
				break;
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr =0x23C0 | (VRAMAddr &0xC00) | ((VRAMAddr &0x380) >> 4) | ((VRAMAddr &0x1C) >> 2);
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
			case 323:	case 331:
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				RenderAddr =PatAddr;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				if (Spr0InLine)	{
					TC =ReverseCHR[RenderData[2]];
					CurTileData =&TileData[Clockticks +11];
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >> 4];
				}
				break;
			case 325:	case 333:
				if (Spr0InLine) {
					TC =ReverseCHR[RenderData[2]];
					CurTileData =&TileData[Clockticks -325];
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >> 4];
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr =PatAddr | 8;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				if (Spr0InLine) {
					TC =ReverseCHR[RenderData[3]];
					CurTileData =&TileData[Clockticks +9];
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
				}
				IncrementH();
				if (Clockticks ==255)
					IncrementV();
				break;
			case 327:	case 335:
				if (Spr0InLine)	{
					TC =ReverseCHR[RenderData[3]];
					CurTileData =&TileData[Clockticks -327];
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum =(Clockticks >> 1) &0x1C;
				TC =SprBuff[SprNum | 1];
				SprSL =(unsigned char)(SLnum -SprBuff[SprNum]);
 				if (Reg2000 &0x20)
					PatAddr =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1));
				else	PatAddr =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9);
				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr =PatAddr;
				break;
			case 261:
				if (Spr0InLine) {
					if (SprBuff[2] &0x40)
						TC =RenderData[2];
					else	
						TC =ReverseCHR[RenderData[2]];
					((unsigned long *)SprData[0])[0] =CHRLoBit[TC &0xF];
					((unsigned long *)SprData[0])[1] =CHRLoBit[TC >> 4];
				}
					case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr =PatAddr | 8;
				break;
			case 263:
				if (Spr0InLine)	{
					if (SprBuff[2] &0x40)
						TC =RenderData[3];
					else	
						TC =ReverseCHR[RenderData[3]];
					((unsigned long *)SprData[0])[0] |=CHRHiBit[TC &0xF];
					((unsigned long *)SprData[0])[1] |=CHRHiBit[TC >> 4];
					SprData[0][8] =SprBuff[2];
					SprData[0][9] =SprBuff[3];
				}
					case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr =0x2000 | (VRAMAddr &0xFFF);
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (!(Clockticks &1)) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2)
					WriteHandler[RenderAddr >> 10](which <<6 | RenderAddr >>10, RenderAddr &0x3FF, RenderAddr &0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr =(unsigned short)(VRAMAddr &0x3FFF);
			if (IOMode >=5 && !IsRendering)
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else if (IOMode ==2) {
				if (!IsRendering) WriteHandler[addr >> 10](which <<6 | addr >>10, addr &0x3FF, IOVal);
			} else
			if (IOMode ==1) {
				IOMode++;
				if (!IsRendering) {
					if (ReadHandler[addr >>10] ==BusRead)
						buf2007 =CHRPointer[addr >>10][addr &0x3FF];
					else	buf2007 =(unsigned char)ReadHandler[addr >>10](which <<6 | addr >>10, addr &0x3FF);
				}
			}
			IOMode -=2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks ==255))
						IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!(((Clockticks &7) ==7) &&!(255 <=Clockticks &&Clockticks <=319)))
						IncrementH();
				} else
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);
		if (Spr0InLine && Clockticks <255 && OnScreen && (Reg2001 &0x18) ==0x18 && (Clockticks >=8 || Settings::PPUNeverClip || (Reg2001 &0x06) ==0x06)) {
			register int SprPixel =Clockticks -SprData[0][9];
			if (!(SprPixel &~7) &&(SprData[0][SprPixel] &0x3) &&(TileData[Clockticks +IntX] &0x3)) {
				//Reg2002 |=0x40;	// Sprite 0 hit
				Spr0Hit =1;
				Spr0InLine =FALSE;
			}
		}
		GfxData++;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) GfxData++;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}

void	PPU_RP2C02::RunAlign (int align) {
	RunSkip(align);
}

void	PPU_RP2C02::Run (void) {
	if (PALRatio) {
		register int cycles =3;
		if (++PALsubticks ==5) {
			PALsubticks =0;
			cycles =4;
		}
		if (GFX::FPSCnt < (Controllers::capsLock && !Controllers::scrollLock? 9: Settings::FSkip))
			RunSkip(cycles);
		else
			RunNoSkip(cycles);
	} else {
		if (GFX::FPSCnt < (Controllers::capsLock && !Controllers::scrollLock? 9: Settings::FSkip))
			RunSkip(3);
		else
			RunNoSkip(3);
	}
}

int	__fastcall	PPU_RP2C02::Read01356 (void) {
	return readLatch;
}

int	__fastcall	PPU_RP2C02::Read2 (void) {
	HVTog =TRUE;

	if (SLnum ==-1 && Clockticks ==0) Reg2002 &=~0x60;
	int result =Reg2002 | (readLatch &0x1F);
	Reg2002 &=~0x80;

	if (RI.ConsoleType <CONSOLE_VT02 && NES::CurRegion !=Settings::REGION_DENDY &&!Settings::IgnoreRaceCondition && SLnum ==SLStartNMI && Clockticks <3) {
		Reg2002 &=~0x80;
		CPU::CPU[which]->WantNMI =FALSE;
		if (Clockticks ==0) result =Reg2002 | (readLatch &0x1F);
	}	
	
	return readLatch =result;
}

int	__fastcall	PPU_RP2C02::Read2Vs (void) {
	register unsigned char tmp;
	HVTog =TRUE;
	tmp =Reg2002 | VsSecurity;
	if (tmp &0x80)
		Reg2002 &=0x60;
	// race conditions
	if (SLnum ==SLStartNMI) {
		if ((Clockticks ==0))
			tmp &=~0x80;
		if (Clockticks < 3)
			CPU::CPU[which]->WantNMI =FALSE;
	}
	return readLatch =tmp;
}
int	__fastcall	PPU_RP2C02::Read4 (void) {
	if (!Settings::DisableOAMData) {
		if (IsRendering) {
			readLatch =Sprite[SpritePtr];
			if (NES::CurRegion ==Settings::REGION_PAL) {
				if (Clockticks >259 && Clockticks <=323) readLatch |=0x80; // Micro Machines
			} else
				if (Clockticks >256 && Clockticks <=320) readLatch |=0x80; // Micro Machines
		} else	
			readLatch =Sprite[SprAddr];
	}
	return readLatch;
}

int	__fastcall	PPU_RP2C02::Read7 (void) {
	IOMode =5;
	if ((VRAMAddr &0x3F00) ==0x3F00) {
		readLatch &=0xC0;
		if (Reg2001 &0x01)
			return readLatch |=Palette[VRAMAddr &0x1F] &0x30;
		else	
			return readLatch |=Palette[VRAMAddr &0x1F];
	}
	else	return readLatch =buf2007;
}
typedef int (__fastcall PPU_RP2C02::*PPU_IntRead)(void);

int	PPU_RP2C02::IntRead (int Bank, int Addr) {
	const PPU_IntRead funcs[8] ={&PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read2,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read4,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read7};
	return (this->*funcs[Addr &7])();
}

int	PPU_RP2C02::IntReadVs (int Bank, int Addr) {
	const PPU_IntRead funcs[8] ={&PPU_RP2C02::Read01356,
	                              &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read2Vs,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read4,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read01356,
				      &PPU_RP2C02::Read7};
	return (this->*funcs[Addr &7])();
}

void	__fastcall	PPU_RP2C02::Write0 (int Val)
{
	if (Val &0x80 && ~Reg2000 &0x80 && Reg2002 &0x80 && (SLnum !=-1 || Clockticks !=0)) CPU::CPU[which]->WantNMI =TRUE;
	if (SLnum ==SLStartNMI && Clockticks <3) CPU::CPU[which]->WantNMI =FALSE;
#ifdef	ENABLE_DEBUGGER
	if ((Reg2000 ^ Val) &0x28) Debugger::SprChanged =TRUE;
	if ((Reg2000 ^ Val) &0x10) Debugger::NTabChanged =TRUE;
#endif	/* ENABLE_DEBUGGER */
	Reg2000 =(unsigned char)Val;
	IntReg &=0x73FF;
	IntReg |=(Val &3) << 10;
	if (Settings::upByOne) IntReg ^=UpByOnePage;
}

void	__fastcall	PPU_RP2C02::Write1 (int Val) {
	Reg2001 =(unsigned char)Val;
	// Apparently, sprite evaluation prevents the IsRendering change from being applied immediately.
	// Therefore, apply it immediately in the pre-render scanline (where sprite evaluation does not occur yet),
	// and delay it for one cycle otherwise.
	if (SLnum ==-1)
		IsRendering =Reg2001 &0x18 && SLnum <240;
	else
		UpdateIsRendering =1;
	ColorEmphasis =(Val &0xE0) << 1;
	ActualColorEmphasis =(NES::CurRegion ==Settings::REGION_PAL || NES::CurRegion ==Settings::REGION_DENDY && Settings::DendySwapEmphasis)? ((ColorEmphasis &0x80) >>1) | ((ColorEmphasis &0x40) <<1) | (ColorEmphasis &0x100) :ColorEmphasis;
	if (Settings::DisableEmphasis) ActualColorEmphasis =0;
	GrayScale =(Val &0x01) ? 0xFFFFFFF0: 0xFFFFFFFF;
}

void	__fastcall	PPU_RP2C02::Write2 (int Val) {
}

void	__fastcall	PPU_RP2C02::Write3 (int Val) {
	SprAddr =(unsigned char)Val;
}

void	__fastcall	PPU_RP2C02::Write4 (int Val) {
	if ((SprAddr &0x03) ==0x02)
		Val &=0xE3;
	if (IsRendering) {
		SprAddr =((SprAddr +1) &~3) | (SprAddr &3);
	} else {
		Sprite[SprAddr] =(unsigned char)Val;
		SprAddr =(SprAddr +1) &0xFF;
	}
#ifdef	ENABLE_DEBUGGER
	Debugger::SprChanged =TRUE;
#endif	/* ENABLE_DEBUGGER */
}

void	__fastcall	PPU_RP2C02::Write5 (int Val) {
	if (HVTog) {
		IntReg &=0x7FE0;
		IntReg |=(Val &0xF8) >> 3;
		IntX =(unsigned char)(Val &7);
	} else {
		IntReg &=0x0C1F;
		IntReg |=(Val &0x07) << 12;
		IntReg |=(Val &0xF8) << 2;
		UpByOnePage =0;
		if (Settings::upByOne) {
			IntReg =IntReg &~0x7000 | (IntReg +0x1000) &0x7000;
			if ((IntReg &0x7000) ==0x0000) {
				IntReg =IntReg &~0x3E0 | (IntReg +0x20) &0x3E0;
				if ((IntReg &0x3E0) ==0x03C0) { IntReg ^=0x0BC0; UpByOnePage =0x800; }
			}
		}
	}
	HVTog =!HVTog;
}

void	__fastcall	PPU_RP2C02::Write6 (int Val)
{
	if (HVTog) {
		IntReg &=0x00FF;
		IntReg |=(Val &0x3F) << 8;
	} else {
		IntReg &=0x7F00;
		IntReg |=Val;
		#if DELAY_VRAM_ADDRESS_CHANGE
		if (!IsRendering)
			VRAMAddr =IntReg;
		else
			UpdateVRAMAddr =3;
		#else
		if (Settings::ScrollGlitch && IsRendering && SLnum <240) {
			if (Clockticks ==257)
				VRAMAddr &=IntReg;
			else
			if (Clockticks >0 && (Clockticks <=256 || Clockticks >320) && (Clockticks &7) ==0)
				VRAMAddr =(IntReg &~0x41F) | (VRAMAddr &IntReg &0x41F);
			else
				VRAMAddr =IntReg;
			IntReg =VRAMAddr;
		} else
			VRAMAddr =IntReg;
		#endif
	}
	HVTog =!HVTog;
}

void	__fastcall	PPU_RP2C02::Write7 (int Val) {
	register unsigned char Addr;
	if ((VRAMAddr &0x3F00) ==0x3F00 && !IsRendering) {
		Val &=0x3F;
		Addr =(unsigned char) VRAMAddr &0x1F;
		Palette[Addr] =(unsigned char)Val;

#ifdef	ENABLE_DEBUGGER
		Debugger::PalChanged =TRUE;
#endif	/* ENABLE_DEBUGGER */
		if (!(Addr &0x03)) Palette[Addr ^0x10] =(unsigned char)Val;
		// If a mapper monitors PPU accesses, its write handler needs to be called
		if (WriteHandler[0xF] !=BusWriteNT) (WriteHandler[0xF])(0xF, VRAMAddr, Val);
		IncrementAddr();
	} else {
		IOVal =(unsigned char)Val;
		IOMode =6;
	}
}

typedef void (__fastcall PPU_RP2C02::*PPU_IntWrite)(int Val);
void	PPU_RP2C02::IntWrite (int Bank, int Addr, int Val) {
	const PPU_IntWrite funcs[8] ={&PPU_RP2C02::Write0,
	                               &PPU_RP2C02::Write1,
				       &PPU_RP2C02::Write2,
				       &PPU_RP2C02::Write3,
				       &PPU_RP2C02::Write4,
				       &PPU_RP2C02::Write5,
				       &PPU_RP2C02::Write6,
				       &PPU_RP2C02::Write7};
	readLatch =(unsigned char)Val;
	(this->*funcs[Addr &7])(Val);
}

void	PPU_RP2C02::IntWriteVs (int Bank, int Addr, int Val) {
	const PPU_IntWrite funcs[8] ={&PPU_RP2C02::Write1,
	                               &PPU_RP2C02::Write0,
				       &PPU_RP2C02::Write2,
				       &PPU_RP2C02::Write3,
				       &PPU_RP2C02::Write4,
				       &PPU_RP2C02::Write5,
				       &PPU_RP2C02::Write6,
				       &PPU_RP2C02::Write7};
	readLatch =(unsigned char)Val;
	(this->*funcs[Addr &7])(Val);
}

PPU_RP2C02 *PPU[2];

void	SetRegion (void) {
	if (PPU[0]) PPU[0]->SetRegion();
	if (PPU[1]) PPU[1]->SetRegion();
}

void	PowerOn (void) {
	if (PPU[0]) PPU[0]->PowerOn();
	if (PPU[1]) PPU[1]->PowerOn();
}

void	Reset (void) {
	if (PPU[0]) PPU[0]->Reset();
	if (PPU[1]) PPU[1]->Reset();
}

int	MAPINT	IntRead (int Bank, int Addr) {
	return PPU[Bank >>4]->IntRead(Bank &0xF, Addr);
}

int	MAPINT	IntReadVs (int Bank, int Addr) {
	return PPU[Bank >>4]->IntReadVs(Bank &0xF, Addr);
}

void	MAPINT	IntWrite (int Bank, int Addr, int Val) {
	PPU[Bank >>4]->IntWrite(Bank &0xF, Addr, Val);
}

void	MAPINT	IntWriteVs (int Bank, int Addr, int Val) {
	PPU[Bank >>4]->IntWriteVs(Bank &0xF, Addr, Val);
}

int	MAPINT	ReadUnsafe (int Bank, int Addr) {
	return 0;
}

int	MAPINT	BusRead (int Bank, int Addr) {
	return PPU[Bank >>6]->CHRPointer[Bank &0x3F][Addr];
}

void	MAPINT	BusWriteCHR (int Bank, int Addr, int Val) {
	if (!PPU[Bank >>6]->Writable[Bank &0x3F]) return;
#ifdef	ENABLE_DEBUGGER
	Debugger::PatChanged =TRUE;
#endif	/* ENABLE_DEBUGGER */
	PPU[Bank >>6]->CHRPointer[Bank &0x3F][Addr] =(unsigned char)Val;
}

void	MAPINT	BusWriteNT (int Bank, int Addr, int Val) {
	if (!PPU[Bank >>6]->Writable[Bank &0x3F]) return;
#ifdef	ENABLE_DEBUGGER
	Debugger::NTabChanged =TRUE;
#endif	/* ENABLE_DEBUGGER */
	PPU[Bank >>6]->CHRPointer[Bank &0x3F][Addr] =(unsigned char)Val;
}

} // namespace PPU