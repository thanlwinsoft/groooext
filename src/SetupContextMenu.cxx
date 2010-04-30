/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: org::sil::graphite::SetupContextMenu.cxx,v $
 *
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
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
#include <cstdio>

#ifdef SAL_UNX
#include "sal/typesizes.h"
#endif
#include "sal/config.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase3.hxx"
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/embed/XEmbeddedObject.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier2.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/FrameActionEvent.hpp"
#include "com/sun/star/frame/XFrameActionListener.hpp"
#include "com/sun/star/frame/DispatchResultEvent.hpp"
#include "com/sun/star/frame/DispatchResultState.hpp"
#include "com/sun/star/sheet/XCellRangeReferrer.hpp"
#include "com/sun/star/table/XCellRange.hpp"
#include "com/sun/star/table/XTableChart.hpp"
#include "com/sun/star/table/XTableCharts.hpp"
#include "com/sun/star/sheet/XSpreadsheetDocument.hpp"
#include "com/sun/star/sheet/XSpreadsheet.hpp"
#include "com/sun/star/sheet/XSpreadsheets.hpp"
#include "com/sun/star/table/XTableChartsSupplier.hpp"
#include "com/sun/star/text/XTextSectionsSupplier.hpp"
#include "com/sun/star/text/XTextViewCursor.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/ui/ContextMenuInterceptorAction.hpp"
#include "com/sun/star/ui/XContextMenuInterception.hpp"
#include "com/sun/star/ui/XContextMenuInterceptor.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"

#include "groooDebug.hxx"
#include "graphiteooo.hxx"
#include "SetupContextMenu.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"

namespace css = ::com::sun::star;

namespace org { namespace sil { namespace graphite { class SetupContextMenu; }}}

class org::sil::graphite::SetupContextMenu:
    public ::cppu::WeakImplHelper3<
	css::task::XJob, css::ui::XContextMenuInterceptor, css::frame::XFrameActionListener>
{
public:
    explicit SetupContextMenu(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::task::XJob:
    virtual ::com::sun::star::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue > & Arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception);

    // ::com::sun::star::uno::XContextMenuInterceptor
    virtual css::ui::ContextMenuInterceptorAction SAL_CALL notifyContextMenuExecute( const ::com::sun::star::ui::ContextMenuExecuteEvent& aEvent ) throw (css::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener:
	virtual void SAL_CALL disposing(const css::lang::EventObject & Source) throw (css::uno::RuntimeException) {}
// ::com::sun::star::frame::XFrameActionListener
	virtual void SAL_CALL frameAction( const ::com::sun::star::frame::FrameActionEvent& Action ) throw (::com::sun::star::uno::RuntimeException);

private:
    SetupContextMenu(const org::sil::graphite::SetupContextMenu &); // not defined
    SetupContextMenu& operator=(const org::sil::graphite::SetupContextMenu &); // not defined

    // destructor is private and will be called indirectly by the release call
    virtual ~SetupContextMenu() {}

	void addMenuInterceptionToModel(css::uno::Reference< css::frame::XModel > xModel);


    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XController > m_xController;
};

org::sil::graphite::SetupContextMenu::SetupContextMenu(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
#ifdef GROOO_DEBUG
    logMsg("SetupContextMenu constructor\n");
#endif
}

// ::com::sun::star::task::XJob:
::com::sun::star::uno::Any SAL_CALL org::sil::graphite::SetupContextMenu::execute(const css::uno::Sequence< css::beans::NamedValue > & arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception)
{
    css::uno::Any environment;
    css::uno::Any config;
    css::uno::Any jobConfig;
#ifdef GROOO_DEBUG
    logMsg("SetupContextMenu execute %ld args\n", arguments.getLength());
#endif
    for (int i=0; i < arguments.getLength(); i++)
    {
        ::rtl::OUString name = arguments[i].Name;
        if (name.compareToAscii("Environment") == 0) environment = arguments[i].Value;
        else if (name.compareToAscii("Config") == 0) config = arguments[i].Value;
        else if (name.compareToAscii("JobConfig") == 0) jobConfig = arguments[i].Value;
    }
    css::uno::Any envType;
    css::uno::Any envFrame;
    css::uno::Any envModel;
    css::uno::Any envName;
    if (environment.has< css::uno::Sequence< css::beans::NamedValue > >())
    {
#ifdef GROOO_DEBUG
        logMsg("Job has environment\n");
#endif
        css::uno::Sequence< css::beans::NamedValue > envSequence = environment.get<css::uno::Sequence< css::beans::NamedValue > >() ;
        for (int i=0; i < envSequence.getLength(); i++)
        {
            ::rtl::OUString name = envSequence[i].Name;
            if (name.compareToAscii("EnvType") == 0) envType = envSequence[i].Value;
            else if (name.compareToAscii("Frame") == 0) envFrame = envSequence[i].Value;
            else if (name.compareToAscii("Model") == 0) envModel = envSequence[i].Value;
            else if (name.compareToAscii("EventName") == 0) envName = envSequence[i].Value;
        }
    }
#ifdef GROOO_DEBUG
    if (envType.hasValue())
    {
        ::rtl::OString asciiEnvType;
        ::rtl::OUString environmentType = envType.get< ::rtl::OUString >();
        environmentType.convertToString(&asciiEnvType, RTL_TEXTENCODING_UTF8, 128);
        logMsg("Job env type %s\n", asciiEnvType.getStr());
    }
    if (envFrame.hasValue())
        logMsg("Job Environment has frame\n");
    if (envModel.hasValue())
        logMsg("Job Environment has model\n");
#endif
    css::uno::Reference< css::frame::XModel > xModel;
    if (envModel.hasValue())
    {
        xModel.set(envModel.get< css::uno::Reference< css::frame::XModel > >());
		addMenuInterceptionToModel(xModel);
#if 0
			css::uno::Reference<css::lang::XMultiServiceFactory> xMSFactory(xModel, css::uno::UNO_QUERY);
			if (xMSFactory.is())
			{
				css::uno::Reference<css::container::XIndexAccess> embeddedObjects;
				css::uno::Reference<css::container::XIndexAccess> embeddedChartObjects;
				try
				{
					embeddedObjects.set(
						xMSFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.text.TextEmbeddedObjects")),
						css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
					logMsg("got TextEmbeddedObjects service %d\n", embeddedObjects.is());
#endif
				}
				catch (css::uno::Exception e)
				{
#ifdef GROOO_DEBUG
					logMsg("failed to get TextEmbeddedObjects service\n");
#endif
				}
				try
				{
					embeddedChartObjects.set(
						xMSFactory->createInstance(rtl::OUString::createFromAscii("com.sun.star.table.TableCharts")),
						css::uno::UNO_QUERY);
#ifdef GROOO_DEBUG
					logMsg("got TableCharts service %d\n", embeddedChartObjects.is());
#endif
				}
				catch (css::uno::Exception e)
				{
#ifdef GROOO_DEBUG
					logMsg("failed to get TableCharts service\n");
#endif
				}
				if (embeddedObjects.is())
				{
					logMsg("doc has %d embedded objects\n", embeddedObjects->getCount());
					for (int i = 0; i < embeddedObjects->getCount(); i++)
					{
						css::uno::Any anEmbedded = embeddedObjects->getByIndex(i);
						css::uno::Reference < css::document::XEmbeddedObjectSupplier > embeddedSupplier;
						if (anEmbedded.has< css::uno::Reference <css::document::XEmbeddedObjectSupplier> >())
						{
							embeddedSupplier.set(anEmbedded.get<css::uno::Reference 
								<css::document::XEmbeddedObjectSupplier> >(),
								css::uno::UNO_QUERY);
							css::uno::Reference<css::lang::XComponent> xComponent = embeddedSupplier->getEmbeddedObject();
							logMsg("have embedded component %d\n", xComponent.is());
						}
						else
						{
#ifdef GROOO_DEBUG
							logMsg("embeded object is not an XEmbeddedObjectSupplier\n");
#endif
						}
					}
				}
				if (embeddedChartObjects.is())
				{
					logMsg("doc has %d embedded objects\n", embeddedChartObjects->getCount());
					for (int i = 0; i < embeddedChartObjects->getCount(); i++)
					{
						css::uno::Any anEmbedded = embeddedChartObjects->getByIndex(i);
						css::uno::Reference < css::document::XEmbeddedObjectSupplier > embeddedSupplier;
						if (anEmbedded.has< css::uno::Reference <css::document::XEmbeddedObjectSupplier> >())
						{
							embeddedSupplier.set(anEmbedded.get<css::uno::Reference 
								<css::document::XEmbeddedObjectSupplier> >(),
								css::uno::UNO_QUERY);
							css::uno::Reference<css::lang::XComponent> xComponent = embeddedSupplier->getEmbeddedObject();
							logMsg("have embedded component %d\n", xComponent.is());
						}
						else
						{
#ifdef GROOO_DEBUG
							logMsg("embeded object is not an XEmbeddedObjectSupplier\n");
#endif
						}
					}
				}
			}
#endif
			// TODO look for embedded documents such as charts and add interceptors to them as well
			// embedded documents don't seem to fire the onDocumentOpenned event
			// com.sun.star.document.XEmbeddedObjectSupplier
			// e.g. XTableCharts.getByIndex(i) or TextEmbeddedObjects.getByIndex(i)
			// XEmbeddedObjectSupplier::getEmbeddedObject()
			//css::uno::Reference<css::document::
    }
    css::uno::Sequence< css::beans::NamedValue > jobReturn(3);
    css::beans::NamedValue deactivate(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Deactivate")), css::uno::Any(sal_False));
    jobReturn[0] = deactivate;
    css::beans::NamedValue saveArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SaveArguments")), jobConfig);
    jobReturn[1] = saveArguments;
    css::frame::DispatchResultEvent resultEvent;
    resultEvent.State = css::frame::DispatchResultState::SUCCESS;
    resultEvent.Result = css::uno::Any();
    css::beans::NamedValue sendDispatchResult(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SendDispatchResult")), css::uno::Any(resultEvent));
    jobReturn[2] = sendDispatchResult;
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return ::com::sun::star::uno::Any();
}


void org::sil::graphite::SetupContextMenu::addMenuInterceptionToModel(css::uno::Reference< css::frame::XModel > xModel)
{
	m_xController.set(xModel->getCurrentController());
    if (m_xController.is())
    {
        css::uno::Reference< css::ui::XContextMenuInterception > interception(m_xController, css::uno::UNO_QUERY);
        if (interception.is())
        {
#ifdef GROOO_DEBUG
            logMsg("have menu interception from Controller\n");
#endif
            interception.get()->registerContextMenuInterceptor(this);
        }
		css::uno::Reference<css::sheet::XSpreadsheetDocument> xSpreadsheetDoc;
		xSpreadsheetDoc.set(xModel, css::uno::UNO_QUERY);
		if (xSpreadsheetDoc.is())
		{
			css::uno::Reference<css::sheet::XSpreadsheets> xSheets = xSpreadsheetDoc->getSheets();
			css::uno::Sequence< ::rtl::OUString> sheetNames = xSheets->getElementNames();
			logMsg("have spreadsheet doc with %d sheets\n", sheetNames.getLength());
			for (int i = 0; i < sheetNames.getLength(); i++)
			{
				css::uno::Any aSheet = xSheets->getByName(sheetNames[i]);
				css::uno::Reference<css::sheet::XSpreadsheet> xSheet = 
					aSheet.get<css::uno::Reference<css::sheet::XSpreadsheet> >();
				if (!xSheet.is())
					logMsg("Failed to get Xspreadsheet\n");
				css::uno::Reference<css::table::XTableChartsSupplier> xSupplier;
				xSupplier.set(xSheet, css::uno::UNO_QUERY);
				if (xSupplier.is())
				{
					css::uno::Reference<css::table::XTableCharts> xCharts = xSupplier->getCharts();
					css::uno::Sequence< ::rtl::OUString> chartNames = xCharts->getElementNames();
					logMsg("have chart supplier with %d charts\n", chartNames.getLength());
					for (int j = 0; j < chartNames.getLength(); j++)
					{
						css::uno::Any aChart = xCharts->getByName(chartNames[j]);
						css::uno::Reference<css::table::XTableChart> xChart =
							aChart.get<css::uno::Reference<css::table::XTableChart> >();
						if (xChart.is())
						{
							logMsg("have chart\n");
							// XEmbeddedObjectSupplier2 is not supported :-(
							css::uno::Reference< css::document::XEmbeddedObjectSupplier > xChartSupplier(xChart, css::uno::UNO_QUERY);
							if (!xChartSupplier.is())
							{
								logMsg("chart nas no object supplier\n");
								continue;
							}
							css::uno::Reference<css::lang::XComponent> xChartComponent = 
								xChartSupplier->getEmbeddedObject();
							if (!xChartComponent.is())
								continue;
							css::uno::Reference <SetupContextMenu> setupContextMenu(this);
							css::uno::Reference <css::lang::XEventListener> listener(setupContextMenu, css::uno::UNO_QUERY);
							xChartComponent->addEventListener(listener);
							/*
							css::uno::Reference< css::embed::XEmbeddedObject > xEmbeddedObject =
								xChartSupplier->getExtendedControlOverEmbeddedObject();
							if (xEmbeddedObject.is())
							{
								xEmbeddedObject->addEventListener(listener);
								logMsg("added event listener\n");
							}
							*/
							css::uno::Reference< css::frame::XModel > xChartModel(xChartComponent, css::uno::UNO_QUERY);
							if (xChartModel.is())
							{
								css::uno::Reference<org::sil::graphite::SetupContextMenu> chartContextMenu =
									new org::sil::graphite::SetupContextMenu(m_xContext);
								
								chartContextMenu->addMenuInterceptionToModel(xChartModel);
								css::uno::Reference<css::frame::XController> xController;
								xController.set(xChartModel->getCurrentController());
								logMsg("have found chart model %d controller\n", xController.is());
							}
						}
					}
				}
				else
				{
					logMsg("No charts supplier\n");
				}
			}
		}
    }
	else
	{
		logMsg("No controller found\n");
	}
}

css::ui::ContextMenuInterceptorAction SAL_CALL
org::sil::graphite::SetupContextMenu::notifyContextMenuExecute( const ::com::sun::star::ui::ContextMenuExecuteEvent& aEvent ) throw (css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
     logMsg("notifyContextMenuExecute\n");
#endif
    // Is there a relevant useful selection?
	css::uno::Any aSelection;
    css::uno::Reference<css::view::XSelectionSupplier> xSelection(aEvent.Selection);
	css::uno::Reference< css::beans::XPropertySet> xTextProperties =
		getTextPropertiesFromModel(m_xController->getModel(), xSelection);
	
    bool haveGraphiteFont = false;

	if (xTextProperties.is())
	{
        for (int i=0; i < FeatureDialogEventHandler::NUM_SCRIPTS; i++)
        {
			if (xTextProperties.get()->getPropertySetInfo().get()->
				hasPropertyByName(FeatureDialogEventHandler::FONT_PROPERTY_NAME[i]))
			{
				css::uno::Any aFontName = xTextProperties.get()->getPropertyValue(FeatureDialogEventHandler::FONT_PROPERTY_NAME[i]);
				::rtl::OUString fontName = aFontName.get< ::rtl::OUString >();
				sal_Int32 featStart = fontName.indexOf(GraphiteFontInfo::FEAT_PREFIX);
				if (featStart > -1)
				{
					fontName = fontName.copy(0, featStart);
				}
				if (GraphiteFontInfo::getFontInfo().isGraphiteFont(fontName))
					haveGraphiteFont = true;
			}
        }
    }
    
    // no need for a menu if there are no graphite fonts in use
    if (!haveGraphiteFont)
        return css::ui::ContextMenuInterceptorAction_IGNORED;

    css::uno::Reference<css::container::XIndexContainer> xContextMenu(aEvent.ActionTriggerContainer);
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory(xContextMenu, css::uno::UNO_QUERY);
    if (xFactory.is())
    {
        static const ::rtl::OUString actionTrigger(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTrigger"));
        static const ::rtl::OUString actionTriggerContainer(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTriggerContainer"));
        static const ::rtl::OUString actionTriggerSeparator(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTriggerSeparator"));
        static const ::rtl::OUString PROP_TEXT(RTL_CONSTASCII_USTRINGPARAM("Text"));
        static const ::rtl::OUString PROP_COMMAND_URL(RTL_CONSTASCII_USTRINGPARAM("CommandURL"));
        static const ::rtl::OUString PROP_SUBCONTAINER(RTL_CONSTASCII_USTRINGPARAM("SubContainer"));
		css::uno::Reference<css::beans::XPropertySet> xGrSeparator(xFactory.get()->createInstance (actionTriggerSeparator), css::uno::UNO_QUERY);
        css::uno::Reference<css::beans::XPropertySet> xGrMenuRoot(xFactory.get()->createInstance (actionTrigger), css::uno::UNO_QUERY);
        css::uno::Reference<css::beans::XPropertySet> xGrSubMenu(xFactory.get()->createInstance (actionTriggerContainer), css::uno::UNO_QUERY);
        css::uno::Any grFeaturesText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphite Features...")));
        xGrMenuRoot.get()->setPropertyValue(PROP_TEXT, grFeaturesText);
        css::uno::Any grFeaturesCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.graphiteoptions:contextmenu")));
        xGrMenuRoot.get()->setPropertyValue(PROP_COMMAND_URL, grFeaturesCommand);
        css::uno::Any grSubMenu(xGrSubMenu);
        xGrMenuRoot.get()->setPropertyValue(PROP_SUBCONTAINER, grSubMenu);

		xContextMenu.get()->insertByIndex(xContextMenu.get()->getCount(), css::uno::Any(xGrSeparator));
        xContextMenu.get()->insertByIndex(xContextMenu.get()->getCount(), css::uno::Any(xGrMenuRoot));

        return css::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED;
    }
  
    return css::ui::ContextMenuInterceptorAction_IGNORED;
}


void SAL_CALL
org::sil::graphite::SetupContextMenu::frameAction( const ::com::sun::star::frame::FrameActionEvent& action ) throw (::com::sun::star::uno::RuntimeException)
{
	logMsg("frameAction %d\n", action.Action);
}

// component helper namespace
namespace org { namespace sil { namespace graphite { namespace setupcontextmenu {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org::sil::graphite::SetupContextMenu"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
#ifdef GROOO_DEBUG
    logMsg("SetupContextMenu _getSupportedServiceNames\n");
#endif
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.SetupContextMenu"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::SetupContextMenu(context));
}

}}}} // closing component helper namespace


