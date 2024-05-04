#pragma once
#include <vector>
#include <queue>

#define PIX16EN 0x01
#define BK16EN 0x02
#define SP16EN 0x04
#define SPEXTEN 0x08
#define BKEXTEN 0x10
#define SPOPEN 0x20
#define V16BEN 0x40
#define COLCOMP 0x80

extern	uint8_t	reg2000[256];
extern	uint8_t	reg4100[256];

namespace CPU {
class CPU_OneBus: public CPU_RP2A03 {
protected:
	uint8_t	GetOpcode(void) override;
	void	PowerOn (void) override;
	void	Reset (void) override;
	int	Save (FILE *) override;
	int	Load (FILE *, int) override;
public:
	CPU_OneBus(uint8_t *);
	CPU_OneBus(int, int, uint8_t *);
	uint8_t Unscramble(uint8_t raw);
	bool	EncryptionStatus;
};

class CPU_VT09: public CPU_OneBus {
public:
	CPU_VT09(uint8_t *);
};
} // namespace CPU

namespace APU {
class CDPCM_OneBus: public CDPCM {
public:
	bool	RawPCM;
	CDPCM_OneBus();
	void	Reset (void) override;
	void	Write (int Reg, unsigned char Val) override;
	void	Run (int) override;
};

class APU_OneBus: public APU_RP2A03 {
public:
	CSquare   square2, square3;
	CTriangle triangle2;
	CNoise    noise2;
	
	APU_OneBus();
	void	RunFrame (void) override;
	void	SetRegion (void) override;
	virtual void	IntWrite (int Bank, int Addr, int Val) override;
	virtual int	IntRead (int Bank, int Addr) override;
	virtual void	PowerOn  (void) override;
	virtual void	Reset  (void) override;
	virtual int	Save (FILE *out) override;
	virtual int	Load (FILE *in, int version_id) override;
	virtual void	Run (void) override;
};
} // namespace APU

namespace PPU {
class	PPU_OneBus: public PPU_RP2C02 {
public:
	PPU_OneBus();
	int	EVA;
	int	ReadType;
virtual	void	Reset (void) override;
	void	RunNoSkip (int NumTicks) override;
	void	RunSkip (int NumTicks) override;
virtual	int	Save (FILE *) override;
virtual int	Load (FILE *, int ver) override;
virtual	int	IntRead (int, int) override;
virtual	void	IntWrite (int, int, int) override;
};

class	PPU_VT03: public PPU_OneBus {
public:
	PPU_VT03(): PPU_OneBus() { }
	int	GetPalIndex (int) override;

};
} // namespace PPU