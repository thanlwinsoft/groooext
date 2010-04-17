/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: org::sil::graphite::FeatureDialogEventHandler.cxx,v $
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
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/awt/XDialogEventHandler.hpp"
#include "com/sun/star/awt/XControlContainer.hpp"
#include "com/sun/star/awt/XControlModel.hpp"
#include "com/sun/star/awt/tree/XTreeControl.hpp"
#include "com/sun/star/awt/tree/XTreeDataModel.hpp"
#include "com/sun/star/awt/tree/XMutableTreeDataModel.hpp"
#include "com/sun/star/awt/tree/XMutableTreeNode.hpp"

#include "FeatureDialogEventHandler.hxx"

namespace css = ::com::sun::star;

namespace osg = ::org::sil::graphite;
const ::rtl::OUString osg::FeatureDialogEventHandler::OK_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_ok"));
const ::rtl::OUString osg::FeatureDialogEventHandler::CANCEL_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_cancel"));
const ::rtl::OUString osg::FeatureDialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString osg::FeatureDialogEventHandler::FOCUS_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_focus"));
const ::rtl::OUString osg::FeatureDialogEventHandler::TREE_CONTROL(RTL_CONSTASCII_USTRINGPARAM("GrFeatureTreeControl"));

org::sil::graphite::FeatureDialogEventHandler::FeatureDialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context, 
                                                                              css::uno::Reference< css::frame::XModel > const & model) :
    m_xContext(context), m_xFactory(context.get()->getServiceManager()), m_xModel(model)
{
    fprintf(stderr, "FeatureDialogEventHandler constructor\n");
}

// ::com::sun::star::awt::XDialogEventHandler:
::sal_Bool SAL_CALL org::sil::graphite::FeatureDialogEventHandler::callHandlerMethod(const css::uno::Reference< css::awt::XDialog > & xDialog, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException)
{
#ifdef GROOO_DEBUG
    rtl::OString aMethodName(128);
    MethodName.convertToString(&aMethodName, RTL_TEXTENCODING_UTF8, 128);
    fprintf(stderr, "FeatureDialogEventHandler callHandlerMethod(%s)\n", aMethodName.getStr());
#endif
    // TODO: Exchange the default return implementation for "callHandlerMethod" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL org::sil::graphite::FeatureDialogEventHandler::getSupportedMethodNames() throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > methodNames(3);
    methodNames[0] = OK_EVENT;
    methodNames[1] = CANCEL_EVENT;
    methodNames[2] = EXTERNAL_EVENT;
    methodNames[3] = FOCUS_EVENT;
#ifdef GROOO_DEBUG
    fprintf(stderr, "FeatureDialogEventHandler::getSupportedMethodNames\n");
#endif
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return methodNames;
}


// ::com::sun::star::lang::XEventListener:
void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::disposing(const css::lang::EventObject & Source) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "disposing" here.
}

// ::com::sun::star::awt::XTopWindowListener:
void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowOpened(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::awt::XControlContainer > xControlContainer(e.Source, css::uno::UNO_QUERY);
    assert(xControlContainer.is());
    css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(TREE_CONTROL);
    css::uno::Reference<css::awt::tree::XTreeControl> xTree(xControl, css::uno::UNO_QUERY);
    css::uno::Reference<css::awt::XControlModel> xControlModel(xControl.get()->getModel());
    //css::uno::Reference<css::awt::tree::XTreeDataModel> xDataModel;
    css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel;
    if (xControlModel.is())
    {
        xMutableDataModel.set(xControl.get()->getModel(), css::uno::UNO_QUERY);
    }
    else
    {
        ::rtl::OUString modelService = ::rtl::OUString::createFromAscii("com.sun.star.awt.tree.MutableTreeDataModel");
        xMutableDataModel.set(m_xFactory.get()->createInstanceWithContext(
            modelService, m_xContext), css::uno::UNO_QUERY);
        xControlModel.set(xMutableDataModel, css::uno::UNO_QUERY);
        xControl.get()->setModel(xControlModel);
    }
    // TODO
    

    //css::uno::Reference< css::beans::XPropertySet> xTreePropSet(xControl, css::uno::UNO_QUERY);
    assert(xTree.is());
    //assert(xTreePropSet.is());
    // TODO: Insert your implementation for "windowOpened" here.
    fprintf(stderr, "FeatureDialogEventHandler::windowOpened\n");
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowClosing(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowClosing" here.
    fprintf(stderr, "FeatureDialogEventHandler::windowClosing\n");
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowClosed(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowClosed" here.
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowMinimized(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowMinimized" here.
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowNormalized(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowNormalized" here.
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowActivated(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowActivated" here.
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowDeactivated(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // TODO: Insert your implementation for "windowDeactivated" here.
}


// component helper namespace
namespace org { namespace sil { namespace graphite { namespace featuredialogeventhandler {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "org::sil::graphite::FeatureDialogEventHandler"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
#ifdef GROOO_DEBUG
    printf("FeatureDialogEventHandler _getSupportedServiceNames\n");
#endif
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.sil.graphite.FeatureDialogEventHandler"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    css::uno::Reference< css::frame::XModel > emptyModel;
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::FeatureDialogEventHandler(context, emptyModel));
}

}}}} // closing component helper namespace

