#pragma once

#define	IRQ_FRAME	0x01
#define	IRQ_DPCM	0x02
#define	IRQ_EXTERNAL	0x04
#define	IRQ_DEBUG	0x08

#ifdef	ENABLE_DEBUGGER
#define	INTERRUPT_NMI	1
#define	INTERRUPT_RST	2
#define	INTERRUPT_IRQ	3
#define	INTERRUPT_BRK	4
#endif	/* ENABLE_DEBUGGER */

#define DMA_PCM		0x01
#define	DMA_PCM_LOAD	0x02
#define DMA_IN_PCM	0x04
#define	DMA_SPR		0x08
#define	DMA_IN_SPR	0x10
#define DMA_HALT	0x20

#define PC rPC.Full
#define PCL rPC.Segment[0]
#define PCH rPC.Segment[1]

namespace CPU {
class CPU_RP2A03 {
	union SplitReg { unsigned short Full; unsigned char Segment[2]; };	
protected:
	int	which;
	SplitReg rCalcAddr;
	SplitReg rTmpAddr;
	unsigned char BranchOffset;
	unsigned char Opcode;
	unsigned short OpAddr;
	BOOL	LastNMI;
	BOOL	LastIRQ;
	BOOL	WantTimerIRQ;

void	DoNMI (void);
void	DoIRQ (void);
virtual uint8_t	GetOpcode(void);
__forceinline	void	AM_IMP (void);
__forceinline	void	AM_IMM (void);
__forceinline	void	AM_ABS (void);
__forceinline	void	AM_REL (void);
__forceinline	void	AM_ABX (void);
__forceinline	void	AM_ABXW (void);
__forceinline	void	AM_ABY (void);
__forceinline	void	AM_ABYW (void);
__forceinline	void	AM_ZPG (void);
__forceinline	void	AM_ZPX (void);
__forceinline	void	AM_ZPY (void);
__forceinline	void	AM_INX (void);
__forceinline	void	AM_INY (void);
__forceinline	void	AM_INYW (void);
__forceinline	void	AM_NON (void);
	virtual	void	IN_ADC (void);
__forceinline	void	IN_AND (void);
__forceinline	void	IN_ASL (void);
__forceinline	void	IN_ASLA(void);
void	IN_BRANCH (bool Condition);
__forceinline	void	IN_BCS (void);
__forceinline	void	IN_BCC (void);
__forceinline	void	IN_BEQ (void);
__forceinline	void	IN_BNE (void);
__forceinline	void	IN_BMI (void);
__forceinline	void	IN_BPL (void);
__forceinline	void	IN_BVS (void);
__forceinline	void	IN_BVC (void);
__forceinline	void	IN_BIT (void);
__forceinline	void	IN_BRK (void);
__forceinline	void	IN_CLC (void);
__forceinline	void	IN_CLD (void);
__forceinline	void	IN_CLI (void);
__forceinline	void	IN_CLV (void);
__forceinline	void	IN_CMP (void);
__forceinline	void	IN_CPX (void);
__forceinline	void	IN_CPY (void);
__forceinline	void	IN_DEC (void);
__forceinline	void	IN_DEX (void);
__forceinline	void	IN_DEY (void);
__forceinline	void	IN_EOR (void);
__forceinline	void	IN_INC (void);
__forceinline	void	IN_INX (void);
__forceinline	void	IN_INY (void);
	virtual	void	IN_JMP (void);
	virtual	void	IN_JMPI (void);
__forceinline	void	IN_JSR (void);
__forceinline	void	IN_LDA (void);
__forceinline	void	IN_LDX (void);
__forceinline	void	IN_LDY (void);
__forceinline	void	IN_LSR (void);
__forceinline	void	IN_LSRA (void);
__forceinline	void	IN_NOP (void);
__forceinline	void	IN_ORA (void);
__forceinline	void	IN_PHA (void);
__forceinline	void	IN_PHP (void);
__forceinline	void	IN_PLA (void);
__forceinline	void	IN_PLP (void);
__forceinline	void	IN_ROL (void);
__forceinline	void	IN_ROLA (void);
__forceinline	void	IN_ROR (void);
__forceinline	void	IN_RORA (void);
__forceinline	void	IN_RTI (void);
	virtual	void	IN_SBC (void);
__forceinline	void	IN_SEC (void);
__forceinline	void	IN_SED (void);
__forceinline	void	IN_SEI (void);
__forceinline	void	IN_STA (void);
__forceinline	void	IN_STX (void);
__forceinline	void	IN_STY (void);
__forceinline	void	IN_TAX (void);
__forceinline	void	IN_TAY (void);
__forceinline	void	IN_TSX (void);
__forceinline	void	IN_TXA (void);
__forceinline	void	IN_TXS (void);
__forceinline	void	IN_TYA (void);
__forceinline	void	IV_XAA (void);
__forceinline	void	IV_UNK (void);
__forceinline	void	IV_HLT (void);
__forceinline	void	IV_NOP (void);
__forceinline	void	IV_NOP2 (void);
__forceinline	void	IV_SLO (void);
__forceinline	void	IV_RLA (void);
__forceinline	void	IV_SRE (void);
__forceinline	void	IV_RRA (void);
__forceinline	void	IV_SAX (void);
__forceinline	void	IV_LAX (void);
__forceinline	void	IV_DCP (void);
__forceinline	void	IV_ISB (void);
__forceinline	void	IV_SBC (void);
__forceinline	void	IV_AAC (void);
__forceinline	void	IV_ASR (void);
__forceinline	void	IV_ARR (void);
__forceinline	void	IV_ATX (void);
__forceinline	void	IV_AXS (void);

public:
	CPU_RP2A03 (int, int, uint8_t*);
	CPU_RP2A03 (int, uint8_t*);
	CPU_RP2A03 (uint8_t*);
	void (MAPINT *CPUCycle) (void);
virtual void RunCycle (void);
	static void MAPINT NoCPUCycle (void) { }
__forceinline	void	Push (unsigned char Val);
__forceinline	unsigned char	Pull (void);
__forceinline	void	IN_RTS (void);
	
	int	RAMSize, RAMMask;
	uint8_t * const RAM;
	FCPURead ReadHandler[0x10], ReadHandlerDebug[0x10];
	FCPUWrite WriteHandler[0x10];
	unsigned char *PRGPointer[0x10];
	BOOL Readable[0x10], Writable[0x10];
	
	uint64_t CycleCount;
	unsigned char WantNMI;
	unsigned char WantIRQ;
	unsigned char EnableDMA;
	unsigned char DMAPage;
	unsigned char DMAMiddleAddr;
	unsigned short DMALength;
	unsigned short DMATarget;
	unsigned char OAMByte;
	int scheduleDMC;
	int OAMCount;
	bool fetching;
	int DMAStateDMC;
	int DMAStateOAM;
#ifdef	ENABLE_DEBUGGER
	unsigned char GotInterrupt;
#endif	/* ENABLE_DEBUGGER */

	unsigned char A, X, Y, SP, P;
	bool FC, FZ, FI, FD, FV, FN;
	unsigned char LastRead;
	SplitReg rPC;

virtual unsigned char	MemGet (unsigned int);
virtual unsigned char	MemGetA (unsigned int);
void	MemSet (unsigned int, unsigned char);

void	JoinFlags (void);
void	SplitFlags (void);

void	GetHandlers (void);
virtual	void	Reset (void);
virtual	void	PowerOn (void);
void	Unload (void);
virtual	int	Save (FILE *);
virtual	int	Load (FILE *, int ver);
virtual void	ExecOp (void);
int	ReadRAM (int, int);
void	WriteRAM (int, int, int);
}; // class CPU_2A03


class CPU_RP2A03_Decimal: public CPU_RP2A03 {
protected:
	virtual	void	IN_ADC (void) override;
	virtual	void	IN_SBC (void) override;
public:
	CPU_RP2A03_Decimal(uint8_t*);
	CPU_RP2A03_Decimal (int, int, uint8_t*);
};

extern CPU_RP2A03* CPU[2];
int	MAPINT	ReadUnsafe (int, int);
int	MAPINT	ReadRAM (int, int);
void	MAPINT	WriteRAM (int, int, int);
int	MAPINT	ReadPRG (int, int);
void	MAPINT	WritePRG (int, int, int);
void	PowerOn (void);
void	Reset (void);
void	Unload (void);
extern	int	RAMInitialization;
extern	void	RandomMemory(void* Destination, size_t Length);

struct Callback {
	int		which;
	uint16_t	pc;
	void		(*handler)(void);
};
extern std::vector<Callback> callbacks;

}