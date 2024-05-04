#pragma once

#include "OneBus.h"

extern uint8_t vt369SoundRAM[8192];
struct VT369Sprite {
	int     startX;
	uint8_t data[16];
	bool    priority;
	bool	sprite0;
};
struct VT369SpriteHi {
	int     startX;
	int	maskX;
	uint8_t dataEven[64];
	uint8_t dataOdd [64];
	bool    priority;
	bool	sprite0;
};
extern std::vector<VT369Sprite>    vt369Sprites;
extern std::vector<VT369SpriteHi>  vt369SpritesHi;


namespace CPU {
class CPU_VT369: public CPU_OneBus {
public:
	CPU_VT369(uint8_t *);
	void RunCycle (void);
	unsigned char	MemGet (unsigned int);
};

class CPU_VT369_Sound: public CPU_RP2A03 {
	int	dataBank;
	int	prescaler;
	BOOL	LastTimerIRQ;
	BOOL	WantTimerIRQ;
	uint16_t summerAddr;
	int32_t  summerResult;
	uint16_t multiplierAddr;
	int32_t	 multiplierResult;
public:
	CPU_VT369_Sound(uint8_t *);
	void	ExecOp (void) override;
	void	RunCycle (void) override;
	void	Reset (void) override;
	int	readReg (int);
	void	writeReg (int, int);
	void	DoTimerIRQ (void);
	uint8_t	MemGet (unsigned int);
	int	Save (FILE *) override;
	int	Load (FILE *, int ver) override;
__forceinline	void	IN_ADX (void);
__forceinline	void	IN_LDAXD (void);
__forceinline	void	IN_LDAD (void);
__forceinline	void	IN_PHX (void);
__forceinline	void	IN_PLX (void);
__forceinline	void	IN_PHY (void);
__forceinline	void	IN_PLY (void);
__forceinline	void	IN_TAD (void);
__forceinline	void	IN_TDA (void);
};

int	MAPINT	readVT369Sound (int bank, int addr);
int	MAPINT	readVT369SoundRAM (int bank, int addr);
void	MAPINT	writeVT369Sound (int bank, int addr, int val);
void	MAPINT	writeVT369Palette (int bank, int addr, int val);

} // namespace CPU

namespace APU {
class APU_VT369: public APU_RP2A03 {
	uint32_t  aluOperand14;
	uint16_t  aluOperand56;
	uint16_t  aluOperand67;
	uint8_t   aluBusy;
public:
	void	IntWrite (int Bank, int Addr, int Val) override;
	int	IntRead (int Bank, int Addr) override;
	void	PowerOn (void) override;
	void	Reset (void) override;
	int	Save (FILE *out) override;
	int	Load (FILE *in, int version_id) override;
	void	Run (void) override;
};

} // namespace APU

namespace PPU {
class	PPU_VT369: public PPU_OneBus {
public:
	uint8_t	SprAddrHigh;
	PPU_VT369(): PPU_OneBus() { }
	int	GetPalIndex (int) override;
        void	Reset (void) override;
        void	IntWrite (int, int, int) override;
	void	ProcessSpritesEnhanced (void);
	void	ProcessSpritesHiRes (void);
	void    Run (void) override;
	void	RunNoSkipEnhanced (int);
	void	RunSkipEnhanced (int);
	void	RunNoSkipHiRes (int);
	int	Save (FILE *) override;
	int	Load (FILE *, int ver) override;
	unsigned short *GfxDataEven;
	unsigned short *GfxDataOdd;
	unsigned short	DrawArrayEven[341*2 *312];
	unsigned short	DrawArrayOdd [341*2 *312];
	void	GetGFXPtr (void) override;
	unsigned char TileDataEven[272*2];
	unsigned char TileDataOdd [272*2];
};
} // namespace PPU