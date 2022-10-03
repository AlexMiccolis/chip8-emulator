#include <unordered_map>
#include "Instruction.h"
#include "Assembler.h"

static constexpr uint16_t Opcode_CLS = 0x00E0;
static constexpr uint16_t Opcode_RET = 0x00EE;

static constexpr uint16_t Opcode_JP_addr = 0x1000;
static constexpr uint16_t Opcode_CALL_addr = 0x2000;
static constexpr uint16_t Opcode_LD_I_addr = 0xA000;
static constexpr uint16_t Opcode_JP_V0_addr = 0xB000;

static constexpr uint16_t Opcode_SE_dst_imm = 0x3000;
static constexpr uint16_t Opcode_SNE_dst_imm = 0x4000;
static constexpr uint16_t Opcode_LD_dst_imm = 0x6000;
static constexpr uint16_t Opcode_ADD_dst_imm = 0x7000;

static constexpr uint16_t Opcode_SE_dst_src = 0x5000;
static constexpr uint16_t Opcode_LD_dst_src = 0x8000;
static constexpr uint16_t Opcode_OR_dst_src = 0x8001;
static constexpr uint16_t Opcode_AND_dst_src = 0x8002;
static constexpr uint16_t Opcode_XOR_dst_src = 0x8003;
static constexpr uint16_t Opcode_ADD_dst_src = 0x8004;
static constexpr uint16_t Opcode_SUB_dst_src = 0x8005;
static constexpr uint16_t Opcode_SHR_dst_src = 0x8006;
static constexpr uint16_t Opcode_SUBN_dst_src = 0x8007;
static constexpr uint16_t Opcode_SHL_dst_src = 0x800E;
static constexpr uint16_t Opcode_SNE_dst_src = 0x9000;

static constexpr uint16_t Opcode_DRW_dst_src_nib = 0xD000;

static constexpr uint16_t Opcode_SKP_dst = 0xE09E;
static constexpr uint16_t Opcode_SKNP_dst = 0xE0A1;

static constexpr uint16_t Opcode_LD_dst_DT = 0xF007;
static constexpr uint16_t Opcode_LD_dst_K = 0xF00A;
static constexpr uint16_t Opcode_LD_DT_dst = 0xF015;
static constexpr uint16_t Opcode_LD_ST_dst = 0xF018;
static constexpr uint16_t Opcode_ADD_I_dst = 0xF01E;
static constexpr uint16_t Opcode_LD_F_dst = 0xF029;
static constexpr uint16_t Opcode_LD_B_dst = 0xF033;
static constexpr uint16_t Opcode_LD_I_regs = 0xF055;
static constexpr uint16_t Opcode_LD_regs_I = 0xF065;

void AssemblerError(int line, const char* fmt, ...)
{
    char errorBuf[1024];
    va_list args;

    va_start(args, fmt);
    vsprintf_s(errorBuf, fmt, args);
    printf("ERROR at line %d: %s\n", line, errorBuf);
    va_end(args);
}

void AssemblerWarning(int line, const char* fmt, ...)
{
    char errorBuf[1024];
    va_list args;

    va_start(args, fmt);
    vsprintf_s(errorBuf, fmt, args);
    printf("WARNING at line %d: %s\n", line, errorBuf);
    va_end(args);
}

static Instruction::Type GetInstructionType(
    const Token& token,
    const Token* dst,
    const Token* src
)
{
    Token::Type dstType = (dst != nullptr) ? dst->type : Token::Type::None;
    Token::Type srcType = (src != nullptr) ? src->type : Token::Type::None;

    /* Easy instructions */
    switch (token.keyword)
    {
    case Keyword::CLS:
        return Instruction::Type::CLS;
    case Keyword::RET:
        return Instruction::Type::RET;
    case Keyword::CALL:
        return Instruction::Type::CALL;
    case Keyword::SE:
        return Instruction::Type::SE;
    case Keyword::SNE:
        return Instruction::Type::SNE;
    case Keyword::ADD:
        return Instruction::Type::ADD;
    case Keyword::OR:
        return Instruction::Type::OR;
    case Keyword::XOR:
        return Instruction::Type::XOR;
    case Keyword::AND:
        return Instruction::Type::AND;
    case Keyword::SUB:
        return Instruction::Type::SUB;
    case Keyword::SHL:
        return Instruction::Type::SHL;
    case Keyword::SHR:
        return Instruction::Type::SHR;
    case Keyword::SUBN:
        return Instruction::Type::SUBN;
    case Keyword::RND:
        return Instruction::Type::RND;
    case Keyword::DRW:
        return Instruction::Type::DRW;
    case Keyword::SKP:
        return Instruction::Type::SKP;
    case Keyword::SKNP:
        return Instruction::Type::SKNP;
    }


    if (token.keyword == Keyword::JP)
    {
        if (dstType == Token::Type::Keyword && dst->text == "V0")
            return Instruction::Type::JP_V0_IMM;
        else if (dstType == Token::Type::Immediate || dstType == Token::Type::Identifier)
            return Instruction::Type::JP;
    }

    /* Probably the trickiest instruction with the most variations */
    if (token.keyword == Keyword::LD)
    {
        if (dstType == Token::Type::Keyword)
        {
            if (dst->keyword == Keyword::I && srcType == Token::Type::Immediate)
                return Instruction::Type::LD_I_IMM;
            else if (dst->keyword == Keyword::DT && srcType == Token::Type::Keyword)
                return Instruction::Type::LD_DT_V;
            else if (dst->keyword == Keyword::ST && srcType == Token::Type::Keyword)
                return Instruction::Type::LD_ST_V;
            else if (dst->keyword == Keyword::F && srcType == Token::Type::Keyword)
                return Instruction::Type::LD_F_V;
            else if (dst->keyword == Keyword::B && srcType == Token::Type::Keyword)
                return Instruction::Type::LD_B_V;
        }

        if (srcType == Token::Type::Keyword)
        {
            if (src->keyword == Keyword::K)
                return Instruction::Type::LD_V_K;
        }
        return Instruction::Type::LD;
    }

    return Instruction::Type::UNKNOWN;
}

static int AssembleDstInstruction(
    Instruction& instruction,
    const Token& token,
    const Token* dst)
{
    int vIndex = -1;
    int tokenLength = 2;

    if (!dst)
    {
        AssemblerError(token.line, "instruction missing destination operand");
        return 1;
    }

    if (dst->type != Token::Type::Keyword)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 2;
    }

    vIndex = static_cast<int>(dst->keyword) - static_cast<int>(Keyword::V0);
    if (vIndex < 0 || vIndex > 15)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 2;
    }

    instruction.encoding = Instruction::Encoding::Destination;
    instruction.dst = vIndex;
    switch (instruction.type)
    {
    case Instruction::Type::SHL:
        instruction.instruction = Opcode_SHL_dst_src;
        break;
    case Instruction::Type::SHR:
        instruction.instruction = Opcode_SHR_dst_src;
        break;
    case Instruction::Type::SKP:
        instruction.instruction = Opcode_SKP_dst;
        break;
    case Instruction::Type::SKNP:
        instruction.instruction = Opcode_SKNP_dst;
        break;
    case Instruction::Type::LD_F_V:
        instruction.instruction = Opcode_LD_F_dst;
        ++tokenLength;
        break;
    case Instruction::Type::LD_B_V:
        instruction.instruction = Opcode_LD_B_dst;
        ++tokenLength;
        break;
    case Instruction::Type::LD_V_K:
        instruction.instruction = Opcode_LD_dst_K;
        ++tokenLength;
        break;
    }

    return tokenLength;
}

static int AssembleDstSrcInstruction(
    Instruction& instruction,
    const Token& token,
    const Token* dst,
    const Token* src)
{
    int vIndex = -1;

    if (!dst)
    {
        AssemblerError(token.line, "instruction missing destination operand");
        return 1;
    }

    if (!src)
    {
        AssemblerError(token.line, "instruction missing source operand");
        return 2;
    }

    if (dst->type != Token::Type::Keyword)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 2;
    }

    vIndex = static_cast<int>(dst->keyword) - static_cast<int>(Keyword::V0);
    if (vIndex < 0 || vIndex > 15)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 2;
    }
    instruction.dst = vIndex;

    if (src->type == Token::Type::Immediate)
    {
        instruction.byte = static_cast<uint8_t>(src->value);
        instruction.encoding = Instruction::Encoding::DestinationByte;
        switch (instruction.type)
        {
        case Instruction::Type::SE:
            instruction.instruction = Opcode_SE_dst_imm;
            break;
        case Instruction::Type::SNE:
            instruction.instruction = Opcode_SNE_dst_imm;
            break;
        case Instruction::Type::LD:
            instruction.instruction = Opcode_LD_dst_imm;
            break;
        case Instruction::Type::ADD:
            instruction.instruction = Opcode_ADD_dst_imm;
            break;
        default:
            AssemblerError(token.line, "instruction '%s' has no immediate form", token.text.c_str());
            break;
        }
    }
    else if (src->type == Token::Type::Keyword)
    {
        vIndex = static_cast<int>(src->keyword) - static_cast<int>(Keyword::V0);
        if (vIndex < 0 || vIndex > 15)
        {
            AssemblerError(token.line, "invalid source register '%s'", dst->text.c_str());
            return 2;
        }

        instruction.src = vIndex;
        instruction.encoding = Instruction::Encoding::DestinationSource;

        switch (instruction.type)
        {
        case Instruction::Type::SE:
            instruction.instruction = Opcode_SE_dst_src;
            break;
        case Instruction::Type::SNE:
            instruction.instruction = Opcode_SNE_dst_src;
            break;
        case Instruction::Type::LD:
            instruction.instruction = Opcode_LD_dst_src;
            break;
        case Instruction::Type::ADD:
            instruction.instruction = Opcode_ADD_dst_src;
            break;
        case Instruction::Type::OR:
            instruction.instruction = Opcode_OR_dst_src;
            break;
        case Instruction::Type::AND:
            instruction.instruction = Opcode_AND_dst_src;
            break;
        case Instruction::Type::XOR:
            instruction.instruction = Opcode_XOR_dst_src;
            break;
        case Instruction::Type::SUB:
            instruction.instruction = Opcode_SUB_dst_src;
            break;
        case Instruction::Type::SUBN:
            instruction.instruction = Opcode_SUBN_dst_src;
            break;
        }
    }

    return 3;
}

static int AssembleDstSrcNibInstruction(
    Instruction& instruction,
    const Token& token,
    const Token* dst,
    const Token* src,
    const Token* nib)
{
    int vIndex = -1;

    if (!dst)
    {
        AssemblerError(token.line, "instruction missing destination operand");
        return 1;
    }
    else if (!src)
    {
        AssemblerError(token.line, "instruction missing source operand");
        return 2;
    }
    else if (!nib)
    {
        AssemblerError(token.line, "instruction missing 4-bit integer operand");
        return 3;
    }

    if (dst->type != Token::Type::Keyword)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 4;
    }
    else if (src->type != Token::Type::Keyword)
    {
        AssemblerError(token.line, "invalid source register '%s'", src->text.c_str());
        return 4;
    }
    else if (nib->type != Token::Type::Immediate)
    {
        AssemblerError(token.line, "invalid 4-bit immediate value '%s'", nib->text.c_str());
        return 4;
    }

    vIndex = static_cast<int>(dst->keyword) - static_cast<int>(Keyword::V0);
    if (vIndex < 0 || vIndex > 15)
    {
        AssemblerError(token.line, "invalid destination register '%s'", dst->text.c_str());
        return 4;
    }
    instruction.dst = vIndex;
    vIndex = static_cast<int>(src->keyword) - static_cast<int>(Keyword::V0);
    if (vIndex < 0 || vIndex > 15)
    {
        AssemblerError(token.line, "invalid source register '%s'", src->text.c_str());
        return 4;
    }
    instruction.src = vIndex;
    instruction.byte = nib->value;

    if (instruction.type == Instruction::Type::DRW)
    {
        instruction.instruction = Opcode_DRW_dst_src_nib;
    }

    instruction.encoding = Instruction::Encoding::DestinationSourceNibble;
    return 4;
}

static int AssembleAddrInstruction(
    Instruction& instruction,
    AssemblerState& state,
    const Token& token,
    const Token* addr)
{
    int vIndex = -1;
    int tokenLength = 1;

    if (!addr)
    {
        AssemblerError(token.line, "instruction missing address operand");
        return tokenLength;
    }

    tokenLength = 2;

    if (addr->type == Token::Type::Immediate)
    {
        instruction.address = addr->value;
    }
    else if (addr->type == Token::Type::Identifier)
    {
        if (state.symbols.count(addr->text) == 0)
        {
            AssemblerError(token.line, "use of undeclared identifier '%s'", addr->text.c_str());
            return 2;
        }

        instruction.address = state.symbols.at(addr->text);
    }

    switch (instruction.type)
    {
    case Instruction::Type::JP:
        instruction.instruction = Opcode_JP_addr;
        break;
    case Instruction::Type::JP_V0_IMM:
        instruction.instruction = Opcode_JP_V0_addr;
        ++tokenLength;
        break;
    case Instruction::Type::CALL:
        instruction.instruction = Opcode_CALL_addr;
        break;
    case Instruction::Type::LD_I_IMM:
        instruction.instruction = Opcode_LD_I_addr;
        ++tokenLength;
        break;
    }

    instruction.encoding = Instruction::Encoding::Address;
    return tokenLength;
}

/**
 * Assemble the instruction starting at tokens[index]
 * @param bytesOut Output vector
 * @param state    Assembler state
 * @param index    Token index
 * @return How many tokens long the instruction was, or -1 if there was an error
 */
static int AssembleInstruction(std::vector<uint8_t>& bytesOut, AssemblerState& state, int index)
{
    const Token& token = state.tokens[index];
    const Token* dst = ((index + 1) < state.tokens.size())
        ? &state.tokens[index + 1]
        : nullptr;
    const Token* src = ((index + 2) < state.tokens.size())
        ? &state.tokens[index + 2]
        : nullptr;
    const Token* nibble = ((index + 3) < state.tokens.size())
        ? &state.tokens[index + 3]
        : nullptr;

    Instruction instruction{ };
    int tokenLength = 1;

    instruction.type = GetInstructionType(token, dst, src);
    switch (instruction.type)
    {
    case Instruction::Type::CLS:
        instruction.instruction = Opcode_CLS;
        break;
    case Instruction::Type::RET:
        instruction.instruction = Opcode_RET;
        break;
    case Instruction::Type::JP:
    case Instruction::Type::CALL:
        tokenLength = AssembleAddrInstruction(instruction, state, token, dst);
        break;
    case Instruction::Type::JP_V0_IMM:
    case Instruction::Type::LD_I_IMM:
        tokenLength = AssembleAddrInstruction(instruction, state, token, src);
        break;
    case Instruction::Type::LD_F_V:
    case Instruction::Type::LD_B_V:
        tokenLength = AssembleDstInstruction(instruction, token, src);
        break;
    case Instruction::Type::SKP:
    case Instruction::Type::SKNP:
    case Instruction::Type::SHL:
    case Instruction::Type::SHR:
    case Instruction::Type::LD_V_K:
    case Instruction::Type::LD_V_DT:
        tokenLength = AssembleDstInstruction(instruction, token, dst);
        break;
    case Instruction::Type::LD:
    case Instruction::Type::OR:
    case Instruction::Type::SE:
    case Instruction::Type::SNE:
    case Instruction::Type::ADD:
    case Instruction::Type::SUB:
    case Instruction::Type::AND:
    case Instruction::Type::XOR:
    case Instruction::Type::SUBN:
        tokenLength = AssembleDstSrcInstruction(instruction, token, dst, src);
        break;
    case Instruction::Type::DRW:
        tokenLength = AssembleDstSrcNibInstruction(instruction, token, dst, src, nibble);
        break;
    }

    /* "Assemble" the instruction based on its encoding */
    if (instruction.encoding == Instruction::Encoding::Address)
        instruction.instruction |= (instruction.address & 0xFFF);
    else if (instruction.encoding == Instruction::Encoding::Destination)
        instruction.instruction |= (instruction.dst << 8);
    else if (instruction.encoding == Instruction::Encoding::DestinationByte)
        instruction.instruction |= (instruction.dst << 8) | instruction.byte;
    else if (instruction.encoding == Instruction::Encoding::DestinationSource)
        instruction.instruction |= (instruction.dst << 8) | (instruction.src << 4);
    else if (instruction.encoding == Instruction::Encoding::DestinationSourceNibble)
        instruction.instruction |= (instruction.dst << 8) | (instruction.src << 4) | (instruction.byte & 0x0F);

    /* Write instruction to byte buffer */
    bytesOut.push_back(instruction.instruction >> 8);
    bytesOut.push_back(instruction.instruction & 0xFF);

    return tokenLength;
}

bool Assemble(std::vector<uint8_t>& bytesOut, const std::string& code, int origin)
{
    AssemblerState state(origin);
    int index = 0;

    bytesOut.reserve(1024);

    if (!TokenizeCode(state.tokens, code))
    {
        return false;
    }

    while (index < state.tokens.size())
    {
        const Token& token = state.tokens[index];
        int stride = 1;

        state.symbols["$"] = state.address;

        switch (token.type)
        {
        case Token::Type::Label:
            state.symbols[token.text] = state.address;
            break;
        case Token::Type::Keyword:
            stride = AssembleInstruction(bytesOut, state, index);
            if (stride == -1)
                return false;
            state.address += 2;
            break;
        case Token::Type::Immediate:
            AssemblerError(token.line, "stray immediate value '%s'", token.text.c_str());
            return false;
        case Token::Type::Expression:
            AssemblerError(token.line, "stray expression '%s'", token.text.c_str());
            return false;
        }

        index += stride;
    }

    return true;
}