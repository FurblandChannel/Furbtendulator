#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "Sound.h"
#include "APU.h"
#include "CPU.h"
#include "PPU.h"
#include "AVI.h"
#include "GFX.h"
#include "Controllers.h"
#include "Filter.h"
#include "Settings.h"
#include "Debugger.h"
#include "OneBus.h"
#include "OneBus_VT32.h"

namespace CPU {
CPU_VT32::CPU_VT32(uint8_t* _RAM):
	CPU_RP2A03(0, 4096, _RAM) {
	ResetEncryption();
}

void	CPU_VT32::ResetEncryption(void) {
	EncryptionStatus =NextEncryptionStatus =ChangeEncryptionStatus =false;
}

uint8_t	CPU_VT32::GetOpcode(void) {
	uint8_t result =CPU_RP2A03::GetOpcode();
	return result ^(EncryptionStatus? 0xA1: 0x00);
}

void	CPU_VT32::IN_JMP (void) {
	if (ChangeEncryptionStatus) EncryptionStatus =NextEncryptionStatus;
	CPU_RP2A03::IN_JMP();
}

void	CPU_VT32::IN_JMPI (void) {
	if (ChangeEncryptionStatus) EncryptionStatus =NextEncryptionStatus;
	CPU_RP2A03::IN_JMPI();
}

void	CPU_VT32::PowerOn (void) {
	CPU_RP2A03::PowerOn();
	ResetEncryption();
}

void	CPU_VT32::Reset (void) {
	CPU_RP2A03::Reset();
	ResetEncryption();
}

int	CPU_VT32::Save (FILE *out) {
	int clen =CPU_RP2A03::Save(out);
	writeBool(EncryptionStatus);
	writeBool(NextEncryptionStatus);
	writeBool(ChangeEncryptionStatus);
	return clen;
}

int	CPU_VT32::Load (FILE *in, int version_id) {
	int clen =CPU_RP2A03::Load(in, version_id);
	readBool(EncryptionStatus);
	readBool(NextEncryptionStatus);
	readBool(ChangeEncryptionStatus);
	return clen;
}

} // namespace CPU

namespace APU {
void	APU_VT32::Channel::reset (void) {
	count =address =0;
	period =0x6F;
	sample =0;
	volume =128;
	playing =false;
}

void	APU_VT32::Channel::start (int _mode, uint32_t _address) {
	adpcm =!!(_mode &0x40);
	if (adpcm) {
		predictor =0;
		stepIndex =0;
		stepSize =7;
		secondNibble =false;
	}
	sample =0;
	count =0;
	address =_address;
	playing =true;
	filterPCM.recalc(20, 1789773.0, 1789773.0 /111 *0.5);
}

void	APU_VT32::Channel::stop (void) {
	sample =0;
	count =0;
	playing =false;
}

static const int indexTable[16] ={
	-1, -1, -1, -1, 1, 2, 4, 6,
	-1, -1, -1, -1, 1, 2, 4, 6
};
static const int stepTable[89] = { 
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

void	APU_VT32::Channel::run (void) {
	while (playing && count <=0) {
		count += period;
		while (address >=RI.PRGROMSize) address -=RI.PRGROMSize;
		if (adpcm) {
			// The end of an ADPCM recording *seems* to be 00 followed by FF
			if (RI.PRGROMData[address] ==0x00 && RI.PRGROMData[address+1] ==0xFF) {
				stop();
				break;
			}
			// Basically standard IMA ADPCM. Modified to output 12-bit samples by reducing the values in stepTable,
			// and saturating the predictor at +/-2048
			int nibble =RI.PRGROMData[address];
			if (secondNibble) { // LSB second
				nibble &=0x0F;
				address++;
			} else // MSB first
				nibble >>=4;
			secondNibble =!secondNibble;
			
			stepSize =stepTable[stepIndex];
			int difference =0;
			if (nibble &4) difference +=stepSize;
			if (nibble &2) difference +=stepSize >>1;
			if (nibble &1) difference +=stepSize >>2;
			difference +=stepSize >>3;
			if (nibble &8) difference = -difference;
			
			predictor =predictor +difference;
			if (predictor > 2047) predictor = 2047;
			if (predictor <-2048) predictor =-2048;
			sample =predictor;
			
			stepIndex +=indexTable[nibble];
			if (stepIndex <0) stepIndex =0;
			if (stepIndex >88) stepIndex =88;
		} else {
			int value =RI.PRGROMData[address++];;
			if (value ==0xFF) { // End of a PCM recording seems to be just FF.
				stop();
				break;
			}
			sample =(value -0x80) <<4; // output 12 bits to be compatible with ADPCM output
		}		
	}
	count--;
}

int	APU_VT32::Channel::output (void) {
	return (sample *volume) >>4;
}

APU_VT32::APU_VT32(void):
	APU_RP2A03(),
	address(0) {
}

void	APU_VT32::PowerOn() {
	APU_RP2A03::PowerOn();
	address =0;
	aluOperand14 =0;
	aluOperand56 =0;
	aluOperand67 =0;
	for (auto& channel: channels) channel.reset();
}

void	APU_VT32::Reset() {
	APU_RP2A03::Reset();
	for (auto& channel: channels) channel.reset();
}

int	APU_VT32::IntRead (int bank, int addr) {
	switch(addr) {
	case 0x014:
		return (channels[0].playing? 1: 0) | (channels[1].playing? 2: 0);
	case 0x119:
		return 0x00; // Was TV system on VT03, must return 0 for VT03->VT32 ports to display the correct palette
	case 0x130:
	case 0x138:
		return aluOperand14 >> 0 &0xFF;
	case 0x131:
	case 0x139:
		return aluOperand14 >> 8 &0xFF;
	case 0x132:
	case 0x13A:
		return aluOperand14 >>16 &0xFF;
	case 0x133:
	case 0x13B:
		return aluOperand14 >>24 &0xFF;
	case 0x134:
	case 0x13C:
		return aluOperand56 >> 0 &0xFF;
	case 0x135:
	case 0x13D:
		return aluOperand56 >> 8 &0xFF;
	case 0x136:
		return aluBusy;
	default:
		return APU_RP2A03::IntRead(bank, addr);
	}
}

void	APU_VT32::IntWrite (int bank, int addr, int val) {
	if ((addr &0xF00) ==0x100) reg4100[addr &0xFF] =val;
	
	switch(addr) {
	case 0x010:
		if (channels[0].playing || channels[1].playing)
			;
		else
			APU_RP2A03::IntWrite(bank, addr, val);
		break;
	case 0x012: // Used both for NES APU DPCM as well as VT32 PCM address
		address =address &~(0xFF <<6) | ((val &0xFF) <<6);
		APU_RP2A03::IntWrite(bank, addr, val);
		break;
	case 0x031: // Value always 80 or 00, always for channel $10
		break;
	case 0x032: // Value always 6F or 00, always for channel $08
		if (val) {
			channels[0].period =val;
			channels[1].period =val;
			filterPCM.recalc(20, 1789773.0, 1789773.0 /val *0.5);
		}
		break;
	case 0x033:
		if ( val &0x10 && !channels[0].playing) channels[0].start(val &0xC0, address &~0x3F);
		if ( val &0x08 && !channels[1].playing) channels[1].start(val &0xC0, address &~0x3F);
		// Clearing a channel's bits only stops it in PCM mode, not in ADPCM mode.
		if (~val &0x10 && val &0x80 && channels[0].playing) channels[0].stop();
		if (~val &0x08 && val &0x80 && channels[1].playing) channels[1].stop();
		// Both ADPCM and PCM bits clear? Disable all channels.
		if (~val &0x80 && ~val &0x40) {
			channels[0].stop();
			channels[1].stop();
		}
		break;
	case 0x034: {
		int shift =val >>1 &7;
		if (shift ==0) shift =8;
		CPU::CPU[which]->DMAMiddleAddr =val &0xF0;
		CPU::CPU[which]->DMALength =1 <<shift;
		CPU::CPU[which]->DMATarget =(val &1)? 0x2007: 0x2004;
		break;
	}
	case 0x035:
		address =address &~(0x7F <<14) | ((val &0x7F) <<14);
		break;
	case 0x036:
		address =address &~(0xFF <<21) | ((val &0xFF) <<21);
		break;
	case 0x037: // Used at start of "Got On", value 00
		channels[0].stop();
		break;
	case 0x038: // Used before starting sample in channel $08. Stop?
		channels[1].stop();
		break;
	case 0x039: // Channel stop in ADPCM mode. May also work in PCM mode, but no way to check.
		if (val &0x10 && channels[0].playing) channels[0].stop();
		if (val &0x08 && channels[1].playing) channels[1].stop();
		break;
	case 0x11E:
		dynamic_cast<CPU::CPU_VT32*>(CPU::CPU[which])->ChangeEncryptionStatus =true;
		dynamic_cast<CPU::CPU_VT32*>(CPU::CPU[which])->NextEncryptionStatus =(val ==5)? true: false;
		break;
	case 0x130:
		aluOperand14 =aluOperand14 &0xFFFFFF00 | val << 0;
		break;
	case 0x131:
		aluOperand14 =aluOperand14 &0xFFFF00FF | val << 8;
		break;
	case 0x132:
		aluOperand14 =aluOperand14 &0xFF00FFFF | val <<16;
		break;
	case 0x133:
		aluOperand14 =aluOperand14 &0x00FFFFFF | val <<24;
		break;
	case 0x134:
		aluOperand56 =aluOperand56 &0xFF00 | val << 0;
		break;
	case 0x135:
		aluOperand56 =aluOperand56 &0x00FF | val << 8;
		aluOperand14 =(aluOperand14 &0xFFFF) *aluOperand56;
		aluBusy =16;
		break;
	case 0x136:
		aluOperand67 =aluOperand67 &0xFF00 | val << 0;
		break;
	case 0x137:
		aluOperand67 =aluOperand67 &0x00FF | val << 8;
		if (aluOperand67 !=0) {			
			aluOperand56 =aluOperand14 %aluOperand67;
			aluOperand14 =aluOperand14 /aluOperand67;
			aluBusy =32;
		}
		break;
	default:
		APU_RP2A03::IntWrite(bank, addr, val);
		break;
	}
}

void	APU_VT32::Run (void) {
	APU_RP2A03::Run();
	
	if (Sound::isEnabled && !Controllers::capsLock && (channels[0].playing || channels[1].playing)) {
		int result =0;
		for (auto& channel: channels) {
			channel.run();
			result +=channel.output();
		}
		if (Sound::isEnabled && !Controllers::capsLock) {
		#if DISABLE_ALL_FILTERS
		#else
			if (Settings::LowPassFilterOneBus)
				Output +=(float) filterPCM.output(result) /32767.0;
			else
		#endif
				Output +=(float) result /32767.0;
		}
	}
	
	if (aluBusy) aluBusy--;
}

int	APU_VT32::Load (FILE *in, int version_id) {
	int clen =APU_RP2A03::Load(in, version_id);
	readLong(address);
	for (auto& channel: channels) {
		readLong(channel.count);
		readLong(channel.period);
		readLong(channel.address);
		readWord(channel.sample);
		readByte(channel.volume);
		readBool(channel.playing);
		readBool(channel.adpcm);
		readLong(channel.predictor);
		readLong(channel.stepIndex);
		readLong(channel.stepSize);
		readBool(channel.secondNibble);
	}
	readLong(aluOperand14);
	readWord(aluOperand56);
	readWord(aluOperand67);
	readByte(aluBusy);
	return clen;
}

int	APU_VT32::Save (FILE *out) {
	int clen =APU_RP2A03::Save(out);
	writeLong(address);
	for (auto& channel: channels) {
		writeLong(channel.count);
		writeLong(channel.period);
		writeLong(channel.address);
		writeWord(channel.sample);
		writeByte(channel.volume);
		writeBool(channel.playing);
		writeBool(channel.adpcm);
		writeLong(channel.predictor);
		writeLong(channel.stepIndex);
		writeLong(channel.stepSize);
		writeBool(channel.secondNibble);
	}
	writeLong(aluOperand14);
	writeWord(aluOperand56);
	writeWord(aluOperand67);
	writeByte(aluBusy);
	return clen;
}
} // namespace APU

namespace PPU {
int	PPU_VT32::GetPalIndex(int TC) {
	if (!(TC &0x63)) TC =0;
	if (reg2000[0x10] &COLCOMP)
		return (Palette[TC |0x80] <<6) +Palette[TC |0x00] +PALETTE_VT32;
	else
		return Palette[TC];
}
} // namespace PPU