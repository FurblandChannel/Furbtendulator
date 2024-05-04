#pragma once
#include "Filter.h"

#ifndef	NSFPLAYER
#endif	/* !NSFPLAYER */

namespace APU {
extern Filter::Butterworth filterPCM;
extern Filter::LPF_RC filterPulse;
extern const double squareSumFactor[32];

class Channel {
public:
	unsigned char wavehold, next_wavehold, LengthCtr1, LengthCtr2;
	int LengthCtr;
	Channel();
	virtual void Reset();
	virtual void Run();
};
class CSquare: public Channel {
public:
	int whichSquare;
	unsigned char volume, envelope, duty, swpspeed, swpdir, swpstep, swpenab;
	unsigned long freq;
	unsigned char Vol;
	unsigned char CurD;
	unsigned char EnvCtr, Envelope, BendCtr;
	BOOL Enabled, ValidFreq, Active;
	BOOL EnvClk, SwpClk;
	unsigned long Cycles;
	signed long Pos;
	CSquare() {}
	CSquare(int _which);
	void	PowerOn ();
	void	Reset ();
	void	CheckActive (void);
	void	Write (int Reg, unsigned char Val);
	void	Run (void);
	void	QuarterFrame (void);
	void	HalfFrame (void);
};
class CTriangle: public Channel {
public:
	unsigned char linear;
	unsigned long freq;
	unsigned char CurD;
	unsigned char LinCtr;
	BOOL Enabled, Active;
	BOOL LinClk;
	unsigned long Cycles;
	signed long Pos;
	CTriangle();
	void	PowerOn (void);
	void	Reset (void);
	void	CheckActive (void);
	void	Write (int Reg, unsigned char Val);
	void	Run (void);
	void	QuarterFrame (void);
	void	HalfFrame (void);
};

class CNoise: public Channel {
public:
	unsigned char volume, envelope, datatype;
	unsigned long freq;
	unsigned long CurD;
	unsigned char Vol;
	unsigned char EnvCtr, Envelope;
	BOOL Enabled;
	BOOL EnvClk;
	unsigned long Cycles;
	signed long Pos;
	const unsigned long *FreqTable;
	CNoise();
	void	PowerOn (void);
	void	Reset (void);
	void	Write (int Reg, unsigned char Val);
	void	Run (void);
	void	QuarterFrame (void);
	void	HalfFrame (void);
};

class CDPCM: public Channel {
public:
	int whichAPU;
	unsigned char freq, doirq, pcmdata, addr, len;
	unsigned long CurAddr, SampleLen;
	BOOL silenced, bufempty, fetching;
	unsigned char shiftreg, outbits, buffer;
	unsigned long Cycles;
	signed long Pos;
	unsigned short baseAddr;
	const	unsigned long	*FreqTable;
	int stop;
	CDPCM() {}
	CDPCM(int _which);
	void	PowerOn (void);
	virtual	void	Reset (void);
	virtual	void	Write (int Reg, unsigned char Val);
	virtual	void	Run (int);
	void	Fetch (void);
};

class APU_RP2A03 {
public:
	CSquare square0, square1;
	CTriangle triangle;
	CNoise noise;
	CDPCM *dpcm;

	int which, numRegs;
	unsigned char Bits;
	int Cycles;
	int Quarter, Half, IRQ, Zero;
	const	int	*CycleTable;	
	double	Output;
	#if SUPPORT_STEREO
	double	OutputL, OutputR;
	#endif
	BYTE	Regs[0x40];
	int	InternalClock;

	APU_RP2A03 (int, int);
	APU_RP2A03 (int);
	APU_RP2A03 ();
	~APU_RP2A03 ();
	void	WriteFrame (unsigned char Val);
	virtual void	RunFrame (void);
	virtual void	IntWrite (int Bank, int Addr, int Val);
	virtual int	IntRead (int Bank, int Addr);
	virtual void	SetRegion (void);
	virtual void	PowerOn  (void);
	virtual void	Reset  (void);
	virtual int	Save (FILE *out);
	virtual int	Load (FILE *in, int version_id);
	virtual void	Run (void);
};

extern APU_RP2A03 *APU[2];

void	SetRegion (void);
void	PowerOn (void);
void	Reset (void);
int	MAPINT	IntRead (int, int);
void	MAPINT	IntWrite (int, int, int);
} // namespace APU
