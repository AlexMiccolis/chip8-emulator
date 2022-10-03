#include "Disassembler.h"

static constexpr char s_HexDigits[] = "0123456789ABCDEF";

std::string Disassemble(const uint8_t* code, size_t length, int origin)
{
    std::string disassembly;

    if (reinterpret_cast<uintptr_t>(code) & 0x1)
    {
        if (length > 1)
        {
            ++code;
            --length;
        }
        else
            return disassembly;
    }

    for (int i = 0; i < length; i += 2)
    {
        int ip = origin + i;
        char lineBuffer[80] = { };
        char insBuffer[32] = { };
        Instruction ins(((int)code[i] << 8) | code[i + 1]);
        const char* name = Instruction::GetName(ins.type).c_str();

        switch (ins.type)
        {
        case Instruction::Type::CLS:
        case Instruction::Type::RET:
            sprintf_s(insBuffer, name);
            break;
        case Instruction::Type::JP:
        case Instruction::Type::CALL:
            sprintf_s(insBuffer, "%s #0x%03X", name, ins.address);
            break;
        case Instruction::Type::JP_V0_IMM:
            sprintf_s(insBuffer, "JP V0, #0x%03X", ins.address);
            break;
        case Instruction::Type::SE:
            if (ins.encoding == Instruction::Encoding::DestinationByte)
                sprintf_s(insBuffer, "SE V%c, #0x%02X", s_HexDigits[ins.dst], ins.byte);
            else if (ins.encoding == Instruction::Encoding::DestinationSource)
                sprintf_s(insBuffer, "SE V%c, V%c", s_HexDigits[ins.dst], s_HexDigits[ins.src]);
            break;
        case Instruction::Type::SNE:
            if (ins.encoding == Instruction::Encoding::DestinationByte)
                sprintf_s(insBuffer, "SNE V%c, #0x%02X", s_HexDigits[ins.dst], ins.byte);
            else if (ins.encoding == Instruction::Encoding::DestinationSource)
                sprintf_s(insBuffer, "SNE V%c, V%c", s_HexDigits[ins.dst], s_HexDigits[ins.src]);
            break;
        case Instruction::Type::LD:
            if (ins.encoding == Instruction::Encoding::DestinationByte)
                sprintf_s(insBuffer, "LD V%c, #0x%02X", s_HexDigits[ins.dst], ins.byte);
            else if (ins.encoding == Instruction::Encoding::DestinationSource)
                sprintf_s(insBuffer, "LD V%c, V%c", s_HexDigits[ins.dst], s_HexDigits[ins.src]);
            break;
        case Instruction::Type::ADD:
            if (ins.encoding == Instruction::Encoding::DestinationByte)
                sprintf_s(insBuffer, "ADD V%c, #0x%02X", s_HexDigits[ins.dst], ins.byte);
            else if (ins.encoding == Instruction::Encoding::DestinationSource)
                sprintf_s(insBuffer, "ADD V%c, V%c", s_HexDigits[ins.dst], s_HexDigits[ins.src]);
            break;
        case Instruction::Type::OR:
        case Instruction::Type::AND:
        case Instruction::Type::XOR:
        case Instruction::Type::SUB:
        case Instruction::Type::SUBN:
            sprintf_s(insBuffer, "%s V%c, V%c", name, s_HexDigits[ins.dst], s_HexDigits[ins.src]);
            break;
        case Instruction::Type::SHR:
        case Instruction::Type::SHL:
        case Instruction::Type::SKP:
        case Instruction::Type::SKNP:
            sprintf_s(insBuffer, "%s V%c", name, s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::RND:
            sprintf_s(insBuffer, "RND V%c, #0x%02X", s_HexDigits[ins.dst], ins.byte);
            break;
        case Instruction::Type::DRW:
            sprintf_s(insBuffer, "DRW V%c, V%c, %d",
                s_HexDigits[ins.dst],
                s_HexDigits[ins.src],
                ins.byte & 0xF);
            break;
        case Instruction::Type::LD_F_V:
            sprintf_s(insBuffer, "LD F, V%c", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_B_V:
            sprintf_s(insBuffer, "LD B, V%c", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_I_IMM:
            sprintf_s(insBuffer, "LD I, #0x%03X", ins.address);
            break;
        case Instruction::Type::LD_I_V0V:
            sprintf_s(insBuffer, "LD [I], V%c", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_V0V_I:
            sprintf_s(insBuffer, "LD V%c, [I]", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_V_DT:
            sprintf_s(insBuffer, "LD V%c, DT", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_V_K:
            sprintf_s(insBuffer, "LD V%c, K", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_DT_V:
            sprintf_s(insBuffer, "LD DT, V%c", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::LD_ST_V:
            sprintf_s(insBuffer, "LD ST, V%c", s_HexDigits[ins.dst]);
            break;
        case Instruction::Type::ADD_I_V:
            sprintf_s(insBuffer, "ADD I, V%c", s_HexDigits[ins.dst]);
            break;
        default:
            sprintf_s(insBuffer, ".BYTE #0x%02X", code[i]);
            --i;
            break;
        }

        sprintf_s(lineBuffer, "0x%03X: %s\n", ip, insBuffer);
        disassembly += lineBuffer;
    }

    return disassembly;
}