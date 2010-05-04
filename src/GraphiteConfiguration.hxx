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

#ifndef GRAPHITECONFIGURATION_HXX
#define GRAPHITECONFIGURATION_HXX

#ifdef SAL_UNX
#include <sal/types.h>
#endif
#include <sal/config.h>
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
