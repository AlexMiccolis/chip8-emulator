#pragma once

#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>
#include <string>

class Core
{
public:
    constexpr static int MemorySize = 4096;
    constexpr static int DisplayWidth = 64;
    constexpr static int DisplayHeight = 32;
    constexpr static int DisplayBitmapSize = (DisplayWidth * DisplayHeight) / 8;

    Core();
    ~Core();

    void DoCycle();
    void LoadData(const uint8_t* data, size_t length, uint16_t memoryOffset);
    void LoadData(const std::vector<uint8_t>& data, uint16_t memoryOffset) { return LoadData(data.data(), data.size(), memoryOffset); }

    bool UpdateDisplay();
    const auto& GetDisplayBuffer() { return m_DisplayBuffer; }

    /** Set instruction pointer */
    void SetIp(uint16_t address) { m_Registers.ip = address; }

    /** Get instruction pointer */
    uint16_t GetIp(uint16_t address) const { return m_Registers.ip; }

    void KeyDown(uint8_t key)
    {
        m_KeyStates |= (1 << key);
        if (m_WaitingForKey)
        {
            m_Registers.v[m_KeyDst] = key;
            m_WaitingForKey = false;
        }
    }

    void KeyUp(uint8_t key)
    {
        m_KeyStates &= ~(1 << key);
    }

    void DecrementDT()
    {
        if (m_Registers.dt)
            --m_Registers.dt;
    }

    bool WaitingForKey() const { return m_WaitingForKey; }

    uint8_t ReadByte(uint16_t address) const
    {
        if (address < sizeof(m_Memory))
            return m_Memory[address];
        return 0;
    }

    uint16_t ReadWord(uint16_t address) const
    {
        if (address < sizeof(m_Memory))
            return _byteswap_ushort(*(const uint16_t*)(m_Memory.data() + address));
        return 0;
    }

    void WriteWord(uint16_t address, uint16_t v)
    {
        if (address < sizeof(m_Memory))
            *(uint16_t*)(m_Memory.data() + address) = _byteswap_ushort(v);
    }

    void WriteByte(uint16_t address, uint8_t v)
    {
        if (address < sizeof(m_Memory))
            m_Memory[address] = v;
    }

private:
    constexpr static std::array<uint8_t, 5 * 16> s_CharSprites = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    struct {
        uint8_t  v[16]; // V[0-15]
        uint8_t  dt;    // Delay timer
        uint8_t  st;    // Sound timer
        uint16_t i;     // I register
        uint16_t ip;    // Instruction pointer
        uint8_t  sp;     // Stack pointer
        uint16_t stack[16];
    } m_Registers;

    std::array<uint8_t, MemorySize> m_Memory;
    std::array<uint8_t, DisplayBitmapSize> m_DisplayBitmap;
    std::array<uint8_t, DisplayWidth * DisplayHeight * 4> m_DisplayBuffer;
    bool m_DisplayDirty;
    bool m_WaitingForKey;
    uint8_t  m_KeyDst;
    uint32_t m_KeyStates;

    bool GetPixel(int x, int y)
    {
        int pixel = (y * DisplayWidth) + x;
        int bit = 1 << (pixel % 8);
        int byte = pixel >> 3;

        return (m_DisplayBitmap[byte] & bit) != 0;
    }

    bool XorPixel(int x, int y, bool v)
    {
        int  pixel = (y * DisplayWidth) + x;
        int  bit = 1 << (pixel % 8);
        int  byte = pixel >> 3;
        bool ret = (GetPixel(x, y) ^ v) == 0;

        if (v)
            m_DisplayBitmap[byte] ^= bit;

        return ret;
    }

    bool DrawSprite(int x, int y, int address, int length);
};