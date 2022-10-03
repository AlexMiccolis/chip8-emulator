#pragma once

#include <memory>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class Font
{
public:
    Font(SDL_Renderer* renderer, const std::string& path, int ptSize);
    ~Font();

    struct TextBuffer
    {
        TextBuffer(const SDL_Rect& _rect, SDL_Texture* _texture) 
            : rect(_rect), texture(_texture) { }
        ~TextBuffer();

        SDL_Rect rect;
        SDL_Texture* texture;
    };

    int GetHeight() const;
    std::unique_ptr<TextBuffer> DrawText(const std::string& text, const SDL_Color& fg, const SDL_Color& bg);
private:
    SDL_Renderer* m_Renderer;
    const std::string m_Path;
    const int m_PointSize;
    TTF_Font* m_Font;
};