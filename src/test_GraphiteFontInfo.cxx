/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2010 ThanLwinSoft.org & SIL International
 *
 * This file is part of the Graphite extension for OpenOffice.org (GraphiteOOo).
 *
 * The GraphiteOOo Extension is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * The GraphiteOOo Extension is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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