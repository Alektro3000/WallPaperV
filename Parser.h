#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <freetype/ftbitmap.h>
#include <string.h>
#include <bitset>
#include "MainApp.h"


struct Parser
{
    FT_Library  library;   /* handle to library     */
    FT_Face     face;      /* handle to face object */
public:
    Parser() {};
    bool binit = false;
public:

    void init()
    {    
        binit = true;
        auto error = FT_Init_FreeType(&library);

        error = FT_New_Face(library,
            "fonts/Rus.ttf",
            0,
            &face);
        int a = face->num_glyphs;

        FT_Set_Pixel_Sizes(
            face,   /* handle to face object */
            0,      /* pixel_width           */
            96);
        
        Draw();
    }
public:
    static const unsigned height = 96;
    std::vector<uint8_t> OutArray;
private:
    void Draw()
    {
        FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */
        int           pen_x, pen_y, n;
        OutArray.resize(TextWidth * 96, 0);

        const std::wstring txt = L"Встречая страх, создаём будущее";
        //const std::string txt = "Face the fear, build the future";

        //"글쎄요 축하합니다한국어를 아시나요  이것이 당신에게 무엇을 주는가 마침표 단지 흥미로워 보이기 위해 온라인 번역기를 통해 작성된 무의미한 텍스트";

        int numChars = txt.length();

        pen_x = 0;
        pen_y = 72;

        for (n = 0; n < numChars; n++)
        {
            auto error = FT_Load_Char(face, txt[n], FT_LOAD_RENDER);
            if (error)
                continue;  /* ignore errors */

            /* convert to an anti-aliased bitmap */
            error = FT_Render_Glyph(face->glyph, FT_Render_Mode_::FT_RENDER_MODE_MONO);
            if (error)
                continue;

            /* now, draw to our target surface */
            savebitmap(&slot->bitmap,
                pen_x + slot->bitmap_left,
                pen_y - slot->bitmap_top);

            /* increment pen position */
            pen_x += slot->advance.x >> 6;
            //pen_y += slot->advance.y >> 6; /* not useful for now */
        }
    }
    //856 * 96 
private:
    void savebitmap(FT_Bitmap* bitmap, int posx, int posy)
    {
        int off = TextWidth;
        if (posy + bitmap->rows > 95)
            std::cout << "toolarge";
        for (unsigned row = 0; row < bitmap->rows; ++row)
        {
            if(0 <= posy + row && posy + row < 96)
                memcpy(&OutArray[posx + (posy + row) * off], bitmap->buffer + row * (bitmap->pitch), bitmap->width);
        }
    }
};

