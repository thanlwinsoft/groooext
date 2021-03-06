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

#ifndef graphiteooo_hxx
#define graphiteooo_hxx

#include "com/sun/star/resource/XStringResourceWithLocation.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/chart2/XChartDocument.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

namespace org { namespace sil { namespace graphite {
    extern const char * SAL_DISABLE_GRAPHITE;
    extern const char * SAL_GRAPHITE_CACHE_SIZE;

	void printPropertyNames(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > propSet);

    void printServiceNames(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xInterface);

    ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceWithLocation> getResource(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context,
                    ::rtl::OUString basename, ::com::sun::star::lang::Locale locale);
    ::rtl::OUString getResourceString(
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver> resource,
                                      const char * resourceId);

	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getTextPropertiesFromModel(
		::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel,
		::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSelection);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getChartComponentProperties(
        ::com::sun::star::uno::Reference < ::com::sun::star::chart2::XChartDocument> & xChartDocument,
        ::rtl::OUString selectionId);

	sal_Bool isGraphiteEnabled();
}}}

#endif

