#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <freetype/ftbitmap.h>
#include <string.h>
#include "MainApp.h"
#include <bitset>


class Parser
{
    FT_Library  library;   /* handle to library     */
    FT_Face     face;      /* handle to face object */
    

    std::string txt = "Face the fear, build the future";//"글쎄요 축하합니다한국어를 아시나요  이것이 당신에게 무엇을 주는가 마침표 단지 흥미로워 보이기 위해 온라인 번역기를 통해 작성된 무의미한 텍스트";
public:

    void init()
    {

        auto error = FT_Init_FreeType(&library);

        error = FT_New_Face(library,
            "fonts/Roman.ttf",
            0,
            &face);
        if (error == FT_Err_Unknown_File_Format)
        {
            std::cerr << "s";
        }
        int a = face->num_glyphs;

        FT_Set_Pixel_Sizes(
            face,   /* handle to face object */
            0,      /* pixel_width           */
            96);
        FT_UInt glyph_index = FT_Get_Char_Index(face, 'c');
        FT_Int load_flags = 0;

        error = FT_Load_Glyph(
            face,          /* handle to face object */
            glyph_index,   /* glyph index           */
            load_flags);
        
        error = FT_Render_Glyph(face->glyph,   /* glyph slot  */
            FT_RENDER_MODE_MONO); /* render mode */
        face->glyph->bitmap;

    }

    void Draw()
    {
        FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */
        int           pen_x, pen_y, n;
        
        int numChars = txt.length();

        pen_x = 300;
        pen_y = 200;

        for (n = 0; n < numChars; n++)
        {
            auto error = FT_Load_Char(face, txt[n], FT_LOAD_RENDER);
            if (error)
                continue;  /* ignore errors */

            /* convert to an anti-aliased bitmap */
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
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
    const unsigned width = 856;
    const unsigned height = 96;
    //856 * 96 / 32
    uint32_t OutBitmap[2568];
    void savebitmap(FT_Bitmap* bitmap, int posx, int posy)
    {
        int off = width/32;

        for (int row = 0; row < 96; ++row)
        {
            memcpy((void*)OutBitmap[row * off], bitmap->buffer + posx + row * bitmap->pitch, bitmap->width);
        }
    }
};

