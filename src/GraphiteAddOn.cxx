/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: org::sil::graphite::GraphiteAddOn.cxx,v $
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

#ifdef __GNUC__
#undef _MSC_VER
#endif

#include <cstdio>

#include "sal/typesizes.h"
#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "cppu/macros.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase4.hxx"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XFrame.hpp"

#include "GraphiteAddOn.hxx"

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

    // destructor is private and will be called indirectly by the release call    virtual ~org::sil::graphite::GraphiteAddOn() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
};

org::sil::graphite::GraphiteAddOn::GraphiteAddOn(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
    printf("GraphiteAddOn constructor\n");
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL org::sil::graphite::GraphiteAddOn::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException)
{
    printf("GraphiteAddOn::initialize\n");
    css::uno::Reference < css::frame::XFrame > xFrame;
    if ( aArguments.getLength() ) {
        aArguments[0] >>= xFrame;
        m_xFrame = xFrame;
    }
}

// ::com::sun::star::frame::XDispatch:
void SAL_CALL org::sil::graphite::GraphiteAddOn::dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) throw(css::uno::RuntimeException)
{
    if ( aURL.Protocol.equalsAscii("org.sil.graphite.graphiteoptions:") )
    {
		rtl::OString pathString(32);
		aURL.Path.convertToString(&pathString, RTL_TEXTENCODING_UTF8, 32);
        printf("Command=%s\n", pathString.getStr());
        if ( aURL.Path.equalsAscii("GraphiteOptionCommand") )
        {
                // add your own code here
                return;
        }
        if ( aURL.Path.equalsAscii("ApplicationEventHandler") )
        {
            
        }
    }
}

void SAL_CALL org::sil::graphite::GraphiteAddOn::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) throw (css::uno::RuntimeException)
{
    // add your own code here
}

void SAL_CALL org::sil::graphite::GraphiteAddOn::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) throw (css::uno::RuntimeException)
{
    // add your own code here
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL org::sil::graphite::GraphiteAddOn::getImplementationName() throw (css::uno::RuntimeException)
{
    return org::sil::graphite::graphiteaddon::_getImplementationName();
}

::sal_Bool SAL_CALL org::sil::graphite::GraphiteAddOn::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    rtl::OString serviceNameString(32);
    serviceName.convertToString(&serviceNameString, RTL_TEXTENCODING_UTF8, 32);
    printf("Command=%s\n", serviceNameString.getStr());
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
css::uno::Reference< css::frame::XDispatch > SAL_CALL org::sil::graphite::GraphiteAddOn::queryDispatch( const css::util::URL& aURL, const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags ) throw(css::uno::RuntimeException)
{
    ::rtl::OString utfPath(128);
    ::rtl::OString utfProtocol(128);
    aURL.Path.convertToString(&utfPath, RTL_TEXTENCODING_UTF8, 32);
    aURL.Protocol.convertToString(&utfProtocol, RTL_TEXTENCODING_UTF8, 32);
    printf("GraphiteAddOn::queryDispatch %s %s frame %d\n", utfProtocol.getStr(), utfPath.getStr(), m_xFrame.is());
    css::uno::Reference< css::frame::XDispatch > xRet;
    if ( !m_xFrame.is() )
        return 0;

    if ( aURL.Protocol.equalsAscii("org.sil.graphite.graphiteoptions:") )
    {
        if ( aURL.Path.equalsAscii("GraphiteOptionCommand") )
        {
            printf("returning dispatch\n");
            xRet = this;
        }
    }
    return xRet;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL org::sil::graphite::GraphiteAddOn::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescripts ) throw(css::uno::RuntimeException)
{
    sal_Int32 nCount = seqDescripts.getLength();
    printf("GraphiteAddOn::queryDispatches %ld\n", nCount);
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
    printf("GraphiteAddOn _getSupportedServiceNames\n");
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
