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

#include <cassert>
#include <cstring>
#include "groooDebug.hxx"
#include "GraphiteConfiguration.hxx"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/resource/XLocale.hpp"

org::sil::graphite::GraphiteConfiguration::GraphiteConfiguration(css::uno::Reference< css::uno::XComponentContext > const & context)
{
    assert(context.is());
    assert(context.get()->getServiceManager().is());
    css::uno::Reference< css::uno::XInterface > configInterface = 
        context.get()->getServiceManager().get()->createInstanceWithContext(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider")), context);
    mConfigurationProvider.set(configInterface, css::uno::UNO_QUERY);
    css::uno::Sequence< css::uno::Any > args(1);
    css::uno::Any nodePathValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.sil.graphite.GraphiteOptions/Leaves")));
    css::beans::PropertyValue nodeProperty(
         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")), 0, nodePathValue,
         css::beans::PropertyState_DIRECT_VALUE);
    args[0] = css::uno::Any(nodeProperty);
    css::uno::Reference< css::uno::XInterface > updateInterface =
        mConfigurationProvider.get()->createInstanceWithArguments(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationUpdateAccess")), args);
    mNameAccess.set(updateInterface, css::uno::UNO_QUERY);

    css::uno::Any linguisticNodePathValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Linguistic/General")));
    css::beans::PropertyValue localeNodeProperty(
         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")), 0, linguisticNodePathValue,
         css::beans::PropertyState_DIRECT_VALUE);
    args[0] = css::uno::Any(localeNodeProperty);
    css::uno::Reference< css::uno::XInterface > configAccess =
        mConfigurationProvider.get()->createInstanceWithArguments(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")), args);

    // TODO find how to get XLocale interface access, since, this way doesn't work
    css::uno::Reference< css::resource::XLocale > xLocale(configAccess, css::uno::UNO_QUERY);
    if (xLocale.is())
    {
        // not default locale
        mLocale = xLocale->getDefault();
#ifdef GROOO_DEBUG
        ::rtl::OString aLang;
        ::rtl::OString aCountry;
        mLocale.Language.convertToString(&aLang, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        mLocale.Country.convertToString(&aCountry, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        logMsg("have default locale %s-%s", aLang.getStr(), aCountry.getStr());
#endif
    }
    css::uno::Reference< css::container::XNameAccess> xLinguisticNameAccess(configAccess, css::uno::UNO_QUERY);
    css::uno::Any localeName = xLinguisticNameAccess->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UILocale")));
    ::rtl::OUString localeFullName = localeName.get< ::rtl::OUString>();
    ::rtl::OUString langCode;
    ::rtl::OUString countryCode;
    ::rtl::OUString subCode;
    sal_Int32 dashIndex = localeFullName.indexOf('-', 0);
    if (dashIndex > -1)
    {
        langCode = ::rtl::OUString(localeFullName.getStr(), dashIndex);
        sal_Int32 secondDash = localeFullName.indexOf('-', dashIndex + 1);
        if (secondDash > -1)
        {
            countryCode = ::rtl::OUString(localeFullName.getStr() + dashIndex + 1,
                                          secondDash - dashIndex - 1);
            subCode = ::rtl::OUString(localeFullName.getStr() + secondDash + 1,
                                      localeFullName.getLength() - secondDash - 1);
        }
        else
        {
            countryCode = ::rtl::OUString(localeFullName.getStr() + dashIndex + 1,
                                          localeFullName.getLength() - dashIndex - 1);
        }
    }
    else if (localeFullName.getLength() == 0)
    {
        // TODO there ought to be a way to get this from OOo, but I haven't found it yet
        // default to English
        langCode = ::rtl::OUString::createFromAscii("en");
#ifdef SAL_UNX
        // make another guess based on the Locale environment variable
        char tempLang[6] = { '\0','\0','\0','\0','\0','\0' };
        const char * pLang = getenv("LANG");
        if (!pLang)
        {
            pLang = getenv("LC_MESSAGES");
        }
        if (pLang)
        {
            size_t langLen = strlen(pLang);
            const char * dotUtf8 = strstr(pLang, ".utf8");
            if (dotUtf8)
            {
                langLen = dotUtf8 - pLang;
            }
            if (langLen == 2)
            {
                strncpy(tempLang, pLang, 2);
                // language only
                langCode = ::rtl::OUString::createFromAscii(tempLang);
            }
            else if (langLen == 5 && pLang[2] == '_')
            {
                // OOo uses - not _
                strncpy(tempLang, pLang, 2);
                langCode = ::rtl::OUString::createFromAscii(tempLang);
                strncpy(tempLang, pLang + 3, 2);
                countryCode = ::rtl::OUString::createFromAscii(tempLang);
            }
        }
#else
        countryCode = ::rtl::OUString::createFromAscii("US");
#endif
    }
    else
    {
        langCode = localeFullName;
    }
    mLocale = css::lang::Locale(langCode, countryCode, subCode);
}
