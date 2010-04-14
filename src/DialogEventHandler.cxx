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

#include "graphiteooo.hxx"
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

    static const ::rtl::OUString ENABLE_GRAPHITE_EVENT;
    static const ::rtl::OUString EXTERNAL_EVENT;
    static const ::rtl::OUString ENABLE_GRAPHITE_CHECKBOX;
    // destructor is private and will be called indirectly by the release call    virtual ~org::sil::graphite::DialogEventHandler() {}
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_EVENT(RTL_CONSTASCII_USTRINGPARAM("enableGraphiteChanged"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_CHECKBOX(RTL_CONSTASCII_USTRINGPARAM("EnableGraphiteCheckBox"));


org::sil::graphite::DialogEventHandler::DialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
    printf("DialogEventHandler constructor\n");
}

// ::com::sun::star::awt::XContainerWindowEventHandler:
::sal_Bool SAL_CALL org::sil::graphite::DialogEventHandler::callHandlerMethod(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException)
{
    rtl::OString aMethodName(128);
    MethodName.convertToString(&aMethodName, RTL_TEXTENCODING_UTF8, 128);
    printf("callHandlerMethod(%s)\n", aMethodName.getStr());
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

    if (eventObject.has< ::rtl::OUString >() )
    {
        ::rtl::OUString eventValue = eventObject.get< ::rtl::OUString >();
        css::uno::Reference< css::awt::XCheckBox > xCheckBox = getGraphiteEnabledCheckBox(xWindow);
        if (!xCheckBox.is())
        {
            printf("Failed to get graphite enabled checkbox");
            return sal_False;
        }
        if (eventValue.equalsAscii("ok"))
        {
            printf("Graphite enable checkbox %d\n", xCheckBox.get()->getState());
            if (xCheckBox.get()->getState())
            {
                unsetenv(SAL_DISABLE_GRAPHITE);
            }
            else
            {
                if (setenv(SAL_DISABLE_GRAPHITE, "1", 1))
                    printf("Failed to set %s\n", SAL_DISABLE_GRAPHITE);
            }
            return sal_True;
        }
        else if (eventValue.equalsAscii("back") || eventValue.equalsAscii("initialize"))
        {
            const char * pDisableGraphiteStr = getenv(SAL_DISABLE_GRAPHITE);
            if (pDisableGraphiteStr == NULL || pDisableGraphiteStr[0]=='0')
                xCheckBox.get()->setState(1); // is the state a com.sun.star.util.TriState?
            else
                xCheckBox.get()->setState(0);
            return sal_True;
        }
        printf("DialogEventHandler::externalEvent Unexpected\n");
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

css::uno::Sequence< ::rtl::OUString > SAL_CALL org::sil::graphite::DialogEventHandler::getSupportedMethodNames() throw (css::uno::RuntimeException)
{
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    css::uno::Sequence< ::rtl::OUString > methodNames(2);
    methodNames[0] = ENABLE_GRAPHITE_EVENT;
    methodNames[1] = EXTERNAL_EVENT;
    printf("DialogEventHandler::getSupportedMethodNames\n");
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
    printf("DialogEventHandler _getSupportedServiceNames\n");
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
