#pragma once

namespace PPU {
extern bool showBG;
extern bool showOBJ;
extern const unsigned char ReverseCHR[256];
extern const unsigned long CHRLoBit[16];
extern const unsigned long CHRHiBit[16];
extern unsigned char OpenBus[0x400];
	
class PPU_RP2C02 {
public:	
	int	which;
	uint32_t pipeline;
	int	NextBank, NextAddr, NextVal;
	FPPURead	ReadHandler[0x40];
	FPPURead	ReadHandlerDebug[0x40];
	FPPUWrite	WriteHandler[0x40];
	int Clockticks;
	int SLStartNMI;
	int SLEndFrame;
	int SLnum;
	unsigned char *CHRPointer[0x40];
	BOOL Writable[0x40];
	unsigned char Sprite[0x201]; // 0x121
	unsigned char SprAddr;
	unsigned char Palette[1024]; // 32 for NES, 64 for UM6578, 256 for VT03, 1024 for VT369
	unsigned char readLatch;
	unsigned char Reg2000, Reg2001, Reg2002;
	unsigned char HVTog, ShortSL, IsRendering, OnScreen, UpdateIsRendering;
	unsigned long VRAMAddr, IntReg, UpdateVRAMAddr, UpByOnePage;
	unsigned char IntX;
	unsigned char TileData[272];
	BOOL PALRatio, SkipTick;
	unsigned char PALsubticks;
	unsigned char	VRAM[0xA][0x400]; // 10 KiB for UM6578; normal NES will only use 4 KiB
	unsigned short	DrawArray[341*312];
	unsigned char VsSecurity;
	unsigned long GrayScale, GrayScaleDelay;
	unsigned long ColorEmphasis;	// ORed with palette index (upper 8 bits of $2001 shifted left 1 bit)
	unsigned long ActualColorEmphasis; // With PAL bit flip applied, if necessary
	unsigned long RenderAddr;
	unsigned char IOVal;
	unsigned char IOMode;	// Start at 6 for writes, 5 for reads - counts down and eventually hits zero
	unsigned char buf2007;
	unsigned char *SprBuff; // Pointer to secondary OAM. Basically &Sprite[256].
	BOOL Spr0InLine;
	int SprCount, Spr0Hit;
	unsigned char SprData[64][18]; // Secondary OAM but with Tile Data. 8 entries with scanline limit, 64 without.
	unsigned short *GfxData;
	int	SprAddrH, SpritePtr;
	int sprstate, sprcount, sprzero;
	unsigned char sprtmp;
	int EndSLTicks;
	unsigned long PatAddr;
	unsigned char RenderData[4];
	unsigned char RenderDataHi[4];
	
	void	(MAPINT *PPUCycle)		(int,int,int,int);	
	PPU_RP2C02(int);
	PPU_RP2C02();
	virtual void	ProcessSprites (void);
	inline void IncrementH ();
	virtual void IncrementV ();
	virtual	void    IncrementAddr ();
	virtual int	GetPalIndex (int);
	virtual	void	RunNoSkip (int NumTicks);
	virtual	void	RunSkip (int NumTicks);
int	__fastcall	Read01356 (void);
int	__fastcall	Read2 (void);
int	__fastcall	Read2Vs (void);
int	__fastcall	Read4 (void);
virtual int	__fastcall	Read7 (void);
void	__fastcall	Write0 (int Val);
void	__fastcall	Write1 (int Val);
void	__fastcall	Write2 (int Val);
void	__fastcall	Write3 (int Val);
void	__fastcall	Write4 (int Val);
void	__fastcall	Write5 (int Val);
virtual void	__fastcall	Write6 (int Val);
virtual void	__fastcall	Write7 (int Val);
	void	GetHandlers (void);
	void	SetRegion (void);
	void	PowerOn (void);
virtual	void	Reset (void);
	virtual	int	Save (FILE *);
	virtual	int	Load (FILE *, int ver);
	virtual	int	IntRead (int, int);
	int	MAPINT	IntReadVs (int, int);
	virtual	void	IntWrite (int, int, int);
	void	MAPINT	IntWriteVs (int, int, int);
	virtual	void	Run (void);
	void	RunAlign (int);
	virtual void	GetGFXPtr (void);
};

class	PPU_Dummy: public PPU_RP2C02 {
public:
	PPU_Dummy(int which): PPU_RP2C02(which) { }
	void	Run (void) override { } ;
};

class	PPU_VT01STN: public PPU_RP2C02 {
public:
	PPU_VT01STN(): PPU_RP2C02() { }
	int	GetPalIndex (int) override;	
};

extern PPU_RP2C02 *PPU[2];

void	SetRegion (void);
void	PowerOn (void);
void	Reset (void);
int	MAPINT	IntRead (int, int);
int	MAPINT	IntReadVs (int, int);
void	MAPINT	IntWrite (int, int, int);
void	MAPINT	IntWriteVs (int, int, int);
int	MAPINT	ReadUnsafe (int, int);
int	MAPINT	BusRead (int, int);
void	MAPINT	BusWriteCHR (int, int, int);
void	MAPINT	BusWriteNT (int, int, int);
} // namespace PPU
