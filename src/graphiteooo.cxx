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

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/deployment/PackageInformationProvider.hpp"
#include "com/sun/star/resource/StringResourceWithLocation.hpp"
#include "com/sun/star/text/XTextViewCursor.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"
#include "com/sun/star/sheet/XCellRangeReferrer.hpp"
#include "com/sun/star/sheet/XSheetCellRange.hpp"
#include "com/sun/star/table/XCellRange.hpp"
#include "com/sun/star/table/XCellCursor.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/Any.hxx"

#ifdef WIN32
#include <windows.h>
#endif

#include "groooDebug.hxx"
#include "graphiteooo.hxx"
#include "GraphiteAddOn.hxx"

#include "DialogEventHandler.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "SetupContextMenu.hxx"

const char * ::org::sil::graphite::SAL_DISABLE_GRAPHITE = "SAL_DISABLE_GRAPHITE";

namespace css = ::com::sun::star;
namespace osg = ::org::sil::graphite;

void osg::printPropertyNames(css::uno::Reference<css::beans::XPropertySet > propSet)
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

void osg::printServiceNames(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> interface)
{
    css::uno::Reference<css::lang::XServiceInfo> xServiceInfo(interface, css::uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        ::rtl::OString aServiceName;
        css::uno::Sequence< ::rtl::OUString> serviceNames =
            xServiceInfo->getSupportedServiceNames();
        for (int i = 0; i < serviceNames.getLength(); i++)
        {
            serviceNames[i].convertToString(&aServiceName, RTL_TEXTENCODING_UTF8, 128);
            osg::logMsg("service: %s\n", aServiceName.getStr());
        }
    }
}

css::uno::Reference< css::beans::XPropertySet> 
osg::getTextPropertiesFromModel(css::uno::Reference< css::frame::XModel > xModel,
								css::uno::Reference<css::view::XSelectionSupplier> xSelection)
{
	css::uno::Reference< css::beans::XPropertySet> xTextProperties; // to hold return value
    css::uno::Reference<css::text::XTextViewCursorSupplier> xTextCursorSupplier;
    css::uno::Reference<css::text::XTextViewCursor> xTextCursor;
    css::uno::Reference<css::sheet::XSheetCellRange> xSheetCellRange;
	css::uno::Reference<css::sheet::XCellRangeReferrer> xCellRangeReferrer;
	css::uno::Reference<css::frame::XController> xController(xModel->getCurrentController(), css::uno::UNO_QUERY);
    if (xController.is())
    {
#ifdef GROOO_DEBUG
        logMsg("Have controller\n");
#endif
		// For Writer
        xTextCursorSupplier.set(xController, css::uno::UNO_QUERY);
        
		// for Calc
		xCellRangeReferrer.set(xController, css::uno::UNO_QUERY);
    }
	css::uno::Any aSelection;
	if (xSelection.is())
    {
        aSelection = xSelection.get()->getSelection();
#ifdef GROOO_DEBUG
        ::rtl::OString aTypeName;
        aSelection.getValueTypeName().convertToString(&aTypeName, RTL_TEXTENCODING_UTF8, 128);
        logMsg("Selection type: %s\n", aTypeName.getStr());
#endif

		if (aSelection.has<css::uno::Reference<css::uno::XInterface> >())
		{
			css::uno::Reference<css::uno::XInterface> xInterface =
					aSelection.get<css::uno::Reference<css::uno::XInterface> >();
			css::uno::Reference<css::drawing::XShapes> xShapes;
			xShapes.set(xInterface, css::uno::UNO_QUERY);
			// for the moment only support setting text when only one shape is selected
			if (xShapes.is() && (xShapes.get()->getCount() == 1) )
			{
				xTextProperties.set(xShapes.get()->getByIndex(0), css::uno::UNO_QUERY);
			}
			else
			{
				xTextCursor.set(xInterface, css::uno::UNO_QUERY);
				if (xTextCursor.is())
				{
					xTextProperties.set(xTextCursor, css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
					logMsg("Have text cursor from selection\n");
#endif
				}
				xSheetCellRange.set(xInterface, css::uno::UNO_QUERY);
                if (xSheetCellRange.is())
                {
                    xTextProperties.set(xSheetCellRange, css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
                    logMsg("Have cell cursor from selection %d\n", xTextProperties.is());
#endif
                }
			}
		}
		if (!xTextProperties.is())
		{
			if (xTextCursorSupplier.is())
			{
				xTextCursor.set(xTextCursorSupplier->getViewCursor());
				if (xTextCursor.is())
				{
					xTextProperties.set(xTextCursor, css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
                    logMsg("Have text cursor %d\n", xTextProperties.is());
#endif
				}
				else
                {
#ifdef GROOO_DEBUG
                    logMsg("Have text cursor supplier but no cursor\n");
#endif
                }
			}
			else if (xCellRangeReferrer.is())
			{
				css::uno::Reference< css::table::XCellRange> xCellRange = xCellRangeReferrer.get()->getReferredCells();
				if (xCellRange.is())
				{
					xTextProperties.set(xCellRange, css::uno::UNO_QUERY);
	#ifdef GROOO_DEBUG
					logMsg("Have cell range\n");
	#endif
				}
			}
		}
	}

	// diagnostic
	if (!xTextProperties.is())
    {
		if (aSelection.has<css::uno::Reference<css::uno::XInterface> >())
		{
			css::uno::Reference<css::uno::XInterface> xInterface =
				aSelection.get<css::uno::Reference<css::uno::XInterface> >();
			xTextProperties.set(xInterface, css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
			logMsg("Have no cursor text properties %d\n", xTextProperties.is());
			if (xTextProperties.is())
				printPropertyNames(xTextProperties);
			css::uno::Reference<css::drawing::XShapes> xShapes;
			xShapes.set(xInterface, css::uno::UNO_QUERY);
			if (xShapes.is())
				logMsg("Selection has %d shapes\n", xShapes.get()->getCount());

#endif
		}
	}
	return xTextProperties;
}

css::uno::Reference< css::resource::XStringResourceWithLocation>
osg::getResource(
        css::uno::Reference< css::uno::XComponentContext > const & context,
                    ::rtl::OUString basename)
{
    css::uno::Reference< css::deployment::XPackageInformationProvider >
                    xInfoProvider( css::deployment::PackageInformationProvider::get( context) );
    rtl::OUString sLocation = xInfoProvider->getPackageLocation(
        rtl::OUString::createFromAscii( "org.sil.graphite.GraphiteOptions" ) );
    rtl::OString aLocation;
    sLocation.convertToString(&aLocation, RTL_TEXTENCODING_UTF8, 128);
    rtl::OUString dialogDir = rtl::OUString::createFromAscii("/dialogs/");
    rtl::OUString resUrl = sLocation + dialogDir;
    ::com::sun::star::lang::Locale locale;
    // null handler
    ::css::uno::Reference<css::task::XInteractionHandler> xInteractionHandler;
    return css::resource::StringResourceWithLocation::create(context,
        resUrl, sal_True, locale, basename, rtl::OUString(),
        xInteractionHandler);
}

// Define the supported services
static ::cppu::ImplementationEntry const entries[] = {
    { &org::sil::graphite::graphiteaddon::_create,
      &org::sil::graphite::graphiteaddon::_getImplementationName,
      &org::sil::graphite::graphiteaddon::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &org::sil::graphite::dialogeventhandler::_create,
      &org::sil::graphite::dialogeventhandler::_getImplementationName,
      &org::sil::graphite::dialogeventhandler::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &org::sil::graphite::setupcontextmenu::_create,
      &org::sil::graphite::setupcontextmenu::_getImplementationName,
      &org::sil::graphite::setupcontextmenu::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

	SAL_DLLPUBLIC_EXPORT 
	void SAL_CALL component_getImplementationEnvironment(
		const char ** envTypeName, uno_Environment **)
	{
#ifdef _MSC_VER
		OutputDebugStringA("getImplementationEnvironment\n");
#endif
		*envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
	}

	SAL_DLLPUBLIC_EXPORT
	void * SAL_CALL component_getFactory(
		const char * implName, void * serviceManager, void * registryKey)
	{
#ifdef GROOO_DEBUG
		osg::logMsg("component_getFactory %s\n", implName);
#endif
		return ::cppu::component_getFactoryHelper(
			implName, serviceManager, registryKey, entries);
	}

	SAL_DLLPUBLIC_EXPORT
	sal_Bool SAL_CALL component_writeInfo(
		void * serviceManager, void * registryKey)
	{
		return ::cppu::component_writeInfoHelper(serviceManager, registryKey, entries);
	}

}
