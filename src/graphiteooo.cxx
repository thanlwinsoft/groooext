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

#include <cstring>

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/chart/XChartDocument.hpp"
#include "com/sun/star/chart2/XAxis.hpp"
#include "com/sun/star/chart2/XDiagram.hpp"
#include "com/sun/star/chart2/XInternalDataProvider.hpp"
#include "com/sun/star/chart2/XChartDocument.hpp"
#include "com/sun/star/chart2/XFormattedString.hpp"
#include "com/sun/star/chart2/XCoordinateSystem.hpp"
#include "com/sun/star/chart2/XCoordinateSystemContainer.hpp"
#include "com/sun/star/chart2/XTitle.hpp"
#include "com/sun/star/chart2/XTitled.hpp"
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
#include "com/sun/star/resource/XLocale.hpp"
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

#include "OptionsDialogEventHandler.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "SetupContextMenu.hxx"

const char * ::org::sil::graphite::SAL_DISABLE_GRAPHITE = "SAL_DISABLE_GRAPHITE";
const char * ::org::sil::graphite::SAL_GRAPHITE_CACHE_SIZE = "SAL_GRAPHITE_CACHE_SIZE";

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
        properties[i].Name.convertToString(&propName, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
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

void osg::printServiceNames(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xInterface)
{
    css::uno::Reference<css::lang::XServiceInfo> xServiceInfo(xInterface, css::uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        ::rtl::OString aServiceName;
        css::uno::Sequence< ::rtl::OUString> serviceNames =
            xServiceInfo->getSupportedServiceNames();
        for (int i = 0; i < serviceNames.getLength(); i++)
        {
            serviceNames[i].convertToString(&aServiceName, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
            osg::logMsg("service: %s\n", aServiceName.getStr());
        }
    }
}

sal_Bool osg::isGraphiteEnabled()
{
	sal_Bool environmentGraphiteEnabled = sal_False;
#ifdef _MSC_VER
    char * pDisableGraphiteStr = NULL;
	size_t envBufSize = 0;
	if (_dupenv_s(&pDisableGraphiteStr, &envBufSize, SAL_DISABLE_GRAPHITE))
		pDisableGraphiteStr = NULL;
#else
	const char * pDisableGraphiteStr = getenv(SAL_DISABLE_GRAPHITE);
#endif

    if (pDisableGraphiteStr == NULL || pDisableGraphiteStr[0]=='0')
        environmentGraphiteEnabled = sal_True;

#ifdef _MSC_VER
	if (pDisableGraphiteStr)
	{
		free(pDisableGraphiteStr);
		pDisableGraphiteStr = NULL;
	}
#endif
	return environmentGraphiteEnabled;
}

css::uno::Reference< css::beans::XPropertySet> osg::getChartComponentProperties(
    css::uno::Reference <css::chart2::XChartDocument> & xChart2Document,
    ::rtl::OUString selectionId)
{
    //CID/Title=
    //subtitle CID/D=0:Title=
    //2nd yaxis title CID/D=0:CS=0:Axis=1,1:Title=
    //1st xaxis title CID/D=0:CS=0:Axis=0,0:Title=
    //legend CID/D=0:Legend=
    css::uno::Reference< css::drawing::XShape> xShape;
    css::uno::Reference< css::beans::XPropertySet> xTextProperties;

    css::uno::Reference <css::chart::XChartDocument> xChartDocument(xChart2Document, css::uno::UNO_QUERY);
    css::uno::Reference <css::chart2::XDiagram> xDiagram2(xChart2Document->getFirstDiagram(), css::uno::UNO_QUERY);
    css::uno::Reference< css::chart2::XCoordinateSystemContainer> xCoordContainer(xDiagram2, css::uno::UNO_QUERY);

    const char * coordPrefix = "CID/D=0:CS=";
    const char * axisPrefix = "Axis=";
    const char * titlePrefix = "Title=";

    printServiceNames(xDiagram2);

    if (selectionId.equalsAscii("CID/Title="))
    {
        xShape = xChartDocument->getTitle();
    }
    else if (selectionId.equalsAscii("CID/D=0:Title="))
    {
        xShape = xChartDocument->getSubTitle();
    }
    else if (selectionId.equalsAscii("CID/D=0:Legend="))
    {
        xShape = xChartDocument->getLegend();
    }
    else if (xDiagram2.is() && xCoordContainer.is() &&
        selectionId.indexOfAsciiL(coordPrefix, strlen(coordPrefix)) == 0)
    {
        ::rtl::OUString coordIdString(selectionId.getStr() + strlen(coordPrefix), 1);
        int coordId = coordIdString.toInt32();
        int axisCharIndex = selectionId.indexOfAsciiL(axisPrefix, strlen(axisPrefix));
        ::rtl::OUString axisPrimaryString(selectionId.getStr() + axisCharIndex + strlen(axisPrefix), 1);
        ::rtl::OUString axisSecondaryString(selectionId.getStr() + axisCharIndex + strlen(axisPrefix) + 2, 1);
        int axisDimension = axisPrimaryString.toInt32();
        int axisIndex = axisSecondaryString.toInt32();
        css::uno::Sequence<css::uno::Reference< css::chart2::XCoordinateSystem> >  coordSystems = xCoordContainer->getCoordinateSystems();

#ifdef GROOO_DEBUG
        logMsg("editing coordsystem %d/%d axis %d/%d,%d/%d\n", coordId,
               coordSystems.getLength(), axisDimension,
               coordSystems[coordId]->getDimension(), axisIndex,
               coordSystems[coordId]->getMaximumAxisIndexByDimension(axisDimension));
#endif
        if (coordId < coordSystems.getLength() &&
            axisDimension < coordSystems[coordId]->getDimension() &&
            axisIndex <= coordSystems[coordId]->getMaximumAxisIndexByDimension(axisDimension))
        {
            css::uno::Reference<css::chart2::XAxis>xAxis =
                coordSystems[coordId]->getAxisByDimension(axisDimension, axisIndex);
            logMsg("have axis %d\n", xAxis.is());
            if (selectionId.indexOfAsciiL(titlePrefix, strlen(titlePrefix)) > -1)
            {
                css::uno::Reference<css::chart2::XTitled>xTitled(xAxis, css::uno::UNO_QUERY);
                if (xTitled.is())
                {
                    css::uno::Sequence<css::uno::Reference<css::chart2::XFormattedString> >
                        formattedStrings = xTitled->getTitleObject()->getText();
                    if (formattedStrings.getLength() > 0)
                    {
                        xTextProperties.set(formattedStrings[0], css::uno::UNO_QUERY);
                    }
                }
            }
            else
            {
                xTextProperties.set(xAxis, css::uno::UNO_QUERY);
            }
        }
    }

    if (xShape.is())
        xTextProperties.set(xShape, css::uno::UNO_QUERY);
    return xTextProperties;
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
    
    css::uno::Reference<css::chart2::XInternalDataProvider> xChartDataProvider;
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
        aSelection.getValueTypeName().convertToString(&aTypeName, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        logMsg("Selection type: %s\n", aTypeName.getStr());
#endif

		if (aSelection.has<css::uno::Reference<css::uno::XInterface> >())
		{
			css::uno::Reference<css::uno::XInterface> xInterface =
					aSelection.get<css::uno::Reference<css::uno::XInterface> >();
			css::uno::Reference<css::drawing::XShapes> xShapes;
			xShapes.set(xInterface, css::uno::UNO_QUERY);
            css::uno::Reference<css::container::XIndexAccess >
                xContainer(xInterface, css::uno::UNO_QUERY);
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
                if (xContainer.is() && (xContainer->getCount() == 1))
                {
                    css::uno::Reference<css::text::XTextRange> xTextRange(
                        xContainer->getByIndex(0), css::uno::UNO_QUERY);
                    if (xTextRange.is())
                    {
                        xTextProperties.set(xTextRange, css::uno::UNO_QUERY);
                        logMsg("have text range from selection\n");
                    }
                }
			}
		}
		else if (aSelection.has< ::rtl::OUString>())
        {
            ::rtl::OUString uSelection(aSelection.get< ::rtl::OUString>());
#ifdef GROOO_DEBUG
            ::rtl::OString aSelectionText;
            uSelection.convertToString(&aSelectionText, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
            logMsg("Selection text: %s\n", aSelectionText.getStr());
            printServiceNames(xController);
            logMsg("chart model?\n");
            printServiceNames(xModel);
#endif
            css::uno::Reference <css::chart2::XChartDocument> xChartDocument(xModel, css::uno::UNO_QUERY);
            if (xChartDocument.is())
            {
#ifdef GROOO_DEBUG
                logMsg("have chart doc\n");
#endif
                xTextProperties.set(getChartComponentProperties(xChartDocument, uSelection));
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
                    ::rtl::OUString basename, css::lang::Locale locale)
{
    css::uno::Reference< css::deployment::XPackageInformationProvider >
                    xInfoProvider( css::deployment::PackageInformationProvider::get( context) );
    rtl::OUString sLocation = xInfoProvider->getPackageLocation(
        rtl::OUString::createFromAscii( "org.sil.graphite.GraphiteOptions" ) );
    rtl::OString aLocation;
    sLocation.convertToString(&aLocation, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    rtl::OUString dialogDir = rtl::OUString::createFromAscii("/dialogs/");
    rtl::OUString resUrl = sLocation + dialogDir;
#ifdef GROOO_DEBUG
    ::rtl::OString aLang;
    ::rtl::OString aCountry;
    locale.Language.convertToString(&aLang, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    locale.Country.convertToString(&aCountry, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    logMsg("graphiteooo locale %s-%s\n", aLang.getStr(), aCountry.getStr());
#endif
    // null handler
    ::css::uno::Reference<css::task::XInteractionHandler> xInteractionHandler;
    return css::resource::StringResourceWithLocation::create(context,
        resUrl, sal_True, locale, basename, rtl::OUString(),
        xInteractionHandler);
}

::rtl::OUString
osg::getResourceString(css::uno::Reference<css::resource::XStringResourceResolver>  resource, const char * resourceId)
{
    ::rtl::OUString resourceIdString = ::rtl::OUString::createFromAscii(resourceId);
    if (resource.is())
    {
        try
        {
            ::rtl::OUString msgString = resource->resolveString(resourceIdString);
            return msgString;
        }
        catch (css::resource::MissingResourceException mre)
        {
            logMsg("Resource %s not found\n", resourceId);
        }
    }
    return resourceIdString;
}

// Define the supported services
static ::cppu::ImplementationEntry const entries[] = {
    { &org::sil::graphite::graphiteaddon::_create,
      &org::sil::graphite::graphiteaddon::_getImplementationName,
      &org::sil::graphite::graphiteaddon::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &org::sil::graphite::optionsdialogeventhandler::_create,
      &org::sil::graphite::optionsdialogeventhandler::_getImplementationName,
      &org::sil::graphite::optionsdialogeventhandler::_getSupportedServiceNames,
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
