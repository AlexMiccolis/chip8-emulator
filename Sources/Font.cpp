#include "Font.h"

Font::Font(SDL_Renderer* renderer, const std::string& path, int ptSize)
    : m_Renderer(renderer), m_Path(path), m_PointSize(ptSize), m_Font(nullptr),
    m_TextCache(1024)
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

Font::FontTexture::~FontTexture()
{
    if (texture != nullptr)
        SDL_DestroyTexture(texture);
}

int Font::GetHeight() const
{
    return TTF_FontHeight(m_Font);
}

void Font::DrawText(const std::string& text, int x, int y, const SDL_Color& fg, const SDL_Color& bg)
{
    SDL_Texture* texture = nullptr;
    SDL_Rect rect { };
    int extent = 0;
    int count = 0;

    if (m_TextCache.Contains(text))
    {
        texture = m_TextCache.Get(text).texture;
    }
    else
    {
        SDL_Surface* surface = TTF_RenderText_Shaded(m_Font, text.c_str(), fg, bg);
        texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
        SDL_FreeSurface(surface);

        m_TextCache.Put(text, { texture });
    }
    
    TTF_MeasureUTF8(m_Font, text.c_str(), 1024, &extent, &count);

    rect.x = x;
    rect.y = y;
    rect.w = extent;
    rect.h = TTF_FontHeight(m_Font);

    SDL_RenderCopy(m_Renderer, texture, nullptr, &rect);
}
