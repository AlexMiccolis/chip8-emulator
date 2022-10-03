#pragma once

#include <cstddef>
#include <cstdint>

struct Instruction
{
    enum class Encoding : uint32_t
    {
        None,
        Address,
        Destination,
        DestinationByte,
        DestinationSource,
        DestinationSourceNibble
    };

    enum class Type : uint32_t
    {
        UNKNOWN = 0,
        CLS,
        RET,
        JP,
        JP_V0_IMM,
        CALL,
        SE,
        SNE,
        LD,
        ADD,
        OR,
        AND,
        XOR,
        SUB,
        SHR,
        SUBN,
        SHL,
        RND,
        DRW,
        SKP,
        SKNP,
        LD_F_V,
        LD_B_V,
        LD_I_IMM,
        LD_I_V0V,
        LD_V0V_I,
        LD_V_DT,
        LD_V_K,
        LD_DT_V,
        LD_ST_V,
        ADD_I_V,
        _END
    };

    static inline const std::string& GetName(Type type)
    {
        static const std::string InstructionNames[] = {
            "???",
            "CLS",
            "RET",
            "JP",
            "JP",
            "CALL",
            "SE",
            "SNE",
            "LD",
            "ADD",
            "OR",
            "AND",
            "XOR",
            "SUB",
            "SHR",
            "SUBN",
            "SHL",
            "RND",
            "DRW",
            "SKP",
            "SKNP",
            "LD",
            "LD",
            "LD",
            "LD",
            "LD",
            "LD",
            "LD",
            "LD",
            "LD",
            "ADD"
        };
        return InstructionNames[(int)type];
    }

    uint16_t instruction;
    uint16_t address;
    uint8_t  opcode;
    uint8_t  dst;
    uint8_t  src;
    uint8_t  byte;
    Type     type;
    Encoding encoding;

    Instruction() = default;
    Instruction(uint16_t _instruction)
        : type(Type::UNKNOWN), encoding(Encoding::None)
    {
        static constexpr Type aluLookup[16] = {
            Type::LD, Type::OR, Type::AND,
            Type::XOR, Type::ADD, Type::SUB,
            Type::SHR, Type::SUBN, Type::UNKNOWN,
            Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
            Type::UNKNOWN, Type::UNKNOWN, Type::SHL
        };

        static constexpr Type ctrlLookup[128] = {
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::LD_V_DT, Type::UNKNOWN,
                Type::UNKNOWN, Type::LD_V_K, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::LD_DT_V, Type::UNKNOWN, Type::UNKNOWN,
                Type::LD_ST_V, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::ADD_I_V, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::LD_F_V,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::LD_B_V, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::LD_I_V0V, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::UNKNOWN,
                Type::UNKNOWN, Type::UNKNOWN, Type::LD_V0V_I,
        };

        instruction = _instruction;
        address = instruction & 0xFFF;
        opcode = instruction >> 12;
        dst = (instruction >> 8) & 0xF;
        src = (instruction >> 4) & 0xF;
        byte = instruction & 0xFF;

        switch (opcode)
        {
        case 0x0:
            if (byte == 0xE0)
                type = Type::CLS;
            else if (byte == 0xEE)
                type = Type::RET;
            break;
        case 0x1:
            type = Type::JP;
            encoding = Encoding::Address;
            break;
        case 0x2:
            type = Type::CALL;
            encoding = Encoding::Address;
            break;
        case 0x3:
            type = Type::SE;
            encoding = Encoding::DestinationByte;
            break;
        case 0x4:
            type = Type::SNE;
            encoding = Encoding::DestinationByte;
            break;
        case 0x5:
            type = Type::SE;
            encoding = Encoding::DestinationSource;
            break;
        case 0x6:
            type = Type::LD;
            encoding = Encoding::DestinationByte;
            break;
        case 0x7:
            type = Type::ADD;
            encoding = Encoding::DestinationByte;
            break;
        case 0x8:
            type = aluLookup[byte & 0x0F];
            encoding = Encoding::DestinationSource;
            break;
        case 0x9:
            type = Type::SNE;
            encoding = Encoding::DestinationSource;
            break;
        case 0xA:
            type = Type::LD_I_IMM;
            encoding = Encoding::Address;
            break;
        case 0xB:
            type = Type::JP_V0_IMM;
            encoding = Encoding::Address;
            break;
        case 0xC:
            type = Type::RND;
            encoding = Encoding::DestinationByte;
            break;
        case 0xD:
            type = Type::DRW;
            encoding = Encoding::DestinationSourceNibble;
            break;
        case 0xE:
            if (byte == 0x9E)
                type = Type::SKP;
            else if (byte == 0xA1)
                type = Type::SKNP;
            encoding = Encoding::Destination;
            break;
        case 0xF:
            type = ctrlLookup[byte & 0x7f];
            encoding = Encoding::Destination;
            break;
        }
    }
};