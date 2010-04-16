/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: org::sil::graphite::DialogEventHandler.cxx,v $
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
#include <cstdlib>
#include <cassert>

#include "sal/typesizes.h"
#include "sal/config.h"
//#include "cppuhelper/bootstrap.hxx" //?
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/awt/XControlContainer.hpp"
#include "com/sun/star/awt/XContainerWindowEventHandler.hpp"
#include "com/sun/star/awt/XCheckBox.hpp"
#include "com/sun/star/awt/XFixedText.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/MessageBoxButtons.hpp"
#include "com/sun/star/awt/XMessageBox.hpp"
#include "com/sun/star/awt/XMessageBoxFactory.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"

#include "graphiteooo.hxx"
#include "GraphiteConfiguration.hxx"
#ifdef SAL_UNX
#include "UnixEnvironmentSetter.hxx"
#endif
#include "DialogEventHandler.hxx"

namespace org { namespace sil { namespace graphite { class DialogEventHandler; }}}

namespace css = ::com::sun::star;

class org::sil::graphite::DialogEventHandler:
    public ::cppu::WeakImplHelper1<
        css::awt::XContainerWindowEventHandler>
{
public:
    explicit DialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::awt::XContainerWindowEventHandler:
    virtual ::sal_Bool SAL_CALL callHandlerMethod(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedMethodNames() throw (css::uno::RuntimeException);

private:
    DialogEventHandler(const org::sil::graphite::DialogEventHandler &); // not defined
    DialogEventHandler& operator=(const org::sil::graphite::DialogEventHandler &); // not defined
    ::sal_Bool externalEvent(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & EventObject);

    css::uno::Reference< css::awt::XCheckBox > getGraphiteEnabledCheckBox(const css::uno::Reference< css::awt::XWindow > & xWindow);
    css::uno::Reference< css::awt::XFixedText > getLabel(const css::uno::Reference< css::awt::XWindow > & xWindow, ::rtl::OUString id);
    void showMessage(const css::uno::Reference< css::awt::XWindowPeer > & xWindowPeer, ::rtl::OUString title, ::rtl::OUString message);

    static const ::rtl::OUString ENABLE_GRAPHITE_EVENT;
    static const ::rtl::OUString EXTERNAL_EVENT;
    static const ::rtl::OUString ENABLE_GRAPHITE_CHECKBOX;
    static const ::rtl::OUString GRAPHITE_ENABLED_LABEL;
    static const ::rtl::OUString GRAPHITE_DISABLED_LABEL;
    // destructor is private and will be called indirectly by the release call    virtual ~org::sil::graphite::DialogEventHandler() {}
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    GraphiteConfiguration m_config;
};

const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_EVENT(RTL_CONSTASCII_USTRINGPARAM("enableGraphiteChanged"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_CHECKBOX(RTL_CONSTASCII_USTRINGPARAM("EnableGraphiteCheckBox"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::GRAPHITE_ENABLED_LABEL(RTL_CONSTASCII_USTRINGPARAM("GraphiteEnabledLabel"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::GRAPHITE_DISABLED_LABEL(RTL_CONSTASCII_USTRINGPARAM("GraphietDisabledLabel"));


org::sil::graphite::DialogEventHandler::DialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_config(context)
{
#ifdef GROOO_DEBUG
    printf("DialogEventHandler constructor\n");
#endif
}

// ::com::sun::star::awt::XContainerWindowEventHandler:
::sal_Bool SAL_CALL org::sil::graphite::DialogEventHandler::callHandlerMethod(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException)
{
#ifdef GROOO_DEBUG
    rtl::OString aMethodName(128);
    MethodName.convertToString(&aMethodName, RTL_TEXTENCODING_UTF8, 128);
    printf("callHandlerMethod(%s)\n", aMethodName.getStr());
#endif
    if (MethodName.compareTo(EXTERNAL_EVENT) == 0)
    {
        return externalEvent(xWindow, EventObject);
    }
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return sal_False;
}

::sal_Bool org::sil::graphite::DialogEventHandler::externalEvent(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & eventObject)
{
    static const ::rtl::OUString GRAPHITE_ENABLED(RTL_CONSTASCII_USTRINGPARAM("graphiteEnabled"));
    if (eventObject.has< ::rtl::OUString >() )
    {
        ::rtl::OUString eventValue = eventObject.get< ::rtl::OUString >();
        css::uno::Reference< css::awt::XCheckBox > xCheckBox = getGraphiteEnabledCheckBox(xWindow);
        css::uno::Reference< css::awt::XControl > xControl(xCheckBox, css::uno::UNO_QUERY);
        css::uno::Reference< css::awt::XWindowPeer > xWindowPeer(xControl.get()->getPeer());
        if (!xCheckBox.is())
        {
#ifdef GROOO_DEBUG
            printf("Failed to get graphite enabled checkbox");
#endif
            return sal_False;
        }
        assert(m_config.nameAccess().is());
        css::uno::Any graphiteOptions = m_config.nameAccess().get()->
            getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphite")));
        css::uno::Reference< css::beans::XPropertySet > graphitePropertySet(graphiteOptions, css::uno::UNO_QUERY);

        sal_Bool environmentGraphiteEnabled = sal_False;
        const char * pDisableGraphiteStr = getenv(SAL_DISABLE_GRAPHITE);
        if (pDisableGraphiteStr == NULL || pDisableGraphiteStr[0]=='0')
            environmentGraphiteEnabled = sal_True;

        if (eventValue.equalsAscii("ok"))
        {
#ifdef GROOO_DEBUG
            printf("Graphite enable checkbox %d\n", xCheckBox.get()->getState());
#endif

            if (xCheckBox.get()->getState())
            {
                graphitePropertySet.get()->setPropertyValue(GRAPHITE_ENABLED, css::uno::Any(sal_True));
#ifdef SAL_UNX
                UnixEnvironmentSetter::parseFile(UnixEnvironmentSetter::BASHRC, SAL_DISABLE_GRAPHITE, "0");
#endif
            }
            else
            {
                graphitePropertySet.get()->setPropertyValue(GRAPHITE_ENABLED, css::uno::Any(sal_False));
#ifdef SAL_UNX
                UnixEnvironmentSetter::parseFile(UnixEnvironmentSetter::BASHRC, SAL_DISABLE_GRAPHITE, "1");
#endif
            }
            css::uno::Reference< css::util::XChangesBatch > batch(m_config.nameAccess(), css::uno::UNO_QUERY);
            if (batch.is())
                batch.get()->commitChanges();
#ifdef GROOO_DEBUG
            else fprintf(stderr, "%s Failed to get XChangesBatch", __FUNCTION__);
#endif
            if (environmentGraphiteEnabled != xCheckBox.get()->getState())
            {
                // TODO localization
                ::rtl::OUString title(RTL_CONSTASCII_USTRINGPARAM("Enable/Disable Graphite"));
#ifdef SAL_UNX
                ::rtl::OUString msg(RTL_CONSTASCII_USTRINGPARAM("The change will take affect after you have logged out and back in again."));
#else
                ::rtl::OUString msg(RTL_CONSTASCII_USTRINGPARAM("The change will take affect the next time you restart OpenOffice."));
#endif
                showMessage(xWindowPeer, title, msg);
            }
            return sal_True;
        }
        else if (eventValue.equalsAscii("back") || eventValue.equalsAscii("initialize"))
        {
            css::uno::Reference< css::awt::XWindow> enabledLabel(getLabel(xWindow, GRAPHITE_ENABLED_LABEL), css::uno::UNO_QUERY);
            css::uno::Reference< css::awt::XWindow> disabledLabel(getLabel(xWindow, GRAPHITE_DISABLED_LABEL), css::uno::UNO_QUERY);
            if (environmentGraphiteEnabled)
            {
                enabledLabel.get()->setVisible(sal_True);
                disabledLabel.get()->setVisible(sal_False);
            }
            else
            {
                enabledLabel.get()->setVisible(sal_False);
                disabledLabel.get()->setVisible(sal_True);
            }
            css::uno::Any graphiteEnabled;
            if (graphitePropertySet.is()) 
                graphiteEnabled = graphitePropertySet.get()->getPropertyValue(GRAPHITE_ENABLED);
            if (graphiteEnabled.has< sal_Bool >())
            {
                if (graphiteEnabled.get< sal_Bool >())
                    xCheckBox.get()->setState(1);
                else
                    xCheckBox.get()->setState(0);
            }
            else // Not set yet, use environment
            {
                if (environmentGraphiteEnabled)
                    xCheckBox.get()->setState(1); // is the state a com.sun.star.util.TriState?
                else
                    xCheckBox.get()->setState(0);
            }
            return sal_True;
        }
#ifdef GROOO_DEBUG
        printf("DialogEventHandler::externalEvent Unexpected\n");
#endif
    }
    return sal_False;
}

css::uno::Reference< css::awt::XCheckBox >
org::sil::graphite::DialogEventHandler::getGraphiteEnabledCheckBox(const css::uno::Reference< css::awt::XWindow > & xWindow)
{
    css::uno::Reference< css::awt::XControlContainer > xControlContainer(xWindow, css::uno::UNO_QUERY);
    if (xControlContainer.is() == sal_False)
        return css::uno::Reference< css::awt::XCheckBox>(NULL);
    css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(ENABLE_GRAPHITE_CHECKBOX);
    css::uno::Reference< css::awt::XCheckBox > xCheckBox(xControl, css::uno::UNO_QUERY);
    return xCheckBox;
}

css::uno::Reference< css::awt::XFixedText >
org::sil::graphite::DialogEventHandler::getLabel(const css::uno::Reference< css::awt::XWindow > & xWindow, ::rtl::OUString id)
{
    css::uno::Reference< css::awt::XControlContainer > xControlContainer(xWindow, css::uno::UNO_QUERY);
    if (xControlContainer.is() == sal_False)
        return css::uno::Reference< css::awt::XFixedText>(NULL);
    css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(id);
    css::uno::Reference< css::awt::XFixedText > xFixedText(xControl, css::uno::UNO_QUERY);
    return xFixedText;
}

void org::sil::graphite::DialogEventHandler::showMessage(const css::uno::Reference< css::awt::XWindowPeer > & xWindowPeer, ::rtl::OUString title, ::rtl::OUString msg)
{
    css::uno::Reference< css::awt::XMessageBoxFactory> messageBoxFactory (
        m_xContext.get()->getServiceManager().get()->
        createInstanceWithContext(::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit")),
        m_xContext), css::uno::UNO_QUERY);
    if (messageBoxFactory.is())
    {
        css::awt::Rectangle rect;
        const ::rtl::OUString msgType(RTL_CONSTASCII_USTRINGPARAM("infoBox"));
        css::uno::Reference< css::awt::XMessageBox > xMsgBox(messageBoxFactory.get()->createMessageBox(xWindowPeer, rect, msgType,
                                                                         css::awt::MessageBoxButtons::BUTTONS_OK, title, msg));
        if (xMsgBox.is())
            xMsgBox.get()->execute();
    }
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL org::sil::graphite::DialogEventHandler::getSupportedMethodNames() throw (css::uno::RuntimeException)
{
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    css::uno::Sequence< ::rtl::OUString > methodNames(2);
    methodNames[0] = ENABLE_GRAPHITE_EVENT;
    methodNames[1] = EXTERNAL_EVENT;
#ifdef GROOO_DEBUG
    printf("DialogEventHandler::getSupportedMethodNames\n");
#endif
    return methodNames;
}

// component helper namespace
namespace org { namespace sil { namespace graphite { namespace dialogeventhandler {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org::sil::graphite::DialogEventHandler"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
#ifdef GROOO_DEBUG
    printf("DialogEventHandler _getSupportedServiceNames\n");
#endif
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.DialogEventHandler"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::DialogEventHandler(context));
}

}}}} // closing component helper namespace
