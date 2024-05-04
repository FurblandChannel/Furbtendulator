#include "stdafx.h"
#include <time.h>
#include <windowsx.h>
#include <vector>
#include <algorithm>
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "Wordify.hpp"
#include "FDS_Patch.h"

namespace FDSPatch {
void Item::clear() {
	address =0;
	compare.clear();
	replace.clear();	
}

void Game::clear() {
	name =L"";
	ignoreNMI =false;
	items.clear();
}

std::vector<Game> parsePAT(const std::vector<wchar_t>& text) {
	std::vector<std::wstring> words =wordify(text);
	
	std::vector<Game> games;
	Game game;
	Item item;
	std::wstring field;
	
	for (auto& word: words) {
		if (word ==L"game") {
			if (item.replace.size()) { game.items.push_back(item); item.clear(); }
			if (game.name != L"" && (game.items.size() || game.ignoreNMI)) { games.push_back(game); game.clear(); }
			field =word;
		} else
		if (word ==L"address") {
			if (item.replace.size()) { game.items.push_back(item); item.clear(); }
			field =word;
		} else
		if (word ==L"compare" || word ==L"replace" || word ==L"ignoreNMI") {
			field =word;
		} else
		if (field !=L"") {
			if (field ==L"ignoreNMI") {
				game.ignoreNMI =!!wcstoul(word.c_str(), NULL, 0);
				field =L"";
			} else
			if (field ==L"game") {
				game.name =word;
				field =L"";
			} else
			if (field ==L"address") {
				item.address =wcstoul(word.c_str(), NULL, 16);
				field =L"";
			} else
			if (field ==L"compare")
				item.compare.push_back(wcstoul(word.c_str(), NULL, 16));
			else
			if (field ==L"replace")
				item.replace.push_back(wcstoul(word.c_str(), NULL, 16));
		} else
			EI.DbgOut(L"fastload.cfg: Syntax error: %s (game was \"%s\")", word.c_str(), game.name.c_str());
	}
	if (item.replace.size()) { game.items.push_back(item); item.clear(); }
	if (game.name !=L"" && (game.items.size() || game.ignoreNMI)) { games.push_back(game); game.clear(); }
	
	// Plausibility checks
	// Check for name values that have been assigned to more than one game
	for (unsigned int i =0; i <games.size(); i++) {
		for (unsigned int j =i+1; j<games.size(); j++) {
			if (i !=j && games[i].name ==games[j].name) EI.DbgOut(L"Game '%s' appears more than once", games[i].name.c_str(), games[j].name.c_str());
		}
	}
	return games;
}

} // namespace FDSPatch