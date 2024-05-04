#include "FDSFile.hpp"

void FDSProtection::notify(const FDSBlock& block) {
	switch (block.type) {
		case 1:
			oujiProtection =autodetectProtection &&
			                block.data.at(15 +0) =='O' &&
			                block.data.at(15 +1) =='U' &&
					block.data.at(15 +2) =='J' &&
					block.data.at(15 +3) =='I';
			kgkProtection =autodetectProtection &&
			               block.data.at(15 +0) =='K' &&
			               block.data.at(15 +1) =='G' &&
			               block.data.at(15 +2) =='K' &&
			               block.data.at(15 +3) ==' ';
			expectMagicCardTrainer =false;
			count2000 =0;
			break;
		case 2:
			filesLeft =block.data.at(0);
			break;
		case 3:
			nextFileAddress =block.data.at(10) |(block.data.at(11) <<8);
			nextFileSize =block.data.at(12) |(block.data.at(13) <<8);
			nextFileType =block.data.at(14) &3;
			if (substituteFileSizes && filesLeft) // On Venus-format disks, the hidden file's size must not be substituted
				nextFileSize =nextFileType !=0? (ppuFileSize? ppuFileSize: nextFileSize): cpuFileSize;
			else
				if (oujiProtection && nextFileType ==0 && nextFileAddress ==0x2000 && nextFileSize ==1 && ++count2000 >=3) nextFileSize =filesLeft? 0xC000: 0xE000;
			if (filesLeft) filesLeft--;
			if (filesLeft ==0 && nextFileAddress ==0x2000 && nextFileSize ==0x900 && nextFileType ==0 &&
			    block.data.at(2) =='K' && 
			    block.data.at(3) =='I' && 
			    block.data.at(4) =='Y' && 
			    block.data.at(5) =='O' && 
			    block.data.at(6) =='N' && 
			    block.data.at(7) =='O' && 
			    block.data.at(8) =='.')
				quickHunterProtection =true;
			break;
		case 4:
			if (autodetectProtection && nextFileType ==0 && nextFileAddress ==0x4FFF && nextFileSize ==8) {
				substituteFileSizes =block.data.at(0) &0x80;
				expectMagicCardTrainer =block.data.at(0) !=0xFE && (block.data.at(0) &0xC0) ==0xC0 && block.data.at(7) ==0x00;
				cpuFileSize =0x8000;
				switch(block.data.at(1) >>5) {
					case 4:
					case 5: ppuFileSize =32768; break;
					case 6: ppuFileSize =16384; break;
					case 7: ppuFileSize =8192; break;
					default:
					        ppuFileSize =0; break;
				}
			}
			break;
	}
}

uint16_t FDSProtection::getFileSize() const {
	return nextFileSize;
}

bool FDSProtection::epiloguePresent () const {
	return expectMagicCardTrainer || kgkProtection || quickHunterProtection;
}

uint8_t FDSProtection::epilogueType () const {
	return expectMagicCardTrainer? 5: kgkProtection? 0x12: quickHunterProtection? 0x00: 0;
}

size_t FDSProtection::epilogueSize () const {
	return expectMagicCardTrainer? 512: kgkProtection? 0x500: quickHunterProtection? 0x3000: 0;
}
