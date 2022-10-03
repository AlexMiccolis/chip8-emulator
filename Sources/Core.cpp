#include "Core.h"
#include "Instruction.h"

Core::Core()
    : m_Registers{ }, m_DisplayDirty(true), m_WaitingForKey(false), m_KeyDst(0), m_KeyStates(0)
{
    std::fill_n(m_DisplayBitmap.begin(), m_DisplayBitmap.size(), 0);
    std::fill_n(m_DisplayBuffer.begin(), m_DisplayBuffer.size(), 0);
    std::copy_n(s_CharSprites.begin(), s_CharSprites.size(), m_Memory.begin());
}

Core::~Core()
{

}

void Core::DoCycle()
{
    Instruction ins(ReadWord(m_Registers.ip));
    int      temp = 0;
    int      pcInc = 2;

    switch (ins.type)
    {
    case Instruction::Type::CLS:
        std::fill_n(m_DisplayBitmap.begin(), m_DisplayBitmap.size(), 0);
        //m_DisplayDirty = true;
        break;
    case Instruction::Type::RET:
        m_Registers.ip = m_Registers.stack[--m_Registers.sp];
        pcInc = 0;
        break;
    case Instruction::Type::JP:
        m_Registers.ip = ins.address;
        pcInc = 0;
        break;
    case Instruction::Type::JP_V0_IMM:
        m_Registers.ip = ins.address + m_Registers.v[0];
        pcInc = 0;
        break;
    case Instruction::Type::CALL:
        m_Registers.stack[m_Registers.sp++] = m_Registers.ip + 2;
        m_Registers.ip = ins.address;
        pcInc = 0;
        break;
    case Instruction::Type::SE:
        if (ins.encoding == Instruction::Encoding::DestinationByte)
        {
            if (m_Registers.v[ins.dst] == ins.byte)
                pcInc = 4;
        }
        else if (ins.encoding == Instruction::Encoding::DestinationSource)
        {
            if (m_Registers.v[ins.dst] == m_Registers.v[ins.src])
                pcInc = 4;
        }
        break;
    case Instruction::Type::SNE:
        if (ins.encoding == Instruction::Encoding::DestinationByte)
        {
            if (m_Registers.v[ins.dst] != ins.byte)
                pcInc = 4;
        }
        else if (ins.encoding == Instruction::Encoding::DestinationSource)
        {
            if (m_Registers.v[ins.dst] != m_Registers.v[ins.src])
                pcInc = 4;
        }
        break;
    case Instruction::Type::LD:
        if (ins.encoding == Instruction::Encoding::DestinationByte)
            m_Registers.v[ins.dst] = ins.byte;
        else if (ins.encoding == Instruction::Encoding::DestinationSource)
            m_Registers.v[ins.dst] = m_Registers.v[ins.src];
        break;
    case Instruction::Type::ADD:
        if (ins.encoding == Instruction::Encoding::DestinationByte)
            m_Registers.v[ins.dst] += ins.byte;
        else if (ins.encoding == Instruction::Encoding::DestinationSource)
        {
            temp = static_cast<int>(m_Registers.v[ins.dst]) + m_Registers.v[ins.src];
            m_Registers.v[0xf] = temp > 255;
            m_Registers.v[ins.dst] = temp & 0xff;
        }
        break;
    case Instruction::Type::OR:
        m_Registers.v[ins.dst] |= m_Registers.v[ins.src];
        break;
    case Instruction::Type::AND:
        m_Registers.v[ins.dst] &= m_Registers.v[ins.src];
        break;
    case Instruction::Type::XOR:
        m_Registers.v[ins.dst] ^= m_Registers.v[ins.src];
        break;
    case Instruction::Type::SUB:
        m_Registers.v[0xf] = m_Registers.v[ins.dst] > m_Registers.v[ins.src];
        m_Registers.v[ins.dst] = m_Registers.v[ins.dst] - m_Registers.v[ins.src];
        break;
    case Instruction::Type::SHR:
        m_Registers.v[0xf] = m_Registers.v[ins.dst] & 0x1;
        m_Registers.v[ins.dst] >>= 2;
        break;
    case Instruction::Type::SUBN:
        m_Registers.v[0xf] = m_Registers.v[ins.src] > m_Registers.v[ins.dst];
        m_Registers.v[ins.dst] = m_Registers.v[ins.src] - m_Registers.v[ins.dst];
        break;
    case Instruction::Type::SHL:
        m_Registers.v[0xf] = !!(m_Registers.v[ins.dst] & 0x80);
        m_Registers.v[ins.dst] <<= 2;
        break;
    case Instruction::Type::RND:
        m_Registers.v[ins.dst] = (rand() % 255) & ins.byte;
        break;
    case Instruction::Type::DRW:
        m_Registers.v[0xF] = DrawSprite(m_Registers.v[ins.dst],
            m_Registers.v[ins.src],
            m_Registers.i,
            ins.byte & 0x0F);
        break;
    case Instruction::Type::SKP:
        if (m_KeyStates & (1 << m_Registers.v[ins.dst]))
            pcInc = 4;
        break;
    case Instruction::Type::SKNP:
        if ((m_KeyStates & (1 << m_Registers.v[ins.dst])) == 0)
            pcInc = 4;
        break;
    case Instruction::Type::LD_F_V:
        m_Registers.i = (m_Registers.v[ins.dst] & 0xF) * 5;
        break;
    case Instruction::Type::LD_B_V:
        // TODO: Implement LD_B_V
        break;
    case Instruction::Type::LD_I_IMM:
        m_Registers.i = ins.address;
        break;
    case Instruction::Type::LD_I_V0V:
    {
        int i = 0;
        int d = m_Registers.i;
        do {
            WriteByte(d++, m_Registers.v[i]);
        } while (i++ < ins.dst);
        break;
    }
    case Instruction::Type::LD_V0V_I:
    {
        int i = 0;
        int d = m_Registers.i;
        do {
            m_Registers.v[i] = ReadByte(d++);
        } while (i++ < ins.dst);
        break;
    }
    case Instruction::Type::LD_V_DT:
        m_Registers.v[ins.dst] = m_Registers.dt;
        break;
    case Instruction::Type::LD_V_K:
        m_KeyDst = ins.dst;
        m_WaitingForKey = true;
        break;
    case Instruction::Type::LD_DT_V:
        m_Registers.dt = m_Registers.v[ins.dst];
        break;
    case Instruction::Type::LD_ST_V:
        m_Registers.st = m_Registers.v[ins.dst];
        break;
    case Instruction::Type::ADD_I_V:
        m_Registers.i += m_Registers.v[ins.dst];
        break;
    default:
        break;
    }

    m_Registers.ip += pcInc;
    return;
}

void Core::LoadData(const uint8_t* data, size_t length, uint16_t memoryOffset)
{
    if ((length + memoryOffset) > sizeof(m_Memory))
        length = sizeof(m_Memory) - memoryOffset;
    memcpy(m_Memory.data() + memoryOffset, data, length);
}

bool Core::UpdateDisplay()
{
    if (m_DisplayDirty)
    {
        for (int i = 0; i < DisplayWidth * DisplayHeight; i++)
        {
            int pixel = i * 4;
            int bit = 1 << (i % 8);
            int byte = i / 8;
            int color = 0;

            if (m_DisplayBitmap[byte] & bit)
                color = 255;

            m_DisplayBuffer[pixel] = color;
            m_DisplayBuffer[pixel + 1] = color;
            m_DisplayBuffer[pixel + 2] = color;
            m_DisplayBuffer[pixel + 3] = 255;
        }
        m_DisplayDirty = false;
        return true;
    }
    return false;
}

bool Core::DrawSprite(int x, int y, int address, int length)
{
    bool vf = false;

    /* Read the sprite from memory */
    for (int i = 0; i < length; i++)
    {
        uint8_t row = ReadByte(address + i);

        for (int c = 7; c >= 0; c--)
        {
            bool result = XorPixel((x + (7 - c)) % DisplayWidth, y % DisplayHeight, (row & (1 << c)));
            if (!vf)
                vf = result;
        }

        ++y;
    }

    m_DisplayDirty = true;
    return vf;
}