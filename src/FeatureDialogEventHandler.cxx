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
#include "com/sun/star/uno/XInterface.hpp"
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
#include "com/sun/star/text/XTextViewCursor.hpp"
#include "com/sun/star/text/XTextViewCursorSupplier.hpp"

#include "FeatureDialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"

namespace css = ::com::sun::star;

namespace osg = ::org::sil::graphite;
const ::rtl::OUString osg::FeatureDialogEventHandler::OK_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_ok"));
const ::rtl::OUString osg::FeatureDialogEventHandler::CANCEL_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_cancel"));
const ::rtl::OUString osg::FeatureDialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString osg::FeatureDialogEventHandler::FOCUS_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_focus"));
const ::rtl::OUString osg::FeatureDialogEventHandler::TREE_CONTROL(RTL_CONSTASCII_USTRINGPARAM("GrFeatureTreeControl"));

const ::rtl::OUString osg::FeatureDialogEventHandler::ENABLED_ICON = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/icons/enabled.png"));
const ::rtl::OUString osg::FeatureDialogEventHandler::DISABLED_ICON = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/icons/disabled.png"));

org::sil::graphite::FeatureDialogEventHandler::FeatureDialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context, 
                                                                              css::uno::Reference< css::frame::XModel > const & model,
                                                                            const ::rtl::OUString & location) :
    m_xContext(context), m_xFactory(context.get()->getServiceManager()), m_xModel(model), m_extensionBase(location)
{
    fprintf(stderr, "FeatureDialogEventHandler constructor\n");
    m_fonts[0] = m_fonts[1] = m_fonts[2] = NULL;
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
    for (int i = 0; i < 3; i++)
        if (m_fonts[i])
        {
            delete m_fonts[i];
            m_fonts[i] = NULL;
        }
}

void org::sil::graphite::FeatureDialogEventHandler::addFontFeatures(
    css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel,
    css::uno::Reference<css::awt::tree::XMutableTreeNode> rootNode, 
    const ::rtl::OUString & fontKey, const ::rtl::OUString & fontName, const ::rtl::OUString & fontDesc)
{
    css::uno::Reference<css::awt::tree::XMutableTreeNode> fontNode = xMutableDataModel.get()->createNode(css::uno::Any(fontName + fontDesc), sal_False);
    css::uno::Reference<css::awt::tree::XTreeNode> xFontTreeNode(fontNode, css::uno::UNO_QUERY);
    fontNode.get()->setDataValue(css::uno::Any(fontName));
    ::rtl::OUString defaultLabel = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (Default)"));
    GraphiteFontInfo & grInfo = GraphiteFontInfo::getFontInfo();
    rootNode.get()->appendChild(fontNode);
    m_xTree.get()->makeNodeVisible(xFontTreeNode);
    
    if (grInfo.isGraphiteFont(fontName))
    {
        gr::Font * grFont = grInfo.loadFont(fontName);
        m_fonts[rootNode.get()->getChildCount() - 1] = grFont;
        if (grFont)
        {
            gr::lgid en_US(0x0409);
            std::pair<gr::FeatureIterator, gr::FeatureIterator>iFeats = grFont->getFeatures();
            std::pair<gr::FeatLabelLangIterator, gr::FeatLabelLangIterator > iFeatLangs = grFont->getFeatureLabelLanguages();
            gr::lgid lang = en_US;
            gr::FeatureIterator iFeat = iFeats.first;
            fprintf(stderr, "Loaded font with %d\n", iFeats.second - iFeats.first);
            while (iFeat != iFeats.second)
            {
                gr::utf16 featLabel[128];
                bool hasLabel = grFont->getFeatureLabel(iFeat, en_US, featLabel);
                // if we don't have English, randomly pick the first language for now
                if (!hasLabel && (iFeatLangs.first != iFeatLangs.second))
                {
                    hasLabel = grFont->getFeatureLabel(iFeat, *(iFeatLangs.first), featLabel);
                    lang = *(iFeatLangs.first);
                }
                ::rtl::OUString featName(featLabel);
                if (hasLabel)
                {
                    css::uno::Reference<css::awt::tree::XMutableTreeNode> featNode = xMutableDataModel.get()->createNode(css::uno::Any(featName), sal_False);
                    featNode.get()->setDataValue(css::uno::Any(static_cast<sal_uInt32>(*iFeat)));
                    css::uno::Reference<css::awt::tree::XTreeNode> xFeatTreeNode(featNode, css::uno::UNO_QUERY);
                    fontNode.get()->appendChild(featNode);
                    m_xTree.get()->makeNodeVisible(xFeatTreeNode);

                    std::pair<gr::FeatureSettingIterator, gr::FeatureSettingIterator > iSettings = grFont->getFeatureSettings(iFeat);
                    gr::FeatureSettingIterator iSetting = iSettings.first;
                    gr::FeatureSettingIterator defaultValue = grFont->getDefaultFeatureValue(iFeat);
                    fprintf(stderr, "Feature %d has %d settings\n", iFeat - iFeats.first, iSettings.second - iSettings.first);
                    while (iSetting != iSettings.second)
                    {
                        hasLabel = grFont->getFeatureSettingLabel(iSetting, lang, featLabel);
                        ::rtl::OUString settingName(featLabel);
                        if (*iSetting == *defaultValue)
                            settingName += defaultLabel;
                        css::uno::Reference<css::awt::tree::XMutableTreeNode> settingNode = xMutableDataModel.get()->createNode(css::uno::Any(settingName), sal_False);
                        settingNode.get()->setDataValue(css::uno::Any(static_cast<sal_Int32>(*iSetting)));
                        featNode.get()->appendChild(settingNode);
                        if (*iSetting == *defaultValue)
                            settingNode.get()->setNodeGraphicURL(m_extensionBase + ENABLED_ICON);
                        else
                            settingNode.get()->setNodeGraphicURL(m_extensionBase + DISABLED_ICON);
                        ++iSetting;
                    }
                }
                ++iFeat;
            }
        }
    }
    else
    {
        m_fonts[rootNode.get()->getChildCount() - 1] = NULL;
    }
}

void org::sil::graphite::FeatureDialogEventHandler::setupTreeModel(css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel)
{
    assert(xMutableDataModel.is());
    fprintf(stderr, "FeatureDialogEventHandler::setupTreeModel\n");
    m_xController.set(m_xModel->getCurrentController());
    if (!m_xController.is()) return;
    css::uno::Reference<css::text::XTextViewCursorSupplier> xTextCursorSupplier;
    css::uno::Reference<css::text::XTextViewCursor> xTextCursor;
    xTextCursorSupplier.set(m_xController, css::uno::UNO_QUERY);
    if (xTextCursorSupplier.is())
        xTextCursor.set(xTextCursorSupplier->getViewCursor());
    ::rtl::OUString rootName = ::rtl::OUString::createFromAscii("Graphite Features");
    css::uno::Reference<css::awt::tree::XMutableTreeNode> rootNode = xMutableDataModel.get()->createNode(css::uno::Any(rootName), sal_False);
    xMutableDataModel.get()->setRoot(rootNode);

    if (xTextCursor.is())
    {
        fprintf(stderr, "Have text cursor\n");
        css::uno::Reference< css::beans::XPropertySet> xTextProperties(xTextCursor, css::uno::UNO_QUERY);
        ::rtl::OUString fontNameKey = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontName"));
        css::uno::Any aFontName = xTextProperties.get()->getPropertyValue(fontNameKey);
        addFontFeatures(xMutableDataModel, rootNode, fontNameKey, aFontName.get< ::rtl::OUString>(), ::rtl::OUString());
        ::rtl::OUString complexFontDesc = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (CTL)"));
        ::rtl::OUString fontNameComplexKey = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameComplex"));
        css::uno::Any aFontNameComplex = xTextProperties.get()->getPropertyValue(fontNameComplexKey);
        addFontFeatures(xMutableDataModel, rootNode, fontNameComplexKey, aFontNameComplex.get< ::rtl::OUString>(), complexFontDesc);
        ::rtl::OUString asianFontDesc = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (Asian)"));
         ::rtl::OUString fontNameAsianKey = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameAsian"));
        css::uno::Any aFontNameAsian = xTextProperties.get()->getPropertyValue(fontNameAsianKey);
        addFontFeatures(xMutableDataModel, rootNode, fontNameAsianKey, aFontNameAsian.get< ::rtl::OUString>(), asianFontDesc);
        
        m_xTree.get()->addTreeEditListener(this);
    }
}

// ::com::sun::star::awt::XTopWindowListener:
void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowOpened(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::awt::XControlContainer > xControlContainer(e.Source, css::uno::UNO_QUERY);
    assert(xControlContainer.is());
    css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(TREE_CONTROL);
    m_xTree.set(xControl, css::uno::UNO_QUERY);
    css::uno::Reference< css::view::XSelectionSupplier > xSelectionSupplier(xControl, css::uno::UNO_QUERY);
    if (xSelectionSupplier.is())
        xSelectionSupplier.get()->addSelectionChangeListener(this);
    css::uno::Reference<css::awt::XControlModel> xControlModel(xControl.get()->getModel());
    css::uno::Reference< css::beans::XPropertySet> xModelProperties(xControlModel, css::uno::UNO_QUERY);
    //css::uno::Reference<css::awt::tree::XTreeDataModel> xDataModel;
    css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel;
    if (xControlModel.is() && xModelProperties.is())
    {
        ::rtl::OUString modelService = ::rtl::OUString::createFromAscii("com.sun.star.awt.tree.MutableTreeDataModel");
        xMutableDataModel.set(m_xFactory.get()->createInstanceWithContext(
            modelService, m_xContext), css::uno::UNO_QUERY);
        assert(xMutableDataModel.is());
        ::rtl::OUString dataModel = ::rtl::OUString::createFromAscii("DataModel");
        xModelProperties.get()->setPropertyValue(dataModel, css::uno::Any(xMutableDataModel));
        setupTreeModel(xMutableDataModel);
    }
    // TODO

    //css::uno::Reference< css::beans::XPropertySet> xTreePropSet(xControl, css::uno::UNO_QUERY);
    assert(m_xTree.is());
    //assert(xTreePropSet.is());
    // TODO: Insert your implementation for "windowOpened" here.
    fprintf(stderr, "FeatureDialogEventHandler::windowOpened\n");
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowClosing(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    for (int i = 0; i < 3; i++)
    {
        ::rtl::OUString featBuilder;
        if (m_fonts[i] != NULL && m_featureSettings[i].size() > 0)
        {
            // analyze the feature settings into a revised fontname
            std::map<sal_uInt32, sal_Int32>::iterator iSetFeat = m_featureSettings[i].begin();
            while (iSetFeat != m_featureSettings[i].end())
            {
                std::pair<gr::FeatureIterator, gr::FeatureIterator>iFeats = m_fonts[i]->getFeatures();
                gr::FeatureIterator iFeat = iFeats.first;
                while (iFeat != iFeats.second)
                {
                    if (*iFeat == iSetFeat->first)
                    {
                        break;
                    }
                    ++iFeat;
                }
                // found the feature
                if (iFeat != iFeats.second)
                {
                    gr::FeatureSettingIterator defaultSetting = m_fonts[i]->getDefaultFeatureValue(iFeat);
                    if (*defaultSetting != iSetFeat->second)
                    {
                        // TODO
                        //featBuilder += ;
                    }
                }
                ++iSetFeat;
            }
        }
    }
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

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::nodeEditing( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::util::VetoException, ::com::sun::star::uno::RuntimeException)
{
    fprintf(stderr, "FeatureDialogEventHandler::nodeEditing\n");
}

void SAL_CALL 
org::sil::graphite::FeatureDialogEventHandler::nodeEdited( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node, const ::rtl::OUString& NewText ) throw (::com::sun::star::uno::RuntimeException)
{
    fprintf(stderr, "FeatureDialogEventHandler::nodeEdited\n");
}

void SAL_CALL 
org::sil::graphite::FeatureDialogEventHandler::selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    css::uno::Reference< css::awt::tree::XTreeControl > xTree(aEvent.Source, css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::tree::XMutableTreeNode > settingNode;
    css::uno::Any aSelection = xTree.get()->getSelection();
    if (aSelection.has<css::uno::Reference< css::uno::XInterface> >())
    {
        settingNode.set(aSelection.get<css::uno::Reference< css::uno::XInterface> >(), css::uno::UNO_QUERY);
        if (settingNode.is())
        {
            css::uno::Reference<css::awt::tree::XTreeNode> setting(settingNode, css::uno::UNO_QUERY);
            css::uno::Reference<css::awt::tree::XTreeNode> feature(settingNode.get()->getParent());
            css::uno::Reference<css::awt::tree::XMutableTreeNode> featureNode(feature, css::uno::UNO_QUERY);
            if (feature.is())
            {
                css::uno::Reference<css::awt::tree::XTreeNode> font = feature.get()->getParent();
                if (font.is() && font.get()->getParent().is())
                {
                    // It is at the correct level for a setting node
                    int fontIndex = font.get()->getParent().get()->getIndex(font);
                    // This is a feature setting node
                    fprintf(stderr, "FeatureDialogEventHandler::selectionChanged font %d node %d\n", fontIndex, settingNode.is());
                    settingNode.get()->setNodeGraphicURL(m_extensionBase + ENABLED_ICON);
                    m_featureSettings[fontIndex][featureNode.get()->getDataValue().get<sal_uInt32>()] = settingNode.get()->getDataValue().get<sal_Int32>();
                    int settingIndex = feature.get()->getIndex(setting);
                    for (int i = 0; i < feature.get()->getChildCount(); i++)
                    {
                        if (i == settingIndex) continue;
                        featureNode.set(feature.get()->getChildAt(i), css::uno::UNO_QUERY);
                        featureNode.get()->setNodeGraphicURL(m_extensionBase + DISABLED_ICON);
                    }
                }
            }
        }
    }
}

// currently we create this ourselves in GraphiteAddOn, so the helper is strictly needed

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
    return static_cast< ::cppu::OWeakObject * >(new org::sil::graphite::FeatureDialogEventHandler(context, emptyModel, ::rtl::OUString()));
}

}}}} // closing component helper namespace

