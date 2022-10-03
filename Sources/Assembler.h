#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include "Tokenizer.h"

struct AssemblerState
{
	std::unordered_map<std::string, uint16_t> symbols;
	std::vector<Token> tokens;

	int line;
	int address;

	AssemblerState(int origin = 0)
		: line(0), address(origin) { }
};

void AssemblerError(int line, const char* fmt, ...);
void AssemblerWarning(int line, const char* fmt, ...);
bool Assemble(std::vector<uint8_t>& bytesOut, const std::string& code, int origin = 0x200);