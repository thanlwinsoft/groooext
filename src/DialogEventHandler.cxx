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
#include <cstdlib>
#include <cassert>

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
#include "com/sun/star/awt/XListBox.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"

#include "groooDebug.hxx"
#include "graphiteooo.hxx"
#include "GraphiteConfiguration.hxx"

#ifdef SAL_UNX
#include "UnixEnvironmentSetter.hxx"
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include "DialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"

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
    css::uno::Reference< css::awt::XListBox > setupGraphiteFontList(const css::uno::Reference< css::awt::XWindow > & xWindow);
    void showMessage(const css::uno::Reference< css::awt::XWindowPeer > & xWindowPeer, ::rtl::OUString title, ::rtl::OUString message);

    static const ::rtl::OUString ENABLE_GRAPHITE_EVENT;
    static const ::rtl::OUString EXTERNAL_EVENT;
    static const ::rtl::OUString ENABLE_GRAPHITE_CHECKBOX;
    static const ::rtl::OUString GRAPHITE_ENABLED_LABEL;
    static const ::rtl::OUString GRAPHITE_FONT_LIST;
    // destructor is private and will be called indirectly by the release call
    virtual ~DialogEventHandler() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    GraphiteConfiguration m_config;
    css::uno::Reference< css::resource::XStringResourceResolver> m_xResource;
};

const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_EVENT(RTL_CONSTASCII_USTRINGPARAM("enableGraphiteChanged"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::ENABLE_GRAPHITE_CHECKBOX(RTL_CONSTASCII_USTRINGPARAM("EnableGraphiteCheckBox"));
const ::rtl::OUString org::sil::graphite::DialogEventHandler::GRAPHITE_ENABLED_LABEL(RTL_CONSTASCII_USTRINGPARAM("GraphiteEnabledLabel"));

const ::rtl::OUString org::sil::graphite::DialogEventHandler::GRAPHITE_FONT_LIST(RTL_CONSTASCII_USTRINGPARAM("GraphiteFontListBox"));


org::sil::graphite::DialogEventHandler::DialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_config(context),
    m_xResource(getResource(context, ::rtl::OUString::createFromAscii("GraphiteMessages")),
               css::uno::UNO_QUERY)
{
#ifdef GROOO_DEBUG
    logMsg("DialogEventHandler constructor with resource %d\n",
           m_xResource.is());
#endif
}

// ::com::sun::star::awt::XContainerWindowEventHandler:
::sal_Bool SAL_CALL org::sil::graphite::DialogEventHandler::callHandlerMethod(const css::uno::Reference< css::awt::XWindow > & xWindow, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException)
{
#ifdef GROOO_DEBUG
    rtl::OString aMethodName;
    MethodName.convertToString(&aMethodName, RTL_TEXTENCODING_UTF8, 128);
    logMsg("callHandlerMethod(%s)\n", aMethodName.getStr());
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
            logMsg("Failed to get graphite enabled checkbox");
#endif
            return sal_False;
        }
        assert(m_config.nameAccess().is());
        css::uno::Any graphiteOptions = m_config.nameAccess().get()->
            getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphite")));
        css::uno::Reference< css::beans::XPropertySet > graphitePropertySet(graphiteOptions, css::uno::UNO_QUERY);

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
		bool updatedEnvVariable = false;
        if (eventValue.equalsAscii("ok"))
        {
#ifdef GROOO_DEBUG
            logMsg("Graphite enable checkbox %d\n", xCheckBox.get()->getState());
#endif

#ifdef WIN32
			HKEY userEnvKey = NULL;
			LSTATUS status = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_SET_VALUE, &userEnvKey);
#ifdef GROOO_DEBUG
			if (status != ERROR_SUCCESS)
			{
				logMsg("Failed to find HKCU Environment key\n");
			}
#endif
#endif
            if (xCheckBox.get()->getState())
            {
                graphitePropertySet.get()->setPropertyValue(GRAPHITE_ENABLED, css::uno::Any(sal_True));
#ifdef SAL_UNX
                updatedEnvVariable = UnixEnvironmentSetter::parseFile(UnixEnvironmentSetter::defaultProfile(), SAL_DISABLE_GRAPHITE, "0");
#endif
#ifdef WIN32
				if (status == ERROR_SUCCESS)
				{
					status = RegSetValueExA(userEnvKey, SAL_DISABLE_GRAPHITE, 0,
						REG_EXPAND_SZ, reinterpret_cast<const BYTE*>("0"), 2);
					updatedEnvVariable = (status == ERROR_SUCCESS);
				}
#endif
            }
            else
            {
                graphitePropertySet.get()->setPropertyValue(GRAPHITE_ENABLED, css::uno::Any(sal_False));
#ifdef SAL_UNX
                updatedEnvVariable = UnixEnvironmentSetter::parseFile(UnixEnvironmentSetter::defaultProfile(), SAL_DISABLE_GRAPHITE, "1");
#endif
#ifdef WIN32
				if (status == ERROR_SUCCESS)
				{
					status = RegSetValueExA(userEnvKey, SAL_DISABLE_GRAPHITE, 0,
						REG_EXPAND_SZ, reinterpret_cast<const BYTE*>("1"), 2);
					updatedEnvVariable = (status == ERROR_SUCCESS);
				}
#endif
            }
			if (updatedEnvVariable == false)
			{
                ::rtl::OUString title;
                title = getResourceString(m_xResource, "GraphiteOptions.EnableDisableGraphite");
                ::rtl::OUString msg;
                msg = getResourceString(m_xResource, "GraphiteOptions.FailedToSetVariable");
#ifdef GROOO_DEBUG
				logMsg("Failed to set environment variable\n");
#endif
				showMessage(xWindowPeer, title, msg);
				return sal_False;
			}

#ifdef WIN32
			if (userEnvKey)
			{
				RegCloseKey(userEnvKey);
				DWORD dMsgResult = 0;
				LPCTSTR envChangeParam = "Environment";
				SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)envChangeParam, SMTO_NOTIMEOUTIFNOTHUNG, 5000, &dMsgResult);
			}
#endif
            css::uno::Reference< css::util::XChangesBatch > batch(m_config.nameAccess(), css::uno::UNO_QUERY);
            if (batch.is())
                batch.get()->commitChanges();
#ifdef GROOO_DEBUG
            else logMsg("%s Failed to get XChangesBatch", __FUNCTION__);
#endif
            if (environmentGraphiteEnabled != xCheckBox.get()->getState())
            {
                ::rtl::OUString title;
                title = getResourceString(m_xResource, "GraphiteOptions.EnableDisableGraphite");
                ::rtl::OUString msg;
#ifdef SAL_UNX
                msg  = getResourceString(m_xResource, "GraphiteOptions.LogoutLogin");
#else
                msg  = getResourceString(m_xResource, "GraphiteOptions.Restart");
#endif
                showMessage(xWindowPeer, title, msg);
            }
            return sal_True;
        }
        else if (eventValue.equalsAscii("back") || eventValue.equalsAscii("initialize"))
        {
            css::uno::Reference< css::awt::XFixedText> enabledLabel(getLabel(xWindow, GRAPHITE_ENABLED_LABEL));
            if (environmentGraphiteEnabled)
            {
                // label is already correct
            }
            else
            {
                ::rtl::OUString disabledLabel = getResourceString(m_xResource, "GraphiteOptions.CurrentlyDisabled");
                enabledLabel->setText(disabledLabel);
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
            setupGraphiteFontList(xWindow);
            return sal_True;
        }
#ifdef GROOO_DEBUG
        logMsg("DialogEventHandler::externalEvent Unexpected\n");
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

css::uno::Reference< css::awt::XListBox > 
org::sil::graphite::DialogEventHandler::setupGraphiteFontList(const css::uno::Reference< css::awt::XWindow > & xWindow)
{
    css::uno::Reference< css::awt::XControlContainer > xControlContainer(xWindow, css::uno::UNO_QUERY);
    if (xControlContainer.is() == sal_False)
        return css::uno::Reference< css::awt::XListBox>(NULL);
    css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(GRAPHITE_FONT_LIST);
    assert(xControl.is());
    css::uno::Reference< css::awt::XListBox > xListBox(xControl, css::uno::UNO_QUERY);
    assert(xListBox.is());
    css::uno::Reference< css::awt::XView > xView(xControl, css::uno::UNO_QUERY);
    assert(xView.is());
    // unfortunately the view doesn't have the graphics set on it
    // css::uno::Reference< css::awt::XGraphics > xGraphics(xView.get()->getGraphics());
    // assert(xGraphics.is());
    css::uno::Reference< css::awt::XWindowPeer> xWindowPeer(xControl.get()->getPeer());
    assert(xWindowPeer.is());
    css::uno::Reference< css::awt::XToolkit> xToolkit( xWindowPeer.get()->getToolkit());
    assert(xToolkit.is());
    // I'm not sure that the size is very important for our purposes
    css::uno::Reference< css::awt::XDevice > xDevice(xToolkit.get()->createScreenCompatibleDevice(100, 100) );
    assert(xDevice.is());
    if (xDevice.is())
    {
        GraphiteFontInfo & grFontInfo = GraphiteFontInfo::getFontInfo();
        css::uno::Sequence< css::awt::FontDescriptor > fontDescriptors = xDevice.get()->getFontDescriptors();
        css::uno::Sequence< ::rtl::OUString > grFontNames(10);
        sal_Int32 grFontCount = 0;
        for (sal_Int32 i = 0; i < fontDescriptors.getLength(); i++)
        {
            if (grFontInfo.isGraphiteFont(fontDescriptors[i].Name))
            {
#ifdef GROOO_DEBUG
                rtl::OString aFontName;
                rtl::OString aFontStyleName;
                fontDescriptors[i].Name.convertToString(&aFontName, RTL_TEXTENCODING_UTF8, 128);
                fontDescriptors[i].StyleName.convertToString(&aFontStyleName, RTL_TEXTENCODING_UTF8, 128);
                logMsg("%s (%s) has graphite tables\n", aFontName.getStr(), aFontStyleName.getStr() );
#endif
                // ignore duplicate names
                if (grFontCount > 0 && grFontNames[grFontCount-1].equals(fontDescriptors[i].Name))
                    continue;
                grFontCount++;
                if (grFontNames.getLength() < grFontCount)
                {
                    grFontNames.realloc(grFontNames.getLength() + 10);
                }
                grFontNames[grFontCount-1] = fontDescriptors[i].Name;
            }
        }
        grFontNames.realloc(grFontCount);
        xListBox.get()->addItems(grFontNames, 0);
    }
    return xListBox;
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
    logMsg("DialogEventHandler::getSupportedMethodNames\n");
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
    logMsg("DialogEventHandler _getSupportedServiceNames\n");
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
