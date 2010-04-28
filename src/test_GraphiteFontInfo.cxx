#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <graphite/GrClient.h>
#include <graphite/Font.h>

#include "groooDebug.hxx"
#include "GraphiteFontInfo.hxx"

int testIsGraphite(const char * fontName, bool expected)
{
    ::rtl::OUString ouFontName(fontName, strlen(fontName), RTL_TEXTENCODING_UTF8);
    org::sil::graphite::GraphiteFontInfo & fontInfo = org::sil::graphite::GraphiteFontInfo::getFontInfo();
    bool isGraphite = fontInfo.isGraphiteFont(ouFontName);
    if (isGraphite != expected)
        org::sil::graphite::logMsg("Font %s was not detected correctly %d not %d\n", fontName, isGraphite, expected);
	if (isGraphite)
	{
		gr::Font * font = fontInfo.loadFont(ouFontName);
		std::pair<gr::FeatureIterator,gr::FeatureIterator> features = font->getFeatures();
		int featureCount = features.second - features.first;
		org::sil::graphite::logMsg("Font %s has %d features\n", fontName, featureCount);
		fontInfo.unloadFont(font);
		if (featureCount <= 0)
			return 2; // unexpected number of features
	}
    return (isGraphite == expected)? 0 : 1;
}

int main(int argc, char ** argv)
{
    int status = 0;
    char name[1024];
#ifdef _MSC_VER
	_snprintf_s(name, 1024, 1024, "Lucida Sans");
#else
    snprintf(name, 1024, "Lucida Sans");
#endif
    status |= testIsGraphite(name, false);
    status |= testIsGraphite("Lucida Sans", false);
    status |= testIsGraphite("Bitstream Charter", false);
    status |= testIsGraphite("Doulos SIL", true);
	status |= testIsGraphite("Charis SIL", true);
    status |= testIsGraphite("Padauk", true);
    status |= testIsGraphite("NonExistant", false);
    // ask again to check cache
    status |= testIsGraphite("Lucida Sans", false);
    status |= testIsGraphite("Doulos SIL", true);
    org::sil::graphite::logMsg("Status %d\n", status);

	org::sil::graphite::GraphiteFontInfo::dispose();
}