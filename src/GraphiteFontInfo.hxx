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

namespace org { namespace sil { namespace graphite {

class GraphiteFontInfo
{
    public:
        GraphiteFontInfo() {};
        virtual ~GraphiteFontInfo() {};
        virtual sal_Bool isGraphiteFont(const ::rtl::OUString & fontName) = 0;
        virtual gr::Font * loadFont(const ::rtl::OUString & fontName) = 0;
        static GraphiteFontInfo & getFontInfo();
    private:
        static GraphiteFontInfo * sInstance;
};

}}}


#endif // GRAPHITEFONTINFO_H
