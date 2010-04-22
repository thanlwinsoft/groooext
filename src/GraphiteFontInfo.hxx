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

#ifndef GRAPHITEFONTINFO_H
#define GRAPHITEFONTINFO_H

#include "sal/typesizes.h"
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
        static ::rtl::OUString featId2OUString(sal_uInt32);
        static ::rtl::OUString featSetting2OUString(sal_Int32);
        static sal_uInt32 ouString2FeatId(const ::rtl::OUString & idString)  throw (css::lang::IllegalArgumentException);
        static sal_Int32 ouString2FeatSetting(const ::rtl::OUString & settingString) throw (css::lang::IllegalArgumentException);
        static GraphiteFontInfo & getFontInfo();

        static const ::rtl::OUString FEAT_PREFIX;
        static const ::rtl::OUString FEAT_SEPARATOR;
        static const ::rtl::OUString FEAT_ID_VALUE_SEPARATOR;
    private:
        static GraphiteFontInfo * sInstance;
};

}}}


#endif // GRAPHITEFONTINFO_H
