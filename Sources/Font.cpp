#include "Font.h"

Font::Font(SDL_Renderer* renderer, const std::string& path, int ptSize)
    : m_Renderer(renderer), m_Path(path), m_PointSize(ptSize), m_Font(nullptr)
{
    m_Font = TTF_OpenFont(m_Path.c_str(), ptSize);
    if (m_Font == nullptr)
        return;
}

Font::~Font()
{
    if (m_Font != nullptr)
        TTF_CloseFont(m_Font);
}

Font::TextBuffer::~TextBuffer()
{
    SDL_DestroyTexture(texture);
}

int Font::GetHeight() const
{
    return TTF_FontHeight(m_Font);
}

std::unique_ptr<Font::TextBuffer> Font::DrawText(const std::string& text, const SDL_Color& fg, const SDL_Color& bg)
{
    SDL_Surface* surface = TTF_RenderText_Shaded(m_Font, text.c_str(), fg, bg);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    SDL_Rect rect { };
    SDL_FreeSurface(surface);

    int extent = 0;
    int count = 0;
    TTF_MeasureUTF8(m_Font, text.c_str(), 1024, &extent, &count);

    rect.w = extent;
    rect.h = TTF_FontHeight(m_Font);

    return std::make_unique<Font::TextBuffer>(rect, texture);
}
