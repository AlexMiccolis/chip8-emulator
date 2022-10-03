#include <iostream>
#include <fstream>
#include <SDL.h>
#include "Instruction.h"
#include "Disassembler.h"
#include "Assembler.h"
#include "Core.h"

class Application
{
public:
    Application(const std::vector<uint8_t>& program)
        : m_Window(nullptr), m_Renderer(nullptr),
        m_DisplayTexture(nullptr),
        m_DisplayRect{ }, m_RegistersRect{ }, m_MemoryRect{ }
    {
        m_Window = SDL_CreateWindow("CHIP-8 Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600, 0);

        m_Renderer = SDL_CreateRenderer(m_Window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        m_DisplayTexture = SDL_CreateTexture(m_Renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            Core::DisplayWidth,
            Core::DisplayHeight);

        UpdateRectangles(800, 600);

        m_Core.LoadData(program, 0x200);
    }

    ~Application()
    {
        SDL_DestroyTexture(m_DisplayTexture);
        SDL_DestroyRenderer(m_Renderer);
        SDL_DestroyWindow(m_Window);
    }

    void Run()
    {
        SDL_Event event;
        bool quitting = false;

        uint64_t frequency = SDL_GetPerformanceFrequency();
        uint64_t start = SDL_GetPerformanceCounter();
        double deltaTime = 0.016;

        double targetSpeed = 5000;
        double targetCount = 0;
        double delaySpeed = 60;
        double delayCount = 0;

        while (!quitting)
        {
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    quitting = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_UP)
                        m_Core.KeyDown(2);
                    else if (event.key.keysym.sym == SDLK_DOWN)
                        m_Core.KeyDown(8);
                    else if (event.key.keysym.sym == SDLK_LEFT)
                        m_Core.KeyDown(4);
                    else if (event.key.keysym.sym == SDLK_RIGHT)
                        m_Core.KeyDown(6);
                    else if (event.key.keysym.sym == SDLK_SPACE)
                        m_Core.KeyDown(5);
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_UP)
                        m_Core.KeyUp(2);
                    else if (event.key.keysym.sym == SDLK_DOWN)
                        m_Core.KeyUp(8);
                    else if (event.key.keysym.sym == SDLK_LEFT)
                        m_Core.KeyUp(4);
                    else if (event.key.keysym.sym == SDLK_RIGHT)
                        m_Core.KeyUp(6);
                    else if (event.key.keysym.sym == SDLK_SPACE)
                        m_Core.KeyUp(5);
                    break;
                }
            }

            const double target = (1 / targetSpeed);
            const double delay = (1 / delaySpeed);

            targetCount += deltaTime;
            delayCount += deltaTime;
            if (targetCount >= target)
            {
                int cycles = static_cast<int>(std::round(targetCount / target));
                for (int i = 0; !m_Core.WaitingForKey() && (i < cycles); i++)
                    m_Core.DoCycle();
                targetCount = 0;
            }

            if (delayCount >= delay)
            {
                int counts = static_cast<int>(std::round(delayCount / delay));
                for (int i = 0; i < counts; i++)
                    m_Core.DecrementDT();
                delayCount = 0;
            }

            if (m_Core.UpdateDisplay())
            {
                SDL_UpdateTexture(m_DisplayTexture,
                    nullptr,
                    m_Core.GetDisplayBuffer().data(),
                    Core::DisplayWidth * 4);
            }

            DoFrame();

            uint64_t count = SDL_GetPerformanceCounter();
            deltaTime = (double)(count - start) / (double)frequency;
            start = count;
        }
    }

    void UpdateRectangles(int displayWidth, int displayHeight)
    {
        m_DisplayRect.w = 8 * 64;
        m_DisplayRect.h = 8 * 32;
        m_DisplayRect.x = 16;
        m_DisplayRect.y = 16;

        m_RegistersRect.x = m_DisplayRect.x + m_DisplayRect.w + 16;
        m_RegistersRect.y = 16;
        m_RegistersRect.w = (displayWidth - m_RegistersRect.x) - 16;
        m_RegistersRect.h = (displayHeight - 32);

        m_MemoryRect.x = 16;
        m_MemoryRect.y = m_DisplayRect.y + m_DisplayRect.h + 16;
        m_MemoryRect.w = m_DisplayRect.w - 4;
        m_MemoryRect.h = (displayHeight - m_DisplayRect.h) - 48;
    }

    void DrawShadedBox(const SDL_Rect& rect)
    {
        SDL_Rect darkBorder = {
            rect.x - 4,
            rect.y - 4,
            rect.w + 8,
            rect.h + 8
        };
        SDL_Rect lightBorder = {
            rect.x,
            rect.y,
            rect.w + 4,
            rect.h + 4
        };

        SDL_SetRenderDrawColor(m_Renderer, 32, 42, 58, 255);
        SDL_RenderFillRect(m_Renderer, &darkBorder);

        SDL_SetRenderDrawColor(m_Renderer, 40, 50, 65, 255);
        SDL_RenderFillRect(m_Renderer, &lightBorder);

        SDL_SetRenderDrawColor(m_Renderer, 45, 55, 70, 255);
        SDL_RenderFillRect(m_Renderer, &rect);
    }

    void DoFrame()
    {
        SDL_SetRenderDrawColor(m_Renderer, 30, 40, 55, 255);
        SDL_RenderClear(m_Renderer);

        //DrawShadedBox(m_DisplayRect);
        DrawShadedBox(m_RegistersRect);
        DrawShadedBox(m_MemoryRect);

        SDL_RenderCopy(m_Renderer, m_DisplayTexture, nullptr, &m_DisplayRect);
        SDL_RenderPresent(m_Renderer);
    }

private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;
    SDL_Texture* m_DisplayTexture;
    SDL_Rect      m_DisplayRect;
    SDL_Rect      m_RegistersRect;
    SDL_Rect      m_MemoryRect;

    Core m_Core;
};

int main(int argc, char** argv)
{
    std::vector<uint8_t> program;

#if 0
    const std::string code = R"(
       LD  V1, #4
    A: LD  V0, K
       CLS
       LD  F,  V0
       DRW V1, V1, #5
       JP  A
    )";

    if (!Assemble(program, code))
    {
        std::cout << "Assembly failed!" << std::endl;
        return 1;
    }

    std::cout << "Assembly source: \n" << code << std::endl;
#else
    std::ifstream input("Roms/Trip8 Demo (2008) [Revival Studios].ch8", std::ios::binary | std::ios::in);
    if (!input.is_open())
        return 1;

    input.seekg(0, std::ios::end);
    size_t size = input.tellg();
    program.resize(size);
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(program.data()), size);
#endif

    std::cout << Disassemble(program.data(), program.size(), 0x200) << std::endl;

    std::ofstream output("out.bin");
    output.write(reinterpret_cast<const char*>(program.data()), program.size());

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        puts("Failed to initialize SDL2!");
        return 1;
    }

    atexit(SDL_Quit);

    Application application(program);
    application.Run();

    return 0;
}