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
#include "groooDebug.hxx"
#include "GraphiteConfiguration.hxx"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"

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
}
