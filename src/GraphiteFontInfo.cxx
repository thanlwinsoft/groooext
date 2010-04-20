/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "GraphiteFontInfo.hxx"

#include <cstdio>
#include <string>
#include <map>
#include <cassert>
#include <graphite/FileFont.h>

#ifdef SAL_UNX

//#include <ext/hash_map>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#endif

org::sil::graphite::GraphiteFontInfo * org::sil::graphite::GraphiteFontInfo::sInstance = NULL;

namespace org { namespace sil { namespace graphite {

static const size_t MAX_FONT_NAME_SIZE = 128;

class FTGraphiteFontInfo : public GraphiteFontInfo
{
    public:
        FTGraphiteFontInfo();
        virtual ~FTGraphiteFontInfo();
        virtual sal_Bool isGraphiteFont(const ::rtl::OUString & fontName);
        virtual gr::Font * loadFont(const ::rtl::OUString & fontName);
    private:
        std::map<std::string, std::string> mFontMap;
        FT_Library mLibrary;
};

}}}

org::sil::graphite::GraphiteFontInfo &
org::sil::graphite::GraphiteFontInfo::getFontInfo()
{
    if (sInstance) return *sInstance;
#ifdef SAL_UNX
    sInstance = new FTGraphiteFontInfo();
    return *sInstance;
#else
#if SAL_W32
    
#else
#error Unsupported
#endif
#endif
}

#ifdef SAL_UNX
org::sil::graphite::FTGraphiteFontInfo::FTGraphiteFontInfo()
{
    FT_Error status = FT_Init_FreeType(&mLibrary);
    assert(status == 0);
    assert(mLibrary);
}

org::sil::graphite::FTGraphiteFontInfo::~FTGraphiteFontInfo()
{
    FT_Error status = FT_Done_FreeType(mLibrary);
    assert(status == 0);
    mLibrary = NULL;
}

gr::Font * org::sil::graphite::FTGraphiteFontInfo::loadFont(const ::rtl::OUString & fontName)
{
    ::rtl::OString asciiFontName(MAX_FONT_NAME_SIZE);
    fontName.convertToString(&asciiFontName, RTL_TEXTENCODING_UTF8, MAX_FONT_NAME_SIZE);
    if (mFontMap.find(asciiFontName.getStr()) != mFontMap.end())
    {
        try
        {
            // dpi isn't important since we only want to query the features
            gr::FileFont * fileFont = new gr::FileFont(mFontMap[asciiFontName.getStr()], 12.0f, 96, 96);
            return fileFont;
        }
        catch (...)
        {
            fprintf(stderr, "Failed to create gr::FileFont %s\n", asciiFontName.getStr());
        }
    }
    return NULL;
}

sal_Bool
org::sil::graphite::FTGraphiteFontInfo::isGraphiteFont(const ::rtl::OUString & fontName)
{
    sal_Bool isGraphite = sal_False;
    ::rtl::OString asciiFontName(MAX_FONT_NAME_SIZE);
    fontName.convertToString(&asciiFontName, RTL_TEXTENCODING_UTF8, MAX_FONT_NAME_SIZE);
    if (mFontMap.find(asciiFontName.getStr()) != mFontMap.end())
    {
        return (mFontMap[asciiFontName.getStr()].length() > 0);
    }
    if (mLibrary)
    {
        if (!FcInit())
        {
            fprintf(stderr, "Failed to init Fontconfig\n");
            return isGraphite;
        }
        FcConfig * fcConfig = FcConfigGetCurrent();
        FcPattern * fcPattern = FcPatternCreate();//FcNameParse(reinterpret_cast<const FcChar8*>(asciiFontName.getStr()));
        assert(fcPattern);
        FcValue fcFontNameValue;
        fcFontNameValue.type = FcTypeString;
        fcFontNameValue.u.s = reinterpret_cast<const FcChar8*>(asciiFontName.getStr());
        if (!FcPatternAdd(fcPattern, "family", fcFontNameValue, FcFalse))
        {
#ifdef GROOO_DEBUG
            fprintf(stderr, "Failed to add to pattern\n");
#endif
        }
        FcResult fcResult = FcResultMatch;
        FcDefaultSubstitute(fcPattern);
        FcPattern * fcMatchPattern = FcFontMatch(fcConfig, fcPattern, &fcResult);
        char * family = NULL;
        if (fcMatchPattern == NULL || fcResult != FcResultMatch ||
           ((FcPatternGetString(fcMatchPattern, "family", 0, reinterpret_cast<FcChar8**>(&family)) == 0) &&
            (strcasecmp(family,  asciiFontName.getStr()) != 0)))
        {
#ifdef GROOO_DEBUG
             fprintf(stderr, "Failed to match %s %d\n", asciiFontName.getStr(), fcResult);
#endif
        }
        else
        {
#ifdef GROOO_DEBUG
            //FcPatternPrint(fcMatchPattern);
#endif
            char * fontFileName = NULL;
            if (FcPatternGetString(fcMatchPattern, "file", 0, reinterpret_cast<FcChar8**>(&fontFileName)) == 0)
            {
                FT_Open_Args args;
                FT_Face face;
                FT_Error status = FT_New_Face(mLibrary, fontFileName, 0, &face);
                if (!status)
                {
                    FT_ULong length = 0;
                    // Check for Silf table
                    status = FT_Load_Sfnt_Table(face, 0x53696C66, 0, NULL, &length);
                    if (status == 0)
                    {
                        isGraphite = sal_True;
                        mFontMap[asciiFontName.getStr()] = fontFileName;
#ifdef GROOO_DEBUG
                        fprintf(stderr, "Font file %s %s\n", asciiFontName.getStr(), fontFileName);
#endif
                    }
                    else
                    {
                        mFontMap[asciiFontName.getStr()] = "";
                    }
                }
                FT_Done_Face(face);
            }
        }
        FcPatternDestroy(fcPattern);
        if (fcMatchPattern) FcPatternDestroy(fcMatchPattern);
        FcConfigDestroy (fcConfig);
    }
    return isGraphite;
}

#endif

