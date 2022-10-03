#pragma once

#include <string>
#include "Instruction.h"

std::string Disassemble(const uint8_t* code, size_t length, int origin);