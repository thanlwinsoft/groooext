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

#include <cstdio>

#ifdef SAL_UNX
#include "sal/typesizes.h"
#endif
#include "sal/config.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase4.hxx"
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/embed/XEmbeddedObject.hpp"
#include "com/sun/star/document/XDocumentEventBroadcaster.hpp"
#include "com/sun/star/document/XEventBroadcaster.hpp"
#include "com/sun/star/document/XEventListener.hpp"
#include "com/sun/star/document/XDocumentEventListener.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier2.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/frame/FrameAction.hpp"
#include "com/sun/star/frame/FrameSearchFlag.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XFrames.hpp"
#include "com/sun/star/frame/XFramesSupplier.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/FrameAction.hpp"
#include "com/sun/star/frame/FrameActionEvent.hpp"
#include "com/sun/star/frame/XFrameActionListener.hpp"
#include "com/sun/star/frame/DispatchResultEvent.hpp"
#include "com/sun/star/frame/DispatchResultState.hpp"
#include "com/sun/star/graphic/XGraphic.hpp"
#include "com/sun/star/sheet/XCellRangeReferrer.hpp"
#include "com/sun/star/table/XCellRange.hpp"
#include "com/sun/star/table/XTableChart.hpp"
#include "com/sun/star/table/XTableCharts.hpp"
#include "com/sun/star/resource/XStringResourceResolver.hpp"
#include "com/sun/star/reflection/XIdlReflection.hpp"
#include "com/sun/star/reflection/XIdlClass.hpp"
#include "com/sun/star/reflection/XIdlClassProvider.hpp"
#include "com/sun/star/sheet/XSpreadsheetDocument.hpp"
#include "com/sun/star/sheet/XSpreadsheet.hpp"
#include "com/sun/star/sheet/XSpreadsheets.hpp"
#include "com/sun/star/table/XTableChartsSupplier.hpp"
#include "com/sun/star/text/XTextSectionsSupplier.hpp"
#include "com/sun/star/text/XTextViewCursor.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/ui/ContextMenuInterceptorAction.hpp"
#include "com/sun/star/ui/XContextMenuInterception.hpp"
#include "com/sun/star/ui/XContextMenuInterceptor.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"
#include "com/sun/star/view/XSelectionChangeListener.hpp"

#include "groooDebug.hxx"
#include "graphiteooo.hxx"
#include "SetupContextMenu.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"
#include "GraphiteConfiguration.hxx"

namespace css = ::com::sun::star;

namespace org { namespace sil { namespace graphite { class SetupContextMenu; }}}

class org::sil::graphite::SetupContextMenu:
    public ::cppu::WeakImplHelper4<
    css::task::XJob, css::ui::XContextMenuInterceptor,
    css::view::XSelectionChangeListener,
    css::frame::XFrameActionListener>
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
    // ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

private:
    SetupContextMenu(const org::sil::graphite::SetupContextMenu &); // not defined
    SetupContextMenu& operator=(const org::sil::graphite::SetupContextMenu &); // not defined

    // destructor is private and will be called indirectly by the release call
    virtual ~SetupContextMenu() {}

	void addMenuInterceptionToModel(css::uno::Reference< css::frame::XModel > xModel);


    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    GraphiteConfiguration m_config;
    css::uno::Reference< css::frame::XController > m_xController;
    css::uno::Reference< css::resource::XStringResourceResolver> m_xResource;
};

org::sil::graphite::SetupContextMenu::SetupContextMenu(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_config(context),
    m_xResource(getResource(context, ::rtl::OUString::createFromAscii("GraphiteMessages"),
                            m_config.locale()), css::uno::UNO_QUERY)
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
    if (isGraphiteEnabled() && envModel.hasValue())
    {
        xModel.set(envModel.get< css::uno::Reference< css::frame::XModel > >());
		addMenuInterceptionToModel(xModel);
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
    return ::com::sun::star::uno::Any(jobReturn);
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
            // NOTE: as of OOo 3.2 ChartController::registerContextMenuInterceptor
            // doesn't do anything, hence this doesn't work for charts, but it
            // should work for other document types.
        }
        css::uno::Reference< css::frame::XFrame > xFrame(m_xController->getFrame());
        if (xFrame.is())
        {
            css::uno::Reference< css::frame::XFrameActionListener> xFrameListener(this);
            try
            {
                xFrame->removeFrameActionListener(this);
            }
            catch(...)
            {
                logMsg("Remove threw exception\n");
            }
            // the frame listener is used to identify when chart frames are about
            // to become active, there is no point looking for charts at this stage
            // since they have no controller
            xFrame->addFrameActionListener(xFrameListener);
        }
        else
        {
#ifdef GROOO_DEBUG
            logMsg("No frame\n");
#endif
        }

#ifdef GROOO_DEBUG
        // this is only for debugging
        css::uno::Reference<css::view::XSelectionSupplier>
            xSelectionSupplier(m_xController, css::uno::UNO_QUERY);
        if (xSelectionSupplier.is())
        {
            css::uno::Reference<css::view::XSelectionChangeListener> xListener(this);
            xSelectionSupplier->addSelectionChangeListener(xListener);
            logMsg("Added Selection listener\n");
        }
#endif
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
     logMsg("notifyContextMenuExecute %lx\n", this);
#endif
    try
    {
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

        static const ::rtl::OUString PROP_COMMAND_URL(RTL_CONSTASCII_USTRINGPARAM("CommandURL"));
        css::uno::Any grFeaturesCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.graphiteoptions:contextmenu")));

        css::uno::Reference<css::container::XIndexContainer> xContextMenu(aEvent.ActionTriggerContainer);
        if (xContextMenu.is() && (xContextMenu->getCount() > 0))
        {
            // check that none of the existing items include GraphiteFeatures
            for (int i = xContextMenu->getCount() - 1; i >= 0; i--)
            {
                css::uno::Reference<css::beans::XPropertySet> xItemPropSet(
                    xContextMenu->getByIndex(i), css::uno::UNO_QUERY);
                if (xItemPropSet.is() &&
                    xItemPropSet->getPropertySetInfo()->hasPropertyByName(PROP_COMMAND_URL) &&
                    (xItemPropSet->getPropertyValue(PROP_COMMAND_URL) == grFeaturesCommand))
                {
                    logMsg("Menu already includes GraphiteFeatures\n");
                    css::uno::Reference< css::ui::XContextMenuInterception > interception(m_xController, css::uno::UNO_QUERY);
                    if (interception.is())
                    {
                        interception->releaseContextMenuInterceptor(this);
                    }
                    return css::ui::ContextMenuInterceptorAction_IGNORED;
                }
            }
        }
        css::uno::Reference< css::lang::XMultiServiceFactory > xFactory(xContextMenu, css::uno::UNO_QUERY);
        if (xFactory.is() && xContextMenu.is())
        {
            static const ::rtl::OUString actionTrigger(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTrigger"));
            static const ::rtl::OUString actionTriggerContainer(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTriggerContainer"));
            static const ::rtl::OUString actionTriggerSeparator(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTriggerSeparator"));
            static const ::rtl::OUString PROP_TEXT(RTL_CONSTASCII_USTRINGPARAM("Text"));
            static const ::rtl::OUString PROP_SUBCONTAINER(RTL_CONSTASCII_USTRINGPARAM("SubContainer"));
            css::uno::Reference<css::beans::XPropertySet> xGrSeparator(xFactory.get()->createInstance (actionTriggerSeparator), css::uno::UNO_QUERY);
            css::uno::Reference<css::beans::XPropertySet> xGrMenuRoot(xFactory.get()->createInstance (actionTrigger), css::uno::UNO_QUERY);
            css::uno::Reference<css::beans::XPropertySet> xGrSubMenu(xFactory.get()->createInstance (actionTriggerContainer), css::uno::UNO_QUERY);
            css::uno::Any grFeaturesText(
                getResourceString(m_xResource, "GraphiteFeatures.MenuLabel"));
            xGrMenuRoot.get()->setPropertyValue(PROP_TEXT, grFeaturesText);
            xGrMenuRoot.get()->setPropertyValue(PROP_COMMAND_URL, grFeaturesCommand);
            css::uno::Any grSubMenu(xGrSubMenu);
            xGrMenuRoot.get()->setPropertyValue(PROP_SUBCONTAINER, grSubMenu);

            xContextMenu.get()->insertByIndex(xContextMenu.get()->getCount(), css::uno::Any(xGrSeparator));
            xContextMenu.get()->insertByIndex(xContextMenu.get()->getCount(), css::uno::Any(xGrMenuRoot));

            return css::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED;
        }
    }
    catch (css::lang::DisposedException e)
    {
        logMsg("Disposed exception during notifyContextMenuExecute\n");
    }
    catch (css::uno::RuntimeException e)
    {
        logMsg("Runtime exception during notifyContextMenuExecute\n");
    }
    return css::ui::ContextMenuInterceptorAction_IGNORED;
}

void SAL_CALL 
org::sil::graphite::SetupContextMenu::selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    css::uno::Reference<css::view::XSelectionSupplier>
        xSupplier(aEvent.Source, css::uno::UNO_QUERY);
    ::rtl::OString aTypeName;
    if (xSupplier.is())
    {
        css::uno::Any aSelection = xSupplier->getSelection();
        if (!aSelection.hasValue()) return;

        css::uno::Reference<css::lang::XMultiServiceFactory>xFactory(m_xContext->getServiceManager(), css::uno::UNO_QUERY);
        if (xFactory.is())
        {
            logMsg("Have factory\n");
            css::uno::Reference<css::reflection::XIdlReflection> xReflection(
            xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.reflection.CoreReflection")), css::uno::UNO_QUERY);
            if (xReflection.is())
            {
                ::rtl::OString typeName;
                xReflection->getType(aSelection)->getName().convertToString(&typeName, RTL_TEXTENCODING_UTF8, 128);
                logMsg("Selection type=%s\n", typeName.getStr());
            }
        }

        aSelection.getValueTypeName().convertToString(&aTypeName, RTL_TEXTENCODING_UTF8, 128);
        logMsg("selectionChanged %s\n", aTypeName.getStr());
        if (aSelection.has<css::uno::Reference<css::uno::XInterface> >())
        {
            css::uno::Reference<css::uno::XInterface>
                xSelectionInterface(aSelection.get<css::uno::Reference<css::uno::XInterface> >());
            css::uno::Reference<css::drawing::XShapes> xShapes(xSelectionInterface, css::uno::UNO_QUERY);
            css::uno::Reference<css::drawing::XShape> xShape(xSelectionInterface, css::uno::UNO_QUERY);
            css::uno::Reference<css::container::XIndexAccess >
                xContainer(xSelectionInterface, css::uno::UNO_QUERY);

            printServiceNames(xSelectionInterface);
            if (xShapes.is())
            {
                logMsg("have %d shapes\n", xShapes->getCount());
                if (xShapes->getCount() > 0)
                {
                    css::uno::Reference<css::beans::XPropertySet> xChartProps;
                    xChartProps.set(xShapes->getByIndex(0), css::uno::UNO_QUERY);
                    xShape.set(xShapes->getByIndex(0), css::uno::UNO_QUERY);
                    ::rtl::OUString modelProp = ::rtl::OUString::createFromAscii("Model");
                    if (xShape.is() && xChartProps.is() && 
                        xChartProps->getPropertySetInfo()->hasPropertyByName(modelProp))
                    {
                        css::uno::Any aModel = xChartProps->getPropertyValue(modelProp);
                        if (aModel.has<css::uno::Reference<css::frame::XModel> >())
                        {
                            css::uno::Reference<css::frame::XModel> xChartModel =
                                aModel.get<css::uno::Reference<css::frame::XModel> >();
                            css::uno::Reference<css::frame::XController> xChartController(xChartModel->getCurrentController(), css::uno::UNO_QUERY);

                            logMsg("have chart with controller %d\n",
                                    xChartController.is());
                        }
                    }
                }
            }
            if (xShape.is())
            {
                logMsg("have a shape\n");
            }
            if (xContainer.is())
            {
                logMsg("have container\n");
            }
        }
    }
#endif
}

void SAL_CALL
org::sil::graphite::SetupContextMenu::frameAction( const ::com::sun::star::frame::FrameActionEvent& action ) throw (::com::sun::star::uno::RuntimeException)
{
#ifdef GROOO_ODEBUG
	logMsg("frameAction %d\n", action.Action);
#endif
    css::uno::Reference< css::frame::XFramesSupplier>
                xFrameSupplier(action.Frame,
                               css::uno::UNO_QUERY);
    if (action.Action == css::frame::FrameAction_FRAME_UI_DEACTIVATING)
    {
        if (xFrameSupplier.is())
        {
            css::uno::Reference< css::frame::XFrames > xFrames = xFrameSupplier->getFrames();
            logMsg("Frame supplier has %d frames\n", xFrames->getCount());
            if (xFrames->getCount() > 0)
            {
                css::uno::Any aFrame = xFrames->getByIndex(0);
                if (aFrame.has<css::uno::Reference< css::frame::XFrame> >())
                {
                    css::uno::Reference< css::frame::XFrame > xSubFrame =
                        aFrame.get<css::uno::Reference< css::frame::XFrame> >();
                    css::uno::Reference< css::frame::XController > xSubController =
                        xSubFrame->getController();
                    if (xSubController.is())
                    {
                        logMsg("Sub frame has controller\n");
                        printServiceNames(xSubController);
                        addMenuInterceptionToModel(xSubController->getModel());
                    }
                    css::uno::Reference< css::frame::XFrameActionListener> xFrameListener(
                        this);
                    xSubFrame->addFrameActionListener(xFrameListener);
                }
            }
        }
    }
    if (action.Action == css::frame::FrameAction_FRAME_UI_ACTIVATED)
    {
        m_xController.set(action.Frame->getController());
    }
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


