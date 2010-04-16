#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "GraphiteFontInfo.hxx"

int testIsGraphite(const char * fontName, bool expected)
{
    ::rtl::OUString ouFontName(fontName, strlen(fontName), RTL_TEXTENCODING_UTF8);
    org::sil::graphite::GraphiteFontInfo & fontInfo = org::sil::graphite::GraphiteFontInfo::getFontInfo();
    bool isGraphite = fontInfo.isGraphiteFont(ouFontName);
    if (isGraphite != expected)
        fprintf(stderr, "Font %s was not detected correctly %d not %d\n", fontName, isGraphite, expected);
    return (isGraphite == expected)? 0 : 1;
}

int main(int argc, char ** argv)
{
    int status = 0;
    char name[1024];
    snprintf(name, 1024, "Lucida Sans");
    status |= testIsGraphite(name, false);
    status |= testIsGraphite("Lucida Sans", false);
    status |= testIsGraphite("Bitstream Charter", false);
    status |= testIsGraphite("Doulos SIL", true);
    status |= testIsGraphite("Padauk", true);
    status |= testIsGraphite("NonExistant", false);
    // ask again to check cache
    status |= testIsGraphite("Lucida Sans", false);
    status |= testIsGraphite("Doulos SIL", true);
    fprintf(stderr, "Status %d\n", status);
}