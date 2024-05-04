#include "stdafx.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "Wordify.hpp"

std::vector<std::wstring> wordify(const std::vector<wchar_t>& text) {
	// Turn vector of wchar_t into vector of words, processing ; as comment, " as quote, \ as escape
	std::wstring word;
	std::vector<std::wstring> words;
	
	bool inComment =false;
	bool inQuote =false;
	bool inEscape =false;
	int line =1;
	
	for (auto& origC: text) {	// For every character
		auto c =origC;		// keep the original character in origC and examine the slightly recoded c
		// Treat LF like CR and TAB like SPACE		
		if (c ==0x0D) c =0x0A;
		if (c ==0x09) c =' ';
		if (c ==0x0A && inComment) inComment =false;
		
		if (inComment && c !=0x0A)
			;
		else
		if (c =='"' && !inEscape)
			inQuote =!inQuote;
		else
		if (c ==';' && !inEscape && !inQuote)
			inComment =true;
		else
		if (c =='\\' && !inEscape)
			inEscape =true;
		else			
		if ((c ==' ' || c ==0x0A) && !inEscape && !inQuote) {
			if (word.size()) words.push_back(word);
			word.clear();
		}
		else {
			word.push_back(origC);
			inEscape =false;
		}
		if (origC ==0x0A) {
			if (inQuote) {
				EI.DbgOut(L"Warning: Line %d ends without closing quote. Last word was \"%s\"", line, words.back().c_str());
				inQuote =false;
			}
			line++;
		}
	}
	if (word.size()) words.push_back(word);
	return words;
}
