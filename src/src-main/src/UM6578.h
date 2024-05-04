#pragma once
#include <queue>

extern uint8_t um6578extraRAM[2048];
namespace CPU {
class CPU_UM6578: public CPU_RP2A03 {
public:
	uint8_t * const RAM2;	
	void	PowerOn (void) override;
	int	Save (FILE *) override;
	int	Load (FILE *, int) override;
	int	ReadRAM2 (int, int);
	void	WriteRAM2 (int, int, int);
	CPU_UM6578(uint8_t*, uint8_t*);
};

int	MAPINT	ReadRAM2 (int, int);
void	MAPINT	WriteRAM2 (int, int, int);
} // namespace CPU

namespace PPU {
class	PPU_UM6578: public PPU_RP2C02 {
public:
	uint8_t reg2008;
	uint8_t colorMask;
	virtual	int             IntRead     (int, int)      override;
	virtual	void            IntWrite    (int, int, int) override;
	virtual void __fastcall Write6      (int)           override;
	virtual int  __fastcall Read7       (void)          override;
	virtual void __fastcall Write7      (int)           override;
	virtual void            RunNoSkip   (int NumTicks)  override;
	virtual void            RunSkip     (int NumTicks)  override;
	virtual int             GetPalIndex (int)           override;
	virtual	void            IncrementV    () override;
	virtual	void            IncrementAddr () override;
	int	Save (FILE *) override;
	int	Load (FILE *, int ver) override;
};
} // namespace PPU

namespace APU {
class APU_UM6578: public APU_RP2A03 {
	uint8_t  dmaControl;
	uint8_t  dmaPage;
	uint16_t dmaSourceAddress;
	uint16_t dmaTargetAddress;
	uint16_t dmaLength;
	uint32_t dmaBusyCount;
	std::queue<uint8_t> keyboardBuffer;
public:
	APU_UM6578(): dmaControl(0), dmaPage(0), dmaSourceAddress(0), dmaTargetAddress(0), dmaLength(0) { };
	virtual void	IntWrite (int Bank, int Addr, int Val) override;
	virtual int	IntRead (int Bank, int Addr) override;
	virtual void	Reset  (void) override;
	virtual int	Save (FILE *out) override;
	virtual int	Load (FILE *in, int version_id) override;
	virtual void	Run (void) override;
	virtual void    handleWindowMessage (UINT message, WPARAM wParam, LPARAM lParam);
};	
} // namespace APU