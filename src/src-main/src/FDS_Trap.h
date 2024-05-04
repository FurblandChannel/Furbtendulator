void		crcAddByte (uint8_t data);
FDSPatch::Game	patchDefinition;
CPU::CPU_RP2A03* theCPU;
uint8_t		lastErrorCode;

// Helper functions for accessing RAM
uint8_t getCPUByte (uint16_t addr) {
	int bank =addr >>12;
	addr &=0xFFF;
	return (theCPU->ReadHandler[bank])(bank, addr +0);
}
void	putCPUByte (uint16_t addr, uint8_t val) {
	int bank =addr >>12;
	addr &=0xFFF;
	(theCPU->WriteHandler[bank])(bank, addr, val);
}
uint8_t getPPUByte (uint16_t addr) {
	int bank =(addr >>10) &0xF;
	addr &=0x3FF;
	return (EI.GetPPUReadHandler(bank))(bank, addr);
}
void	putPPUByte (uint16_t addr, uint8_t val) {
	int bank =(addr >>10) &0xF;
	addr &=0x3FF;
	return (EI.GetPPUWriteHandler(bank))(bank, addr, val);
}

uint16_t getCPUWord (uint16_t addr) {
	return getCPUByte(addr +0) | (getCPUByte(addr +1) <<8);
}

void	putCPUWord (uint16_t addr, uint16_t val) {
	putCPUByte(addr +0, val &0xFF);
	putCPUByte(addr +1, val >>8);
}

// Disk decoding functions
bool	diskCheckBlockType (uint8_t type) {
	if (!RI.diskData28) NES::insert28();
	lastErrorCode =0x01;
	if (RI.diskData28) {
		lastErrorCode =0x22 +type;
		putCPUByte(0x0007, type);	
		try {
			crc =0;
			while (RI.diskData28->at(diskPosition++) !=0x80);
			crcAddByte(0x80);
			
			uint8_t c =RI.diskData28->at(diskPosition++);
			crcAddByte(c);
			if (c ==type) lastErrorCode =0;
		} catch (...) {}
	}
	return lastErrorCode ==0? true: false;
}

std::vector<uint8_t> diskReadBlock (uint8_t type, size_t size) {
	std::vector<uint8_t> result;
	try {
		if (diskCheckBlockType(type)) {
			lastErrorCode =0x28;
			while (size--) {
				uint8_t c =RI.diskData28->at(diskPosition++);
				crcAddByte(c);
				result.push_back(c);
			}
			crcAddByte(RI.diskData28->at(diskPosition++));
			crcAddByte(RI.diskData28->at(diskPosition++));
			lastErrorCode =0x00;
			if (crc !=0x00) {
				EI.DbgOut(L"CRC error, pos %X, have: %04X. Type was %d", diskPosition, crc, type);
				lastErrorCode =0x27;
			}
			diskPosition +=488/8;
		}
	}
	catch(...) { }
	if (lastErrorCode !=0) EI.DbgOut(L"Error %02X", lastErrorCode);
	return result;
}

void	diskWriteByte (uint8_t data) {
	if (diskPosition >= RI.diskData28->size())
		RI.diskData28->push_back(data);
	else {
		(*RI.diskData28)[diskPosition] =data;
	}
	RI.modifiedSide28 =true;
	crcAddByte(data);
	diskPosition++;
}

void	diskWriteBlock (uint8_t type, std::vector<uint8_t>& data) {
	if (NES::writeProtected28) {
		lastErrorCode =0x03;
		return;
	}
	putCPUByte(0x0007, type);	

	// write second half of 976-bit gap (first half was skipped by diskReadBlock)
	for (int i =0; i <488/8; i++) diskWriteByte(0x00);

	crc =0;
	diskWriteByte(0x80);
	diskWriteByte(type);
	for (auto& c: data) diskWriteByte(c);
	crcAddByte(0x00);
	crcAddByte(0x00);

	int theCRC =crc; // save CRC, because the next diskWriteByte will already modify it
	diskWriteByte(theCRC &0xFF);
	diskWriteByte(theCRC >>8);

	// write first half of 976-bit gap
	for (int i =0; i <488/8; i++) diskWriteByte(0x00);
}

void	applyPatches (void) {
	for (auto& item: patchDefinition.items) {
		bool match =true;
		if (item.address < 0x10000) { // CPU address space
			for (unsigned int i =0; i <item.compare.size(); i++) {
				if (getCPUByte(item.address +i) != item.compare[i]) {
					match =false;
					break;
				}
			}
			if (match) for (unsigned int i =0; i <item.replace.size(); i++) putCPUByte(item.address+i, item.replace[i]);
		} else { // PPU address space
			for (unsigned int i =0; i <item.compare.size(); i++) {
				if (getPPUByte(item.address +i) != item.compare[i]) {
					match =false;
					break;
				}
			}
			if (match) for (unsigned int i =0; i <item.replace.size(); i++) putPPUByte(item.address+i, item.replace[i]);
		}
	}
}

void	cbGetDiskInfo (void) {
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskInfo =getCPUWord(pParam +0);

	diskPosition =0;
	std::vector<uint8_t> diskHeader =diskReadBlock(1, 55);
	if (lastErrorCode ==0) {
		for (int i =0; i <10; i++) putCPUByte(pDiskInfo++, diskHeader[14 +i]);

		std::vector<uint8_t> temp =diskReadBlock(2, 1);
		if (lastErrorCode ==0) {
			putCPUByte(0x0006, temp[0]);
			putCPUByte(pDiskInfo++, temp[0]);
			
			uint16_t totalSize =0;
	
			while (lastErrorCode ==0 && getCPUByte(0x0006) >0) {
				std::vector<uint8_t> header =diskReadBlock(3, 15);
				if (lastErrorCode !=0) break;
	
				uint8_t id =header[1];
				uint16_t size =header[12] |(header[13] <<8);
				totalSize +=size +261;
				
				putCPUByte(pDiskInfo++, id);
				for (int i =0; i <8; i++) putCPUByte(pDiskInfo++, header[2 +i]);
	
				std::vector<uint8_t> data =diskReadBlock(4, size);
				if (lastErrorCode !=0) break;
				
				putCPUByte(0x0006, getCPUByte(0x0006) -1);
			}
			putCPUByte(pDiskInfo++, totalSize &0xFF);
			putCPUByte(pDiskInfo++, totalSize >>8);
		}
	}
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;
	
	pParam +=1;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
}

bool	diskCheckSide (uint16_t ofs, unsigned int which) {
	bool result =false;

	static const uint8_t wantedString[] ="*NINTENDO-HVC*";
	unsigned int prevSide =NES::side28;
	if (which !=prevSide) {
		NES::eject28();
		NES::side28 =which;
		NES::insert28();
	}
	diskPosition =0;

	std::vector<uint8_t> header =diskReadBlock(1, 55);
	if (lastErrorCode ==0 && memcmp(&header[0], wantedString, 14) ==0) {
		result =true;
		
		putCPUByte(0x0008, header[24]); // Max boot file ID
		for (int i =0; i <10; i++) {
			uint8_t wantedByte =getCPUByte(ofs +i);
			if (header[14 +i] !=wantedByte && wantedByte !=0xFF) {
				result =false;
				break;
			}
		}
	}
	if (result !=true) {
		NES::eject28();
		NES::side28 =prevSide;
		NES::insert28();
	} else
		if (NES::side28 !=prevSide) EI.DbgOut(L"Automatically switched to side %d", NES::side28);
	
	RI.changedDisk28 =false; // prevent FDS::cpuCycle() from resetting the disk position
	return result;
}

void	switchToSide (unsigned int which) {
	if (which != NES::side28 && which < NES::disk28.size()) {
		EI.DbgOut(L"Inserting %d via $401F", which);
		NES::eject28();
		NES::side28 =which;
		NES::insert28();
	}
}

bool	diskFindCorrectSide (uint16_t ofs) {
	bool found =true;
	if (!diskCheckSide(ofs, NES::side28)) {	// if current side is not already the correct side
		found =false;
		for (unsigned int i =0; i <NES::disk28.size(); i++) {
			if (i ==NES::side28) continue;	// already checked the current side
			if (diskCheckSide(ofs, i)) {
				found =true;
				break;
			}
		}
	}
	return found;
}

void	diskRewriteFileCount (void) {
	std::vector <uint8_t> temp;
	temp.push_back(getCPUByte(0x0006));

	diskPosition =0;
	diskReadBlock(1, 55); // skip header
	if (lastErrorCode ==0) diskWriteBlock(2, temp); // write file count block
}

// Callback handlers
void	cbLoadFiles (void) {
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskID =getCPUWord(pParam +0);
	uint16_t pLoadList =getCPUWord(pParam +2);
	putCPUWord(0x0000, pDiskID);
	putCPUWord(0x0002, pLoadList);
	putCPUByte(0x0004, 0xF8); // ???
	putCPUByte(0x0005, 0x02); // ???
	putCPUByte(0x000E, 0x00); // # of files already loaded

	bool nmi =false;
	bool gameDoctor =false;

	if (diskFindCorrectSide(pDiskID)) {
		std::vector<uint8_t> temp =diskReadBlock(2, 1);
		if (lastErrorCode ==0) putCPUByte(0x0006, temp[0]);

		while (lastErrorCode ==0 && getCPUByte(0x0006) >0 && !nmi) {
			std::vector<uint8_t> header =diskReadBlock(3, 15);
			if (lastErrorCode !=0) break;

			uint8_t id =header[1];
			uint16_t addr =header[10] |(header[11] <<8);
			uint16_t size =header[12] |(header[13] <<8);
			uint16_t skip =0;
			uint8_t type =header[14]; // Anything other than #$00 must be PPU; necessary for 探偵神宮寺三郎: 危険な二人

			std::vector<uint8_t> data =diskReadBlock(4, size);
			if (lastErrorCode !=0) break;

			bool loadThisFile =false;
			if (getCPUByte(pLoadList) ==0xFF) // Booting
				loadThisFile =id <=getCPUByte(0x0008); // max boot file id
			else {
				for (int i =0; i <20; i++) {
					uint8_t c =getCPUByte(pLoadList +i);
					if (c ==id) loadThisFile =true;
					if (c ==id || c ==0xFF) break;
				}
			}
			if (getCPUByte(pLoadList) ==0xFF && id ==0x10 && addr ==0x6000 && size +addr >0xE000) { // IREM protection
				loadThisFile =true;
				skip =size -0x8000;
				addr -=skip;
				EI.DbgOut(L"Skipping first $%X bytes", skip);
			}
			putCPUByte(0x0009, loadThisFile? 0x00: 0xFF);
			
			if (loadThisFile) {
				putCPUByte(0x000E, getCPUByte(0x000E) +1); // Increase # of loaded files
				//EI.DbgOut(L"Loading file %02X (type %d) to addr %04X, size %04X", id, type, addr, size);
				switch (type) {
					case 0:	for (int i =skip; i <size; i++) {
							// If address $2000 is written to with bit 7, it is an unlicensed disk and wants an NMI as quickly as possible.
							if (((addr +i) &0xE007) ==0x2000) nmi =!!(data[i] &0x80) && !patchDefinition.ignoreNMI;
							if (addr ==0x43FF || addr ==0x4FFF) { // Must be first so that the device's handler can access the byte at 0x0080 if necessary
								putCPUByte(0x80 +i, data[i]);
								gameDoctor =true;								
							}
							// Do not write to Game Doctor 5000-5007, because that will corrupt FDS RAM on SGD2M
							if (!gameDoctor || (addr +i) <0x5000 || (addr +i) >=0x6000) putCPUByte(addr +i, data[i]);
						}
						if (gameDoctor) return;
						break;
					default:addr &=0x3FFF;
						putCPUByte(0x2000, getCPUByte(0x00FF) &0xFB);
						putCPUByte(0x2001, getCPUByte(0x00FE) &0xE7);
						putCPUByte(0x00FE, getCPUByte(0x00FE) &0xE7);
						putCPUByte(0x00FF, getCPUByte(0x00FF) &0xFB);
						for (int i =0; i <size; i++) putPPUByte(addr++, data[i]);
						getCPUByte(0x2002);
						putCPUByte(0x2006, addr >>8);
						putCPUByte(0x2006, addr &0xFF);
						break;
				}
			}
			putCPUByte(0x0006, getCPUByte(0x0006) -1);
		}
		if (getCPUByte(pLoadList) ==0xFF) {	// Booting: Set up the APU to the state it would have if we had not skipped the "insert disk" sound
			putCPUByte(0x4000, 0x10);
			putCPUByte(0x4001, 0x7F);
			putCPUByte(0x4002, 0x47);
			putCPUByte(0x4003, 0x58);
			putCPUByte(0x4004, 0x10);
			putCPUByte(0x4005, 0x7F);
			putCPUByte(0x4006, 0x5F);
			putCPUByte(0x4007, 0x58);
			putCPUByte(0x4008, 0x20);
			putCPUByte(0x400A, 0x00);
			putCPUByte(0x400B, 0x58);
			putCPUByte(0x4010, 0x00);
			putCPUByte(0x4015, 0x0F);
			putCPUByte(0x4017, 0xC0);
			putCPUByte(0x4080, 0x80);
			putCPUByte(0x408A, 0xE8);
		}
	}
	
	theCPU->Y =getCPUByte(0x000E);
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;

	pParam +=3;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
	if (nmi) theCPU->WantNMI =TRUE;
	applyPatches();
}

void	cbFileMatchTest (void) {
	uint16_t pLoadList =getCPUWord(0x0002);
	
	try {
		diskPosition++; // past file #
		uint8_t id =RI.diskData28->at(diskPosition++);
		diskPosition +=8; // past file name

		bool loadThisFile =false;
		if (getCPUByte(pLoadList) ==0xFF) // Booting
			loadThisFile =id <=getCPUByte(0x0008); // max boot file id
		else {
			for (int i =0; i <20; i++) {
				uint8_t c =getCPUByte(pLoadList +i);
				if (c ==id) loadThisFile =true;
				if (c ==id || c ==0xFF) break;
			}
		}
		
		putCPUByte(0x0009, loadThisFile? 0x00: 0xFF);
		putCPUByte(0x000E, getCPUByte(0x000E) +loadThisFile);		
	} catch(...) {}
	
	theCPU->IN_RTS();
}

void	cbLoadOrSkipFile (void) {
	try {
		uint16_t addr =RI.diskData28->at(diskPosition++) +(RI.diskData28->at(diskPosition++) <<8);
		uint16_t size =RI.diskData28->at(diskPosition++) +(RI.diskData28->at(diskPosition++) <<8);
		uint8_t  type =RI.diskData28->at(diskPosition++);
		diskPosition +=2 +488/8; // skip CRC and gap
		
		std::vector<uint8_t> data =diskReadBlock(4, size);
		
		if (lastErrorCode ==0 && getCPUByte(0x0009) ==0) switch (type) {
			case 0:	for (int i =0; i <size; i++) putCPUByte(addr +i, data[i]);
				break;
			default:addr &=0x3FFF;
				putCPUByte(0x2000, getCPUByte(0x00FF) &0xFB);
				putCPUByte(0x2001, getCPUByte(0x00FE) &0xE7);
				putCPUByte(0x00FE, getCPUByte(0x00FE) &0xE7);
				putCPUByte(0x00FF, getCPUByte(0x00FF) &0xFB);
				for (int i =0; i <size; i++) putPPUByte(addr++, data[i]);
				getCPUByte(0x2002);
				putCPUByte(0x2006, addr >>8);
				putCPUByte(0x2006, addr &0xFF);
				break;
		}
	} catch(...) {}
	applyPatches();
	
	theCPU->IN_RTS();
}

void	cbWriteFile (void) {
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskID =getCPUWord(pParam +0);
	uint16_t pFileHeader =getCPUWord(pParam +2);
	//EI.DbgOut(L"Writing file #%02X: ", theCPU->A);
	
	if (diskFindCorrectSide(pDiskID)) {
		std::vector<uint8_t> temp =diskReadBlock(2, 1);
		if (lastErrorCode ==0) {
			putCPUByte(0x0006, temp[0]);

			uint8_t fileNumber;
			for (fileNumber =0; fileNumber <getCPUByte(0x0006); fileNumber++) {
				if (lastErrorCode !=0) break;
				if (fileNumber ==theCPU->A) break;
	
				// Skip File
				std::vector<uint8_t> header =diskReadBlock(3, 15);
				if (lastErrorCode !=0) break;
	
				uint16_t size =header[12] |(header[13] <<8);
				std::vector<uint8_t> data =diskReadBlock(4, size);
				if (lastErrorCode !=0) break;
			}
	
			// Build header
			std::vector<uint8_t> header;
			header.push_back(theCPU->A);
			for (int i =0; i <14; i++) header.push_back(getCPUByte(pFileHeader +i));
			uint16_t size =getCPUWord(pFileHeader +11);
			uint16_t addr =getCPUWord(pFileHeader +14);
			uint8_t type =getCPUByte(pFileHeader +16);
	
			// Build data
			std::vector<uint8_t> data;
			switch(type) {
				case 0:	while (size--) data.push_back(getCPUByte(addr++));
					break;
				case 1:
				case 2: while (size--) data.push_back(getPPUByte(addr++));
					break;
			}

			// Write header and data
			diskWriteBlock(0x03, header);
			diskWriteBlock(0x04, data);
	
			// Increase file count, if necessary
			if (fileNumber >=getCPUByte(0x0006)) {
				putCPUByte(0x0006, getCPUByte(0x0006) +1);
				diskRewriteFileCount();
			}
		}
	}
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;

	pParam +=3;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
}

void	cbCheckDiskHeader (void) {
	diskFindCorrectSide(getCPUWord(0x0000));
	
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;
	theCPU->IN_RTS();
}

void	cbReadFileCount (void) {
	std::vector<uint8_t> temp =diskReadBlock(2, 1);
	if (lastErrorCode ==0) putCPUByte(0x0006, temp[0]);
	
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;
	theCPU->IN_RTS();	
}

void	cbCheckFileCount (void) {
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskID =getCPUWord(pParam +0);

	if (diskFindCorrectSide(pDiskID)) {
		std::vector<uint8_t> temp =diskReadBlock(2, 1);
		if (lastErrorCode ==0)  {
			temp[0] = theCPU->A;
			if (temp[0] &0x80) temp[0] =0;	
			putCPUByte(0x0006, temp[0]);
			diskRewriteFileCount();
		}
	}
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;

	pParam +=1;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
}

void	cbReduceFileCount (void) {
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskID =getCPUWord(pParam +0);

	if (diskFindCorrectSide(pDiskID)) {
		std::vector<uint8_t> temp =diskReadBlock(2, 1);
		if (lastErrorCode ==0)  {
			temp[0] -= theCPU->A;
			if (temp[0] &0x80) temp[0] =0;	
			putCPUByte(0x0006, temp[0]);
			diskRewriteFileCount();
		}
	}
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;

	pParam +=1;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
}

void	cbSetFileCount (void) {
	uint8_t newFileCount =theCPU->A +getCPUByte(0x0007); // Must be done here, since diskFindCorrectSide modifies the byte at 0x0007
	
	uint16_t pParam =1 +(theCPU->Pull() | (theCPU->Pull() <<8));
	uint16_t pDiskID =getCPUWord(pParam +0);

	if (diskFindCorrectSide(pDiskID)) {
		putCPUByte(0x0006, newFileCount);
		diskRewriteFileCount();
	}
	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;

	pParam +=1;
	theCPU->Push(pParam >>8);
	theCPU->Push(pParam &0xFF);
	theCPU->IN_RTS();
}

void	cbCheckBlockType (void) {
	diskCheckBlockType(theCPU->A);

	theCPU->A =theCPU->X =lastErrorCode;
	theCPU->FZ =lastErrorCode ==0;
	theCPU->IN_RTS();
}

void	cbSkip2 (void) {
	theCPU->PC +=2;
}

void	cbRTS (void) {
	theCPU->IN_RTS();
}

void	cbWaitLicense (void) {
	theCPU->PC +=2;
	theCPU->A =0;
}

void	initTrap (void) {
	NES::eject28();
	NES::side28 =0;
	NES::insert28();
	theCPU =CPU::CPU[0];
	CPU::callbacks.clear();
	CPU::callbacks.push_back({0, 0xE1F8, cbLoadFiles});
	CPU::callbacks.push_back({0, 0xE239, cbWriteFile});
	CPU::callbacks.push_back({0, 0xE2B7, cbCheckFileCount});
	CPU::callbacks.push_back({0, 0xE2BB, cbReduceFileCount});
	CPU::callbacks.push_back({0, 0xE309, cbSetFileCount});
	CPU::callbacks.push_back({0, 0xE32A, cbGetDiskInfo});	
	CPU::callbacks.push_back({0, 0xE445, cbCheckDiskHeader});
	CPU::callbacks.push_back({0, 0xE484, cbReadFileCount});
	CPU::callbacks.push_back({0, 0xE4A0, cbFileMatchTest});
	CPU::callbacks.push_back({0, 0xE4F9, cbLoadOrSkipFile});
	CPU::callbacks.push_back({0, 0xE68F, cbCheckBlockType});
	CPU::callbacks.push_back({0, 0xEF44, cbSkip2});		// Wait after disk insertion
	CPU::callbacks.push_back({0, 0xEFAF, cbWaitLicense});
	CPU::callbacks.push_back({0, 0xF46E, cbSkip2});		// License check
	
	TCHAR patFileName[MAX_PATH];
	int i =GetModuleFileName(NULL, patFileName, MAX_PATH);
	if (i) {
		while (i) if (patFileName[--i] =='\\') break;
		patFileName[i++] ='\\';
		patFileName[i++] =0;
		_tcscat(patFileName, L"fastload.cfg");
		FILE *handle =_tfopen(patFileName, _T("rt,ccs=UTF-16LE"));
		if (handle) {
			std::vector<wchar_t> text;
			wint_t c;
			while (1) {
				c =fgetwc(handle);
				if (c ==WEOF) break;
				text.push_back(c);
			};
			wchar_t gameNameW[16];
			std::mbstowcs(gameNameW, NES::fdsGameID, 16);
			std::wstring name =gameNameW;
			if (NES::currentPlugThruDevice !=ID_PLUG_FFE_SMC) Cheats::findFDSCheats(name);
			
			patchDefinition =FDSPatch::Game();
			patchDefinition.clear();
			
			std::vector<FDSPatch::Game> patDefinitions =FDSPatch::parsePAT(text);
			for (auto& game: patDefinitions) if (game.name ==name) {
				patchDefinition =game;
				break;
			}
			fclose(handle);
		} else
			EI.DbgOut(L"Could not open dip.cfg in emulator directory.");
	}
}

