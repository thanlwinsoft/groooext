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

#include "groooDebug.hxx"
#include "GraphiteFontInfo.hxx"

#include <cstdio>
#include <string>
#include <map>
#include <cassert>

#include <graphite/FileFont.h>
#ifdef WIN32
#include <windows.h>
#include <WinFont.h>
#endif
#include "com/sun/star/uno/Exception.hpp"

// For Windows
#ifdef WIN32 // Is this always correct?
#define LITTLE_ENDIAN
#endif

namespace css = ::com::sun::star;

#ifdef SAL_UNX

//#include <ext/hash_map>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#endif

org::sil::graphite::GraphiteFontInfo * org::sil::graphite::GraphiteFontInfo::sInstance = NULL;

namespace org { namespace sil { namespace graphite {

const ::rtl::OUString GraphiteFontInfo::FEAT_PREFIX = ::rtl::OUString::createFromAscii(":");
const ::rtl::OUString GraphiteFontInfo::FEAT_SEPARATOR = ::rtl::OUString::createFromAscii("&");
const ::rtl::OUString GraphiteFontInfo::FEAT_ID_VALUE_SEPARATOR = ::rtl::OUString::createFromAscii("=");

union FeatUnion
{
    sal_uInt32 uNum;
    sal_Char label[5];
};

static bool isValidChar(char c)
{
    if (c == 0 || (c >= 0x20 && c < 0x7F))
        return true;
    return false;
}

::rtl::OUString GraphiteFontInfo::featId2OUString(sal_uInt32 id)
{
    FeatUnion featId;
    featId.uNum = id;
#ifdef LITTLE_ENDIAN
    featId.label[0] = (id >> 24) & 0xFF;
    featId.label[1] = (id >> 16) & 0xFF;
    featId.label[2] = (id >> 8) & 0xFF;
    featId.label[3] = (id) & 0xFF;
#endif
    if (isValidChar(featId.label[0]) && isValidChar(featId.label[1]) &&
        isValidChar(featId.label[2]) && isValidChar(featId.label[3]))
    {
        featId.label[4] = '\0';
        return ::rtl::OUString::createFromAscii(featId.label);
    }
    else  // treat it as an unsigned integer
    {
        char aInt[12]; // should be sufficient for any 32 bit integer even with sign
#ifdef _MSC_VER
		_snprintf_s(aInt, 12, 12, "%lu", id);
#else
        snprintf(aInt, 12, "%lu", id);
#endif
        return ::rtl::OUString::createFromAscii(aInt);
    }
}

::rtl::OUString GraphiteFontInfo::featSetting2OUString(sal_Int32 setting)
{
    char aInt[12]; // should be sufficient for any 32 bit integer even with sign
#ifdef _MSC_VER
	_snprintf_s(aInt, 12, 12, "%ld", setting);
#else
    snprintf(aInt, 12, "%ld", setting);
#endif
    return ::rtl::OUString::createFromAscii(aInt);
}

sal_uInt32 GraphiteFontInfo::ouString2FeatId(const ::rtl::OUString & idString) throw (css::lang::IllegalArgumentException)
{
    FeatUnion featId;
    if (idString.getLength() > 4) throw css::lang::IllegalArgumentException(idString, NULL, 0);
    if (idString[0] >= '0' && idString[0] <= '9')
    {
        // it the first character is a digit, assume it is an integer number
        ::rtl::OString aInt(12); // should be sufficient for any 32 bit integer even with sign
        idString.convertToString(&aInt, RTL_TEXTENCODING_UTF8, aInt.getLength());
        sal_uInt32 uintId = atoi(aInt.getStr());
        return uintId;
    }
    for (int i = 0; i < 4; i++)
    {
        if (idString.getLength() <= i)
        {
#ifdef LITTLE_ENDIAN
            featId.label[3-i] = '\0';
#else
#ifdef BIG_ENDIAN
            featId.label[i] = '\0';
#else
#error endianness not defined
#endif
#endif
            continue;
        }
        if (idString[i] > 127) throw css::lang::IllegalArgumentException(idString, NULL, 0);
#ifdef LITTLE_ENDIAN
        featId.label[3-i] = static_cast<char>(idString[i]);
#else
        featId.label[i] = static_cast<char>(idString[i]);
#endif
    }
    return featId.uNum;
}

sal_Int32 GraphiteFontInfo::ouString2FeatSetting(const ::rtl::OUString & settingString) throw (css::lang::IllegalArgumentException)
{
    sal_Int32 setting = 0;
    ::rtl::OString aInt(12); // should be sufficient for any 32 bit integer even with sign
    settingString.convertToString(&aInt, RTL_TEXTENCODING_UTF8, aInt.getLength());
    setting = atoi(aInt.getStr());
    return setting;
}

static const size_t MAX_FONT_NAME_SIZE = 128;

#ifdef SAL_UNX
class FTGraphiteFontInfo : public GraphiteFontInfo
{
    public:
        FTGraphiteFontInfo();
        virtual ~FTGraphiteFontInfo();
        virtual sal_Bool isGraphiteFont(const ::rtl::OUString & fontName);
        virtual gr::Font * loadFont(const ::rtl::OUString & fontName);
		virtual void unloadFont(gr::Font * font) { delete font; };
    private:
        std::map<std::string, std::string> mFontMap;
        FT_Library mLibrary;
};
#endif

#ifdef WIN32
class WinGraphiteFontInfo : public GraphiteFontInfo
{
	public:
        WinGraphiteFontInfo();
        virtual ~WinGraphiteFontInfo();
        virtual sal_Bool isGraphiteFont(const ::rtl::OUString & fontName);
        virtual gr::Font * loadFont(const ::rtl::OUString & fontName);
		virtual void unloadFont(gr::Font * font);
    private:
		std::map< ::rtl::OUString, gr::WinFont* > mFontMap;
		HDC mHdc;
		HFONT mhFontNew;
		HFONT mhFontOld;
};
#endif
}}}

org::sil::graphite::GraphiteFontInfo &
org::sil::graphite::GraphiteFontInfo::getFontInfo()
{
    if (sInstance) return *sInstance;
#ifdef SAL_UNX
    sInstance = new FTGraphiteFontInfo();
    return *sInstance;
#else
#ifdef SAL_W32
    sInstance = new WinGraphiteFontInfo();
	return *sInstance;
#else
#error Unsupported
#endif
#endif
}

void org::sil::graphite::GraphiteFontInfo::dispose()
{
	if (sInstance)
	{
		delete sInstance;
		sInstance = NULL;
	}
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
#ifdef GROOO_DEBUG
            logMsg("Failed to create gr::FileFont %s\n", asciiFontName.getStr());
#endif
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
#ifdef GROOO_DEBUG
            logMsg("Failed to init Fontconfig\n");
#endif
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
            logMsg("Failed to add to pattern\n");
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
             logMsg("Failed to match %s %d\n", asciiFontName.getStr(), fcResult);
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
                        logMsg("Font file %s %s\n", asciiFontName.getStr(), fontFileName);
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


#ifdef WIN32
	
org::sil::graphite::WinGraphiteFontInfo::WinGraphiteFontInfo()
: mHdc(NULL), mhFontNew(NULL), mhFontOld(NULL)
{

}

org::sil::graphite::WinGraphiteFontInfo::~WinGraphiteFontInfo()
{
	std::map< ::rtl::OUString, gr::WinFont* >::iterator i = mFontMap.begin();
	while (i != mFontMap.end())
	{
		if (i->second != NULL)
			delete i->second;
		++i;
	}
	mFontMap.clear();
}

LOGFONTW createLogFontFromName(const ::rtl::OUString & fontName)
{
	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	lf.lfItalic = false;
	lf.lfHeight = 12;
	lf.lfWidth = 0;
	lf.lfWeight = FW_DONTCARE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfUnderline = false;

	for (int i = 0; i < fontName.getLength() && i < LF_FACESIZE - 1; i++)
		lf.lfFaceName[i] = fontName[i];
	// name should already be null terminated from the memset above
	return lf;
}

sal_Bool org::sil::graphite::WinGraphiteFontInfo::isGraphiteFont(const ::rtl::OUString & fontName)
{
	if (mFontMap.find(fontName) != mFontMap.end())
		return (mFontMap.find(fontName)->second != NULL);

	LOGFONTW lf = createLogFontFromName(fontName);
	HFONT hFontNew = CreateFontIndirectW(&lf);
	// Using NULL means the DC is not associated with a window, which is fine for just
	// getting FeatureInfo
	if (!mHdc)
		mHdc = GetDC(NULL);

	HFONT hFontOld = (HFONT) SelectObject(mHdc, hFontNew);

	// test the font
	if (gr::WinFont::FontHasGraphiteTables(mHdc))
		mFontMap[fontName] = new gr::WinFont(mHdc);
	else
		mFontMap[fontName] = NULL;

	SelectObject(mHdc, hFontOld);
	DeleteObject(hFontNew);
	DeleteDC(mHdc);
	mHdc = NULL;

	return (mFontMap[fontName] != NULL);
}

gr::Font * org::sil::graphite::WinGraphiteFontInfo::loadFont(const ::rtl::OUString & fontName)
{
	gr::WinFont * winFont = NULL;
	if (mFontMap.find(fontName) != mFontMap.end())
	{
		winFont = mFontMap.find(fontName)->second;
		if (winFont == NULL) return NULL;
	}
	LOGFONTW lf = createLogFontFromName(fontName);
	// if these asserts fire, it probably means unloadFont wasn't called after the last loadFont
	assert(mhFontNew == NULL);
	assert(mhFontOld == NULL);
	mhFontNew = CreateFontIndirectW(&lf);
	if (!mHdc)
		mHdc = GetDC(NULL);
	mhFontOld = (HFONT) SelectObject(mHdc, mhFontNew);

	if (winFont == NULL)
		winFont = new gr::WinFont(mHdc);

	winFont->replaceDC(mHdc);

	return winFont;
}

void org::sil::graphite::WinGraphiteFontInfo::unloadFont(gr::Font * font)
{
	gr::WinFont * winFont = reinterpret_cast<gr::WinFont*>(font);
	winFont->restoreDC();
	SelectObject(mHdc, mhFontOld);
	DeleteObject(mhFontNew);
	DeleteDC(mHdc);
	mHdc = NULL;
	mhFontNew = NULL;
	mhFontOld = NULL;
}

#endif
