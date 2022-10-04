#pragma once

#include <memory>
#include <string>
#include <deque>
#include <unordered_map>
#include <SDL.h>
#include <SDL_ttf.h>

#include "LRUCache.h"

class Font
{
public:
    Font(SDL_Renderer* renderer, const std::string& path, int ptSize);
    ~Font();

    int GetHeight() const;
    void DrawText(const std::string& text, int x, int y, const SDL_Color& fg, const SDL_Color& bg);
private:
    struct FontTexture
    {
        FontTexture(SDL_Texture* _texture)
            : texture(_texture) { }
        FontTexture(FontTexture&& f) noexcept
            : texture(f.texture)
        {
            f.texture = nullptr;
        }
        ~FontTexture();
        SDL_Texture* texture;
    };

    SDL_Renderer* m_Renderer;
    const std::string m_Path;
    const int m_PointSize;
    TTF_Font* m_Font;

    /* This sucks in theory but works well in practice */
    LRUCache<std::string, FontTexture> m_TextCache;
};
