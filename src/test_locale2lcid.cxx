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
#include "locale2lcid.hxx"

namespace osg = org::sil::graphite;

bool test(::rtl::OUString lang, ::rtl::OUString country, ::rtl::OUString region, unsigned short expectedLcid, bool exact)
{
    static const osg::Locale2Lang locale2lcid;
    unsigned short lcid = locale2lcid.getMsId(lang, country, region);
    // check reverse
    const osg::IsoLangEntry * entry = locale2lcid.findEntryById(lcid);
    if (lcid == expectedLcid && entry &&
        ((exact == false) ||
         (lang.compareToAscii(entry->maLangStr) == 0 &&
          country.compareToAscii(entry->maCountry) == 0)))
    {
        return true;
    }
    ::rtl::OString aLang;
    ::rtl::OString aCountry;
    lang.convertToString(&aLang, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    country.convertToString(&aCountry, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    printf("%s-%s expected %x got %x reverse %x\n", aLang.getStr(), aCountry.getStr(), expectedLcid, lcid,
           entry? entry->mnLang : 0);
    return false;
}

int main(int argc, char ** argv)
{
    
    ::rtl::OUString lang = ::rtl::OUString::createFromAscii("en");
    ::rtl::OUString country = ::rtl::OUString::createFromAscii("GB");
    ::rtl::OUString region;
    // en-GB
    if (!test(lang, country, region, 0x809, true))
    {
        return 1;
    }
    // en-MM
    country = ::rtl::OUString::createFromAscii("MM");
    if (!test(lang, country, region, 0x409, false))
        return 2;
    // my-MM
    lang = ::rtl::OUString::createFromAscii("my");
    if (!test(lang, country, region, 0x455, true))
        return 3;
    // ar-MM
    lang = ::rtl::OUString::createFromAscii("ar");
    if (!test(lang, country, region, 0x401, false))
        return 4;
    // ar-SQ
    country = ::rtl::OUString::createFromAscii("SA");
    if (!test(lang, country, region, 0x401, true))
        return 4;
    // ar-QA
    country = ::rtl::OUString::createFromAscii("QA");
    if (!test(lang, country, region, 0x4001, true))
        return 5;
    // fil-QA
    lang = ::rtl::OUString::createFromAscii("fil");
    if (!test(lang, country, region, 0x464, false))
        return 6;
    // fil-PH
    country = ::rtl::OUString::createFromAscii("PH");
    if (!test(lang, country, region, 0x464, true))
        return 7;
    // es-US
    // check last entry
    lang = ::rtl::OUString::createFromAscii("es");
    country = ::rtl::OUString::createFromAscii("US");
    if (!test(lang, country, region, 0x540A, true))
        return 8;

    // language without entry
    lang = ::rtl::OUString::createFromAscii("ksw");
    country = ::rtl::OUString::createFromAscii("MM");
    if (!test(lang, country, region, 0x409, false))
        return 9;
    
    return 0;
}