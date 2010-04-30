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

#ifndef featuredialogeventhandler_hxx
#define featuredialogeventhandler_hxx

#include <map>

#include <sal/config.h>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase4.hxx>
#include "com/sun/star/awt/XDialogEventHandler.hpp"
#include "com/sun/star/awt/XTopWindowListener.hpp"
#include "com/sun/star/awt/XCheckBox.hpp"
#include "com/sun/star/awt/tree/XMutableTreeDataModel.hpp"
#include "com/sun/star/awt/tree/XTreeControl.hpp"
#include "com/sun/star/awt/tree/XTreeEditListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/view/XSelectionChangeListener.hpp"
//#include "com/sun/star
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XController.hpp"


namespace gr { class Font; }

// component helper namespace
namespace org { namespace sil { namespace graphite { namespace featuredialogeventhandler {

namespace css = ::com::sun::star;

// component and service helper functions:
::rtl::OUString SAL_CALL _getImplementationName();
css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

}}}} // closing component helper namespace

namespace org { namespace sil { namespace graphite { 

namespace css = ::com::sun::star;

class FeatureDialogEventHandler:
    public ::cppu::WeakImplHelper4<css::awt::XDialogEventHandler, css::awt::XTopWindowListener, 
        css::awt::tree::XTreeEditListener, css::view::XSelectionChangeListener>
{
public:
    explicit FeatureDialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context, 
                                       css::uno::Reference< css::frame::XModel > const & model, ::rtl::OUString const & location);

    // ::com::sun::star::awt::XDialogEventHandler:
    virtual ::sal_Bool SAL_CALL callHandlerMethod(const css::uno::Reference< css::awt::XDialog > & xDialog, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedMethodNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener:
    virtual void SAL_CALL disposing(const css::lang::EventObject & Source) throw (css::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindowListener:
    virtual void SAL_CALL windowOpened(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowClosing(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowClosed(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowMinimized(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowNormalized(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowActivated(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    virtual void SAL_CALL windowDeactivated(const css::lang::EventObject & e) throw (css::uno::RuntimeException);
    
    // ::com::sun::star::awt::tree::XTreeEditListener
    virtual void SAL_CALL nodeEditing( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::util::VetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL nodeEdited( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node, const ::rtl::OUString& NewText ) throw (::com::sun::star::uno::RuntimeException);
    
    // ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // set the fontnames with the selected features
    void setFontNames(void);


    enum FontScript {
        WESTERN_SCRIPT = 0,
        CTL_SCRIPT = 1,
        ASIAN_SCRIPT = 2,
        NUM_SCRIPTS = 3
    };
    static const ::rtl::OUString FONT_PROPERTY_NAME[NUM_SCRIPTS];

private:
    FeatureDialogEventHandler(const org::sil::graphite::FeatureDialogEventHandler &); // not defined
    FeatureDialogEventHandler& operator=(const org::sil::graphite::FeatureDialogEventHandler &); // not defined

    void setupTreeModel(css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel);
    void addFontFeatures(css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel,
                         css::uno::Reference<css::awt::tree::XMutableTreeNode> rootNode,
                         FontScript fontScript, const ::rtl::OUString & fontName, const ::rtl::OUString & fontDesc);
    void initFeatureMap(FontScript fontScript, const ::rtl::OUString & featureDesc);
    void storeFeatures(void);

    // destructor is private and will be called indirectly by the release call
    virtual ~FeatureDialogEventHandler() {}

    static const ::rtl::OUString OK_EVENT;
    static const ::rtl::OUString CANCEL_EVENT;
    static const ::rtl::OUString DIALOG_FOCUS_EVENT;
    static const ::rtl::OUString EXTERNAL_EVENT;
    static const ::rtl::OUString TREE_CONTROL;
    static const ::rtl::OUString UPDATE_STYLE_CHECKBOX;
    static const ::rtl::OUString ENABLED_ICON;
    static const ::rtl::OUString DISABLED_ICON;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xFactory;
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Reference< css::frame::XController > m_xController;
	css::uno::Reference< css::view::XSelectionSupplier > m_xSelectionSupplier;
    css::uno::Reference<css::awt::tree::XTreeControl> m_xTree;
    css::uno::Reference<css::awt::XCheckBox> m_xUpdateStyle;
    css::uno::Reference< css::beans::XPropertySet> m_xStyleTextProperties;
    ::rtl::OUString m_extensionBase;
    ::gr::Font * m_fonts[NUM_SCRIPTS]; // normal, ctl, asian
    std::map<sal_uInt32, sal_Int32> m_featureSettings[NUM_SCRIPTS];
    ::rtl::OUString m_fontNamesWithFeatures[NUM_SCRIPTS];
};

}}}

#endif
