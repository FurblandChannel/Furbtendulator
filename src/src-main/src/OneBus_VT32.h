#pragma once
namespace CPU {
class CPU_VT32: public CPU_RP2A03 {
protected:
	void	ResetEncryption(void);
	uint8_t	GetOpcode(void) override;
	void	IN_JMP(void) override;
	void	IN_JMPI(void) override;
	void	PowerOn (void) override;
	void	Reset (void) override;
	int	Save (FILE *) override;
	int	Load (FILE *, int) override;
public:
	CPU_VT32(uint8_t *);
	bool	EncryptionStatus;
	bool	NextEncryptionStatus;
	bool	ChangeEncryptionStatus;
};
} // namespace CPU

namespace APU {
class APU_VT32: public APU_RP2A03 {
public:
	uint32_t address;
	struct Channel {
		int32_t count;
		uint32_t period;
		uint32_t address;
		int16_t sample;
		uint8_t volume;
		bool playing;
		bool adpcm;
		
		int predictor;
		int stepIndex;
		int stepSize;
		bool secondNibble;

		void reset (void);
		void start (int, uint32_t);
		void stop  (void);
		void run   (void);
		int output (void);
	} channels[2];
	uint32_t  aluOperand14;
	uint16_t  aluOperand56;
	uint16_t  aluOperand67;
	uint8_t   aluBusy;

	APU_VT32();
	void	IntWrite (int Bank, int Addr, int Val) override;
	int	IntRead (int Bank, int Addr) override;
	void	PowerOn  (void) override;
	void	Reset  (void) override;
	int	Save (FILE *out) override;
	int	Load (FILE *in, int version_id) override;
	void	Run (void) override;
};
} // namespace APU

namespace PPU {
class	PPU_VT32: public PPU_OneBus {
public:
	PPU_VT32(): PPU_OneBus() { }
	int	GetPalIndex (int) override;

};
} // namespace PPU