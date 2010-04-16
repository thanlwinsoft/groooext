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
#include <cassert>
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
