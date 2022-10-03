#pragma once

enum class Keyword : int
{
	CLS,
	RET,
	SYS,
	JP,
	CALL,
	SE,
	SNE,
	LD,
	ADD,
	OR,
	XOR,
	AND,
	SUB,
	SHL,
	SHR,
	SUBN,
	RND,
	DRW,
	SKP,
	SKNP,

	/* Registers */
	V0, V1, V2, V3, V4, V5, V6, V7, V8, V9,
	VA, VB, VC, VD, VE, VF, DT, ST, I, K, F, B
};

struct Token
{
	enum class Type : int
	{
		None,
		Label,      // Label definition
		Keyword,    // Instruction name or register name
		Directive,  // Directives like .BYTE and .WORD
		Immediate,  // Immediate values like #5 or #0x200
		Identifier, // Label identifier
		Expression
	};

	std::string  text;
	Type         type;
	int          line;
	int          address;

	union
	{
		Keyword      keyword;
		int          value;
	};

	Token(const std::string& _text, Type _type = Type::Identifier, int _line = 0, int _value = 0, uint16_t _address = 0)
		: type(_type), text(_text), line(_line), value(_value), address(_address) { }
};

bool TokenizeLine(std::vector<Token>& tokensOut, int nLine, char* str);
bool TokenizeCode(std::vector<Token>& tokensOut, const std::string& str);