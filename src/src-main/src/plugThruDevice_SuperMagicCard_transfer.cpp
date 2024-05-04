#include "stdafx.h"
#include <vector>
#include <queue>
#include <mutex>

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "plugThruDevice.hpp"

#define FASTLOAD 1

namespace PlugThruDevice {
namespace SuperMagicCard {
extern	std::queue<uint8_t> receiveQueue;
extern	std::queue<uint8_t> sendQueue;
extern	std::queue<uint8_t> externalQueue;
extern	std::mutex receiveQueueBusy;
extern	std::mutex sendQueueBusy;
extern	std::mutex externalQueueBusy;
extern union {
	uint8_t k8 [64][2][4096];
	uint8_t k16[32][4][4096];
	uint8_t	b  [512*1024];
} prgram;
extern union {
	uint8_t k1 [256][1024];
	uint8_t k8 [32][8][1024];
	uint8_t b  [256*1024];
} chrram;

std::vector<uint8_t> fileData;
bool	hardReset;
bool	haveFileToSend;

void	sendCommand (uint8_t Code, uint16_t Addr, size_t Len) {
	std::lock_guard<std::mutex> lock(receiveQueueBusy);
	receiveQueue.push(0xD5);
	receiveQueue.push(0xAA);
	receiveQueue.push(0x96);
	receiveQueue.push(Code);
	receiveQueue.push(Addr &0xFF);
	receiveQueue.push(Addr >>8);
	receiveQueue.push(Len &0xFF);
	receiveQueue.push(Len >>8);
	receiveQueue.push(0x81 ^Code ^(Addr &0xFF) ^(Addr >>8) ^(Len &0xFF) ^(Len >>8));
}

void	sendCPUDataBlock (uint16_t Addr, uint8_t *Data, size_t Len) {
	sendCommand(0x00, Addr, Len);

	uint8_t checksum =0x81;
	std::lock_guard<std::mutex> lock(receiveQueueBusy);
	for (int i =0; i <Len; i++) {
		receiveQueue.push(Data[i]);
		checksum ^=Data[i];
	}
	receiveQueue.push(checksum);
}

uint8_t	receiveNibble() {
	while (1) {
		sendQueueBusy.lock();
		if (sendQueue.empty()) {
			sendQueueBusy.unlock();
			Sleep(1);
			continue;
		}
		break;
	}
	uint8_t result =sendQueue.front();
	sendQueue.pop();
	sendQueueBusy.unlock();
	return result;
}

uint8_t	receiveByte() {
	uint8_t low =receiveNibble() &0x0F;
	uint8_t high=receiveNibble() &0x0F;
	return (high <<4) | low;
}

void	clearsendQueue() {
	std::lock_guard<std::mutex> lock(sendQueueBusy);
	sendQueue =std::queue<uint8_t>();
}

void	receiveCPUDataBlock (uint16_t Addr, uint8_t *Data, size_t Len) {
	clearsendQueue();
	sendCommand(0x01, Addr, Len);

	uint8_t checksum =0x81;
	for (int i =0; i <Len; i++) {
		Data[i] =receiveByte(); 
		checksum ^=Data[i];
	}	
	uint8_t got =receiveByte();
	if (checksum !=got)
		EI.DbgOut(L"CPU %04X %04X Checksum mismatch, want %02X, got %02X", Addr, Len, checksum, got);
	else
		EI.DbgOut(L"CPU %04X %04X Checksum match, want %02X, got %02X", Addr, Len, checksum, got);
}

void	sendPPUDataBlock (uint16_t Addr, uint8_t *Data, size_t Len) {
	sendCommand(0x02, Addr, Len);

	uint8_t checksum =0x81;
	std::lock_guard<std::mutex> lock(receiveQueueBusy);
	for (int i =0; i <Len; i++) {
		receiveQueue.push(Data[i]);
		checksum ^=Data[i];
	}
	receiveQueue.push(checksum);
}

void	receivePPUDataBlock (uint16_t Addr, uint8_t *Data, size_t Len) {
	clearsendQueue();
	sendCommand(0x03, Addr, Len);

	uint8_t checksum =0x81;
	for (int i =0; i <Len; i++) {
		Data[i] =receiveByte();
		checksum ^=Data[i];
	}	
	uint8_t got =receiveByte();
	if (checksum !=got) 
		EI.DbgOut(L"PPU %04X %04X Checksum mismatch, want %02X, got %02X", Addr, Len, checksum, got);
	else
		EI.DbgOut(L"PPU %04X %04X Checksum match, want %02X, got %02X", Addr, Len, checksum, got);
}

void	sendCPUWrite (uint16_t Addr, uint8_t Val) {
	sendCommand(0x00, Addr, 1);

	std::lock_guard<std::mutex> lock(receiveQueueBusy);
	receiveQueue.push(Val);
	receiveQueue.push(0x81 ^Val);
}

void	sendFile (void) {
	uint8_t* data =&fileData[0];
	
	bool detected =true;
	if (data[8] !=0xAA || data[9] !=0xBB || data[10] !=0x00) {	// SMC ROM image signature not detected?
		// Some SMC ROM images have no valid signature.
		// They will however have a file size that is a multiple of 512,
		// and 512 bytes above an 8192 byte boundary with no trainer,
		// and 1024 bytes above an 8192 byte boundary with trainer.
		if (fileData.size() %512 && fileData.size() !=0x20208) detected =false;
		if ( data[0] &0x40 && (fileData.size() %8192) !=1024 && fileData.size() !=0x20208) detected =false;
		if (~data[0] &0x40 && (fileData.size() %8192) != 512 && fileData.size() !=0x20208) detected =false;
	}
	if (detected) {
		sendCPUWrite(0x2001, 0x00);	// Disable rendering
		size_t PRGBytes =0;
		size_t CHRBytes =0;
		
		if (data[7] ==0xAA) {
			PRGBytes =data[3] *8192;
			CHRBytes =data[4] *8192;
		} else switch (data[1] >>5) {
			case 0:	PRGBytes =128 *1024; CHRBytes = 0*1024; break;	// UNROM
			case 1:	PRGBytes =256 *1024; CHRBytes = 0*1024; break;	// Custom
			case 2:	PRGBytes =256 *1024; CHRBytes = 0*1024; break;	// UOROM
			case 3:	PRGBytes =256 *1024; CHRBytes = 0*1024; break;	// Reverse UNROM
			case 4:	PRGBytes =128 *1024; CHRBytes =32*1024; break;	// GNROM
			case 5:	PRGBytes = 32 *1024; CHRBytes =32*1024; break;	// CNROM-256
			case 6:	PRGBytes = 32 *1024; CHRBytes =16*1024; break;	// CNROM-128
			case 7:	PRGBytes = 32 *1024; CHRBytes = 8*1024; break;	// NROM-256
		}
		size_t headerSize =fileData.size() ==0x20208? 8: 512;
		uint8_t *s =data +headerSize;		// Current position in file: after header
		if ((headerSize +(data[0] &0x40? 512: 0) +PRGBytes +CHRBytes) >fileData.size()) {
			EI.DbgOut(L"File is shorter than indicated by header.");
			return;
		}

		sendCPUWrite(0x4500, 0x02);	// GUI Mode: 8 KiB memory window at $6000
		sendCPUWrite(0x42FD, 0x20);	// Write-enable PRG-ROM
		sendCPUWrite(0x43FC, 0x00);	// Set 4M PRG mode, so that the memory window at $6000 can be selected via $4507

		// Transfer Copier Info Block
		sendCPUDataBlock(0x5020, data, 8);

		// Transfer Trainer, if present, to $0600
		if (data[0] &0x40) {
			sendCPUDataBlock(0x0600, s, 512);
			s +=512;
		}

		// Transfer PRG data
		if (data[7] !=0xAA && data[0] &0x30) {	// Special PRG-ROM 256 KiB split
			// PRG half
			PRGBytes =(data[0] &0x20)? 262144: 131072;
			for (uint8_t Bank =0; Bank <PRGBytes/8192; Bank++) {
				sendCPUWrite(0x4507, Bank);
				#if FASTLOAD
				memcpy(&prgram.k8[Bank][0][0], s, 8192);
				sendCPUDataBlock(0x6000, s, 1); // Required for BIOS internal state consistency
				#else
				sendCPUDataBlock(0x6000, s, 8192);
				#endif
				s +=8192;
			}
			// CHR half
			PRGBytes =(data[0] &0x10)? 262144: 131072;
			for (uint8_t Bank =0; Bank <PRGBytes/8192; Bank++) {
				sendCPUWrite(0x4507, Bank +256/8);
				#if FASTLOAD
				memcpy(&prgram.k8[Bank +256/8][0][0], s, 8192);
				sendCPUDataBlock(0x6000, s, 1); // Required for BIOS internal state consistency
				#else
				sendCPUDataBlock(0x6000, s, 8192);
				#endif
				s +=8192;
			}
			CHRBytes =0;
		} else if ((data[1] >>5) >=5) {		// CNROM and NROM games are loaded into the last 32 KiB of the first 128 KiB bank
			for (uint8_t Bank =0; Bank <32768/8192; Bank++) {
				sendCPUWrite(0x4507, Bank +96/8);
				#if FASTLOAD
				memcpy(&prgram.k8[Bank +96/8][0][0], s, 8192);
				sendCPUDataBlock(0x6000, s, 1); // Required for BIOS internal state consistency
				#else
				sendCPUDataBlock(0x6000, s, 8192);
				#endif
				s +=8192;
			}
		} else {
			for (uint8_t Bank =0; Bank <PRGBytes/8192; Bank++) {
				sendCPUWrite(0x4507, Bank);
				#if FASTLOAD
				memcpy(&prgram.k8[Bank][0][0], s, 8192);
				sendCPUDataBlock(0x6000, s, 1); // Required for BIOS internal state consistency
				#else
				sendCPUDataBlock(0x6000, s, 8192);
				#endif
				s +=8192;
			}
		}
		
		// Transfer CHR data
		for (uint8_t Bank =0; Bank <CHRBytes/8192; Bank++) {
			if (Bank ==0) { // First block of CHR is copied from WRAM into CHR-RAM by BIOS
				sendCPUWrite(0x4500, 0x22);
				sendCPUWrite(0x42FF, 0x30);
				sendCPUDataBlock(0x6000, s, 8192);
				sendCPUWrite(0x4500, 0x07);
			} else {	// Other blocks must be poked into CHR-RAM via special command
				for (int i =0; i <8; i++) sendCPUWrite(0x4510 +i, Bank*8 +i);
				#if FASTLOAD
				memcpy(&chrram.k8[Bank][0][0], s, 8192);
				sendPPUDataBlock(0x0000, s, 1); // Required for BIOS internal state consistency
				#else
				sendPPUDataBlock(0x0000, s, 8192);
				#endif
			}
			s +=8192;
		}				
		// Final initialization and command to run game
		for (int i =0; i < 4; i++) sendCPUWrite(0x4504 +i, 0x00);
		for (int i =0; i <12; i++) sendCPUWrite(0x4510 +i, 0x00);
		sendCommand(0x05, 1, 0);
	} else	// not an SMC ROM image, check SMC RTS image
	if (data[8] ==0xAA && data[9] ==0xBB && data[10] ==0x01) {	// SMC RTS image
		EI.DbgOut(L"sending RTS");
		uint8_t *s =data +512;		// Current position in file: after header
		sendCommand(0x05, 3, 0);
		sendCPUDataBlock(0x5840, s, 0x68);
		s +=0x68;
		
		sendCPUWrite(0x4500, 0x32);
		sendCPUWrite(0x42FF, 0x30);
		
		sendCPUDataBlock(0x6000, s, 4096);
		s +=4096;
		
		for (int n =0x02; n <=0x22; n +=0x20) {
			sendCPUWrite(0x4500, n);
			sendCPUDataBlock(0x6000, s, 8192);
			s +=8192;
		}
		
		sendCPUWrite(0x2001, 0);
		for (int n =1; n <=3; n++) {
			sendCPUWrite(0x43FC, n);
			sendPPUDataBlock(0x0000, s, 8192);
			s +=8192;
		}
		sendCPUWrite(0x43FC, 0);
		sendCPUWrite(0x2001, 0x6B);
		sendCommand(0x05, 4, 0);
	} else	// neither SMC ROM nor SMC RTS image.
		EI.DbgOut(L"File is not a Super Magic Card ROM/RTS image.");
}

void	doAnyTransfer(void) {	
	if (hardReset && haveFileToSend && !sendQueue.empty()) {
		std::lock_guard<std::mutex> lock(sendQueueBusy);
		if (sendQueue.front() ==0x04) {
			sendQueue.pop();
			hardReset =false;
			sendFile();
		}
	}
	externalQueueBusy.lock();
	if (externalQueue.size()) {
		std::lock_guard<std::mutex> lock(receiveQueueBusy);
		receiveQueue.push(externalQueue.front());
		externalQueue.pop();
	}
	externalQueueBusy.unlock();
}

void	addToReceiveQueue(uint8_t val) {
	std::lock_guard<std::mutex> lock(externalQueueBusy);
	externalQueue.push(val);
}

} // namespace SuperMagicCard
} // namespace PlugThruDevice

/*
	SMC Command
	D5 AA 96 nn aa aa ll ll cc ... cc
	         || || || || || ||     ++- Checksum over data
		 || || || || || ++-------- Checksum over nn aa aa ll ll
		 || || || ++-++----------- Data length LSB MSB
		 || ++-++----------------- Target address LSB MSB
		 ++----------------------- Command#

	Commands:
	00	Upload data into CPU address space
	01	Download data from CPU address space
	02	Upload data into PPU address space
	03	Download data from PPU address space
	04	JMP to specified address
	05	Special function
		0	Reset copier to boot menu ("Backup Cassette"?!)
		1	Run game in DRAM
		2	Run game in cartridge slot ("Play Cassette")
		3	Pause
		4	Continue
		5	Abort current operation
	06	???
	Receive bytes:
	04	ACK
*/
