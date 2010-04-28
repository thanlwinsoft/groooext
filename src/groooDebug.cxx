/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2010 ThanLwinSoft.org
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

#include <stdarg.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#ifdef WIN32
#include <windows.h>
#endif

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"

#include "groooDebug.hxx"

namespace css = ::com::sun::star;
namespace osg = ::org::sil::graphite;

namespace org { namespace sil { namespace graphite {

	void logMsg(const char * msg, ...)
	{
#ifdef GROOO_DEBUG
		va_list args;
		va_start(args, msg);
#ifdef _MSC_VER
		char buffer[1024];
		if (vsnprintf_s(buffer, 1024, 1024, msg, args) > 0)
			OutputDebugStringA(buffer);
#else
		vfprintf(stderr, format, args);
#endif
		va_end(args);
#endif
	}

}}}

void org::sil::graphite::printPropertyNames(css::uno::Reference<css::beans::XPropertySet > propSet)
{
    css::uno::Reference< css::beans::XPropertySetInfo>xPropSetInfo(propSet.get()->getPropertySetInfo());
    css::uno::Sequence< css::beans::Property> properties = xPropSetInfo.get()->getProperties();
    for (int i = 0; i < properties.getLength(); i++)
    {
        ::rtl::OString propName;
        ::rtl::OString propValue;
        properties[i].Name.convertToString(&propName, RTL_TEXTENCODING_UTF8, 128);
        try
        {
            ::css::uno::Any aValue = propSet.get()->getPropertyValue(properties[i].Name);
            if (aValue.hasValue() && aValue.has< ::rtl::OUString>())
            {
                ::rtl::OUString value = aValue.get< ::rtl::OUString>();
                value.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
            }
            else
            {
                propValue = "?";
            }
        }
        catch (::com::sun::star::lang::WrappedTargetException we)
        {
            css::uno::Exception e = we.TargetException.get<css::uno::Exception>();
            e.Message.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
        }
        catch (::com::sun::star::uno::Exception e)
        {
            e.Message.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
        }
#ifdef GROOO_DEBUG
        logMsg("Property name:%s value:%s\n", propName.getStr(), propValue.getStr());
#endif
    }
}
