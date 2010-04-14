/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: org::sil::graphite::SetEnvJob.cxx,v $
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
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/task/XJob.hpp"

#include "graphiteooo.hxx"
#include "SetEnvJob.hxx"

namespace css = ::com::sun::star;

namespace org { namespace sil { namespace graphite { class SetEnvJob; }}}

class org::sil::graphite::SetEnvJob:
    public ::cppu::WeakImplHelper1<
        css::task::XJob>
{
public:
    explicit SetEnvJob(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::task::XJob:
    virtual ::com::sun::star::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue > & Arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception);

private:
    SetEnvJob(const org::sil::graphite::SetEnvJob &); // not defined
    SetEnvJob& operator=(const org::sil::graphite::SetEnvJob &); // not defined

    // destructor is private and will be called indirectly by the release call    virtual ~org::sil::graphite::SetEnvJob() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

org::sil::graphite::SetEnvJob::SetEnvJob(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
    printf("SetEnvJob constructor\n");
}

// ::com::sun::star::task::XJob:
::com::sun::star::uno::Any SAL_CALL org::sil::graphite::SetEnvJob::execute(const css::uno::Sequence< css::beans::NamedValue > & Arguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::uno::Exception)
{
    // TODO: Exchange the default return implementation for "execute" !!!
    // Exchange the default return implementation.
    printf("SetEnvJob execute\n");
    unsetenv(SAL_DISABLE_GRAPHITE);
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return ::com::sun::star::uno::Any();
}

// component helper namespace
namespace org { namespace sil { namespace graphite { namespace setenvjob {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org::sil::graphite::SetEnvJob"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    printf("SetEnvJob _getSupportedServiceNames\n");
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.SetEnvJob"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::SetEnvJob(context));
}

}}}} // closing component helper namespace


