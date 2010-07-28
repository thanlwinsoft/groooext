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


#ifndef GRAPHITEFONTINFO_H
#define GRAPHITEFONTINFO_H

#include "sal/types.h"
#include "sal/config.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include <graphite/GrClient.h>
#include <graphite/Font.h>

#include "com/sun/star/lang/IllegalArgumentException.hpp"

namespace org { namespace sil { namespace graphite {

namespace css = ::com::sun::star;

class GraphiteFontInfo
{
    public:
        GraphiteFontInfo() {};
        virtual ~GraphiteFontInfo() {};
        virtual sal_Bool isGraphiteFont(const ::rtl::OUString & fontName) = 0;
        virtual gr::Font * loadFont(const ::rtl::OUString & fontName) = 0;
        virtual void unloadFont(gr::Font * font) = 0;
        static ::rtl::OUString featId2OUString(sal_uInt32);
        static ::rtl::OUString featSetting2OUString(sal_Int32);
        static sal_uInt32 ouString2FeatId(const ::rtl::OUString & idString)  throw (css::lang::IllegalArgumentException);
        static sal_Int32 ouString2FeatSetting(const ::rtl::OUString & settingString) throw (css::lang::IllegalArgumentException);
        static GraphiteFontInfo & getFontInfo();
        static void dispose();

        static const ::rtl::OUString FEAT_PREFIX;
        static const ::rtl::OUString FEAT_SEPARATOR;
        static const ::rtl::OUString FEAT_ID_VALUE_SEPARATOR;
    private:
        static GraphiteFontInfo * sInstance;
};

}}}


#endif // GRAPHITEFONTINFO_H
