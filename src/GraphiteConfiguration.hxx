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

#ifndef GRAPHITECONFIGURATION_HXX
#define GRAPHITECONFIGURATION_HXX

#include "sal/typesizes.h"
#include "sal/config.h"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"

namespace org { namespace sil { namespace graphite {

    namespace css = ::com::sun::star;

    class GraphiteConfiguration
    {
        public:
            GraphiteConfiguration(css::uno::Reference< css::uno::XComponentContext > const & context);
            css::uno::Reference< css::container::XNameAccess> & nameAccess() { return mNameAccess; };
        private:
            css::uno::Reference< css::lang::XMultiServiceFactory > mConfigurationProvider;
            css::uno::Reference< css::container::XNameAccess> mNameAccess;
    };

}}}
#endif // GRAPHITECONFIGURATION_HXX
