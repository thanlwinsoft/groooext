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


#ifdef __GNUC__
#undef _MSC_VER
#endif

#include <cstdio>

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "cppu/macros.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase4.hxx"
#include "com/sun/star/awt/XControl.hpp"
#include "com/sun/star/awt/XControlModel.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XDialogProvider2.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/deployment/PackageInformationProvider.hpp"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XLocalizable.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

#include "groooDebug.hxx"
#include "GraphiteAddOn.hxx"
#include "FeatureDialogEventHandler.hxx"

namespace org { namespace sil { namespace graphite { class GraphiteAddOn; } } }


namespace css = ::com::sun::star;

class org::sil::graphite::GraphiteAddOn:
    public ::cppu::WeakImplHelper4<
        css::lang::XInitialization,
        css::frame::XDispatch,
        css::lang::XServiceInfo,
        css::frame::XDispatchProvider>
{
public:
    explicit GraphiteAddOn(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::frame::XDispatch:
    virtual void SAL_CALL dispatch(const css::util::URL & URL, const css::uno::Sequence< css::beans::PropertyValue > & Arguments) throw (css::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & Control, const css::util::URL & URL) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & Control, const css::util::URL & URL) throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::frame::XDispatchProvider:
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(const css::util::URL & URL, const ::rtl::OUString & TargetFrameName, ::sal_Int32 SearchFlags) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor > & Requests) throw (css::uno::RuntimeException);

private:
    GraphiteAddOn(const org::sil::graphite::GraphiteAddOn &); // not defined
    GraphiteAddOn& operator=(const org::sil::graphite::GraphiteAddOn &); // not defined

    // destructor is private and will be called indirectly by the release call
    virtual ~GraphiteAddOn() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xFactory;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::lang::Locale m_locale;
};

org::sil::graphite::GraphiteAddOn::GraphiteAddOn(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_xFactory(context.get()->getServiceManager())
{
#ifdef GROOO_DEBUG
    printf("GraphiteAddOn constructor\n");
#endif
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL org::sil::graphite::GraphiteAddOn::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    printf("GraphiteAddOn::initialize\n");
#endif
    css::uno::Reference < css::frame::XFrame > xFrame;
    if ( aArguments.getLength() ) {
        aArguments[0] >>= xFrame;
        m_xFrame = xFrame;
        css::uno::Reference < css::frame::XController > xController(m_xFrame.get()->getController());
        if (xController.is())
            m_xModel.set(xController.get()->getModel());
    }
}

// ::com::sun::star::frame::XDispatch:
void SAL_CALL org::sil::graphite::GraphiteAddOn::dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& /*aArguments*/ ) throw(css::uno::RuntimeException)
{
    if ( aURL.Protocol.equalsAscii("org.sil.graphite.graphiteoptions:") )
    {
#ifdef GROOO_DEBUG
		rtl::OString pathString(32);
		aURL.Path.convertToString(&pathString, RTL_TEXTENCODING_UTF8, 32);
        printf("Command=%s\n", pathString.getStr());
#endif
        if ( aURL.Path.equalsAscii("contextmenu") ||  aURL.Path.equalsAscii("GraphiteFeatureCommand") )
        {
            try
            {
                css::uno::Reference< css::deployment::XPackageInformationProvider >
                    xInfoProvider( css::deployment::PackageInformationProvider::get( m_xContext) );
                rtl::OUString sLocation = xInfoProvider->getPackageLocation(
                    rtl::OUString::createFromAscii( "org.sil.graphite.GraphiteOptions" ) );
                rtl::OString aLocation;
                sLocation.convertToString(&aLocation, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
#ifdef GROOO_DEBUG
                logMsg("AddOn location %s\n", aLocation.getStr());
#endif
                if (m_xModel.is())
                {
                    css::uno::Sequence<css::uno::Any> args(1);
                    args[0] = css::uno::Any(m_xModel);
                    static const ::rtl::OUString DIALOG_PROVIDER(rtl::OUString::createFromAscii("com.sun.star.awt.DialogProvider2"));
                    css::uno::Reference<css::uno::XInterface> dialogProvider =
                        m_xFactory.get()->createInstanceWithArgumentsAndContext(DIALOG_PROVIDER, args, m_xContext);
                    css::uno::Reference<css::awt::XDialogProvider2> xDialogProvider(dialogProvider, ::css::uno::UNO_QUERY);
                    if (xDialogProvider.is())
                    {
#ifdef GROOO_DEBUG
                        logMsg("Have dialogprovider\n");
#endif
                        ::rtl::OUString dialogPath(RTL_CONSTASCII_USTRINGPARAM("/dialogs/GraphiteFeatures.xdl"));
                        ::rtl::OUString dialogUrl(sLocation + dialogPath);
                        css::uno::Reference<org::sil::graphite::FeatureDialogEventHandler> eventHandler(new org::sil::graphite::FeatureDialogEventHandler(m_xContext, m_xModel, sLocation));
                        css::uno::Reference<css::awt::XDialogEventHandler> xDialogEventHandler(eventHandler, css::uno::UNO_QUERY);
                        css::uno::Reference<css::awt::XDialog> xDialog(
                            xDialogProvider.get()->createDialogWithHandler(dialogUrl, xDialogEventHandler));
                        if (xDialog.is())
                        {
                            css::uno::Reference<css::awt::XControl> xDialogControl(xDialog, css::uno::UNO_QUERY);
                            css::uno::Reference<css::beans::XPropertySet> xPropertySet(xDialogControl.get()->getModel(), css::uno::UNO_QUERY);
                            if (xPropertySet.is())
                            {
                                static const ::rtl::OUString sizeable(RTL_CONSTASCII_USTRINGPARAM("Sizeable"));
                                xPropertySet.get()->setPropertyValue(sizeable, css::uno::Any(sal_True));
                            }
                            css::uno::Reference<css::awt::XTopWindow> xTopWindow(xDialog, css::uno::UNO_QUERY);
                            css::uno::Reference<css::awt::XTopWindowListener> xTopWindowListener(eventHandler, css::uno::UNO_QUERY);
                            if (xTopWindow.is())
                                xTopWindow.get()->addTopWindowListener(xTopWindowListener);
                            short status = xDialog.get()->execute();
#ifdef GROOO_DEBUG
                            logMsg("FeatureDialog returned %d\n", status);
#endif
                            if (status == 1) // OK button pressed
                            {
                                eventHandler.get()->setFontNames();
                            }
                            if (xTopWindow.is())
                                xTopWindow.get()->removeTopWindowListener(xTopWindowListener);
                        }
                    }
                }
            }
            catch (css::uno::RuntimeException e)
            {
                rtl::OString msg;
                e.Message.convertToString(&msg, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
#ifdef GROOO_DEBUG
                logMsg("Exception %s\n", msg.getStr());
#endif
            }
            return;
        }
        if ( aURL.Path.equalsAscii("ApplicationEventHandler") )
        {
            
        }
    }
}

void SAL_CALL org::sil::graphite::GraphiteAddOn::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw (css::uno::RuntimeException)
{
    
}

void SAL_CALL org::sil::graphite::GraphiteAddOn::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xControl*/, const css::util::URL& /*aURL*/ ) throw (css::uno::RuntimeException)
{
    
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL org::sil::graphite::GraphiteAddOn::getImplementationName() throw (css::uno::RuntimeException)
{
    return org::sil::graphite::graphiteaddon::_getImplementationName();
}

::sal_Bool SAL_CALL org::sil::graphite::GraphiteAddOn::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    rtl::OString serviceNameString;
    serviceName.convertToString(&serviceNameString, RTL_TEXTENCODING_UTF8, 32);
    printf("Command=%s\n", serviceNameString.getStr());
#endif
    css::uno::Sequence< ::rtl::OUString > serviceNames = org::sil::graphite::graphiteaddon::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL org::sil::graphite::GraphiteAddOn::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return org::sil::graphite::graphiteaddon::_getSupportedServiceNames();
}

// ::com::sun::star::frame::XDispatchProvider:
css::uno::Reference< css::frame::XDispatch > SAL_CALL org::sil::graphite::GraphiteAddOn::queryDispatch( const css::util::URL& aURL, const ::rtl::OUString& /*sTargetFrameName*/, sal_Int32 /*nSearchFlags*/ ) throw(css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    ::rtl::OString utfPath;
    ::rtl::OString utfProtocol;
    aURL.Path.convertToString(&utfPath, RTL_TEXTENCODING_UTF8, 32);
    aURL.Protocol.convertToString(&utfProtocol, RTL_TEXTENCODING_UTF8, 32);
    printf("GraphiteAddOn::queryDispatch %s %s frame %d\n", utfProtocol.getStr(), utfPath.getStr(), m_xFrame.is());
#endif
    css::uno::Reference< css::frame::XDispatch > xRet;
    if ( !m_xFrame.is() )
        return 0;

    if ( aURL.Protocol.equalsAscii("org.sil.graphite.graphiteoptions:") )
    {
        if ( aURL.Path.equalsAscii("GraphiteFeatureCommand") || aURL.Path.equalsAscii("contextmenu"))
        {
#ifdef GROOO_DEBUG
            printf("returning dispatch\n");
#endif
            xRet = this;
        }
    }
    return xRet;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL org::sil::graphite::GraphiteAddOn::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescripts ) throw(css::uno::RuntimeException)
{
    sal_Int32 nCount = seqDescripts.getLength();
#ifdef GROOO_DEBUG
    printf("GraphiteAddOn::queryDispatches %d\n", nCount);
#endif
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher(nCount);

    for( sal_Int32 i=0; i<nCount; ++i ) {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL,
                                        seqDescripts[i].FrameName,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}


// component helper namespace
namespace org { namespace sil { namespace graphite { namespace graphiteaddon {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org.sil.graphite.GraphiteAddOn"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
#ifdef GROOO_DEBUG
//    printf("GraphiteAddOn _getSupportedServiceNames\n");
#endif
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.frame.ProtocolHandler"));
//    s[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
//        "com.sun.star.lang.XInitialization"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::GraphiteAddOn(context));
}

}}}} // closing component helper namespace
