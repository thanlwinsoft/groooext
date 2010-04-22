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

#include "sal/typesizes.h"
#include "sal/config.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/DispatchResultEvent.hpp"
#include "com/sun/star/frame/DispatchResultState.hpp"
#include "com/sun/star/text/XTextSectionsSupplier.hpp"
#include "com/sun/star/text/XTextViewCursor.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/ui/ContextMenuInterceptorAction.hpp"
#include "com/sun/star/ui/XContextMenuInterception.hpp"
#include "com/sun/star/ui/XContextMenuInterceptor.hpp"
#include "com/sun/star/view/XSelectionSupplier.hpp"

#include "graphiteooo.hxx"
#include "SetupContextMenu.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"

namespace css = ::com::sun::star;

namespace org { namespace sil { namespace graphite { class SetupContextMenu; }}}

class org::sil::graphite::SetupContextMenu:
    public ::cppu::WeakImplHelper2<
        css::task::XJob, css::ui::XContextMenuInterceptor>
{
public:
    explicit SetupContextMenu(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::task::XJob:
    virtual ::com::sun::star::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue > & Arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception);

    // ::com::sun::star::uno::XContextMenuInterceptor
    virtual css::ui::ContextMenuInterceptorAction SAL_CALL notifyContextMenuExecute( const ::com::sun::star::ui::ContextMenuExecuteEvent& aEvent ) throw (css::uno::RuntimeException);
private:
    SetupContextMenu(const org::sil::graphite::SetupContextMenu &); // not defined
    SetupContextMenu& operator=(const org::sil::graphite::SetupContextMenu &); // not defined

    // destructor is private and will be called indirectly by the release call
    virtual ~SetupContextMenu() {}

    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XController > m_xController;
};

org::sil::graphite::SetupContextMenu::SetupContextMenu(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
    printf("SetupContextMenu constructor\n");
}

// ::com::sun::star::task::XJob:
::com::sun::star::uno::Any SAL_CALL org::sil::graphite::SetupContextMenu::execute(const css::uno::Sequence< css::beans::NamedValue > & arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception)
{
    css::uno::Any environment;
    css::uno::Any config;
    css::uno::Any jobConfig;
    fprintf(stderr, "SetupContextMenu execute %ld args\n", arguments.getLength());
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
        fprintf(stderr, "Job has environment\n");
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
        ::rtl::OString asciiEnvType(128);
        ::rtl::OUString environmentType = envType.get< ::rtl::OUString >();
        environmentType.convertToString(&asciiEnvType, RTL_TEXTENCODING_UTF8, 128);
        fprintf(stderr, "Job env type %s\n", asciiEnvType.getStr());
    }
    if (envFrame.hasValue())
        fprintf(stderr, "Job Environment has frame\n");
    if (envModel.hasValue())
        fprintf(stderr, "Job Environment has model\n");
#endif
    css::uno::Reference< css::frame::XModel > xModel;
    if (envModel.hasValue())
    {
        xModel.set(envModel.get< css::uno::Reference< css::frame::XModel > >());
        m_xController.set(xModel->getCurrentController());
        if (m_xController.is())
        {
            css::uno::Reference< css::ui::XContextMenuInterception > interception(m_xController, css::uno::UNO_QUERY);
            if (interception.is())
            {
#ifdef GROOO_DEBUG
                fprintf(stderr, "have menu interception from Controller\n");
#endif
                interception.get()->registerContextMenuInterceptor(this);
            }
        }
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

css::ui::ContextMenuInterceptorAction SAL_CALL
org::sil::graphite::SetupContextMenu::notifyContextMenuExecute( const ::com::sun::star::ui::ContextMenuExecuteEvent& aEvent ) throw (css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
     fprintf(stderr, "notifyContextMenuExecute\n");
#endif
    // Is there a relevant useful selection?
    css::uno::Reference<css::view::XSelectionSupplier> xSelection(aEvent.Selection);
    if (xSelection.is())
    {
        css::uno::Any selection = xSelection.get()->getSelection();
#ifdef GROOO_DEBUG
        ::rtl::OString aTypeName(128);
        selection.getValueTypeName().convertToString(&aTypeName, RTL_TEXTENCODING_UTF8, 128);
        fprintf(stderr, "Selection type: %s\n", aTypeName.getStr());
#endif

        css::uno::Reference<css::text::XTextViewCursorSupplier> xTextCursorSupplier;
        css::uno::Reference<css::text::XTextViewCursor> xTextCursor;
        if (m_xController.is())
        {
            fprintf(stderr, "Have controller\n");
            xTextCursorSupplier.set(m_xController, css::uno::UNO_QUERY);
            if (xTextCursorSupplier.is())
                xTextCursor.set(xTextCursorSupplier->getViewCursor());
        }
        bool haveGraphiteFont = false;
        if (xTextCursor.is())
        {
            fprintf(stderr, "Have text cursor\n");
            css::uno::Reference< css::beans::XPropertySet> xTextProperties(xTextCursor, css::uno::UNO_QUERY);
            for (int i=0; i < FeatureDialogEventHandler::NUM_SCRIPTS; i++)
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
        else
        {
            fprintf(stderr, "Have no cursor\n");
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
            //const ::rtl::OUString actionTriggerSeparator(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.ActionTriggerSeparator"));
            static const ::rtl::OUString TEXT(RTL_CONSTASCII_USTRINGPARAM("Text"));
            static const ::rtl::OUString COMMAND_URL(RTL_CONSTASCII_USTRINGPARAM("CommandURL"));
            static const ::rtl::OUString SUBCONTAINER(RTL_CONSTASCII_USTRINGPARAM("SubContainer"));
            css::uno::Reference<css::beans::XPropertySet> xGrMenuRoot(xFactory.get()->createInstance (actionTrigger), css::uno::UNO_QUERY);
            css::uno::Reference<css::beans::XPropertySet> xGrSubMenu(xFactory.get()->createInstance (actionTriggerContainer), css::uno::UNO_QUERY);
            css::uno::Any grFeaturesText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphite Features...")));
            xGrMenuRoot.get()->setPropertyValue(TEXT, grFeaturesText);
            css::uno::Any grFeaturesCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.graphiteoptions:contextmenu")));
            xGrMenuRoot.get()->setPropertyValue(COMMAND_URL, grFeaturesCommand);
            css::uno::Any grSubMenu(xGrSubMenu);
            xGrMenuRoot.get()->setPropertyValue(SUBCONTAINER, grSubMenu);

            xContextMenu.get()->insertByIndex(xContextMenu.get()->getCount(), css::uno::Any(xGrMenuRoot));

            return css::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED;
        }
    }
    return css::ui::ContextMenuInterceptorAction_IGNORED;
}

// component helper namespace
namespace org { namespace sil { namespace graphite { namespace setupcontextmenu {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org::sil::graphite::SetupContextMenu"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    printf("SetupContextMenu _getSupportedServiceNames\n");
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


