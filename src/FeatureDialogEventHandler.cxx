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
#include "rtl/ustrbuf.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/beans/XProperty.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/drawing/XShape.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Sequence.hxx"
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
#include "com/sun/star/style/XStyleFamiliesSupplier.hpp"
#include "com/sun/star/style/XStyleSupplier.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"

#include "FeatureDialogEventHandler.hxx"
#include "GraphiteFontInfo.hxx"
#include "groooDebug.hxx"
#include "graphiteooo.hxx"

namespace css = ::com::sun::star;

namespace osg = ::org::sil::graphite;
const ::rtl::OUString osg::FeatureDialogEventHandler::OK_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_ok"));
const ::rtl::OUString osg::FeatureDialogEventHandler::CANCEL_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_cancel"));
const ::rtl::OUString osg::FeatureDialogEventHandler::EXTERNAL_EVENT(RTL_CONSTASCII_USTRINGPARAM("external_event"));
const ::rtl::OUString osg::FeatureDialogEventHandler::DIALOG_FOCUS_EVENT(RTL_CONSTASCII_USTRINGPARAM("grfeaturedialog_focus"));
const ::rtl::OUString osg::FeatureDialogEventHandler::TREE_CONTROL(RTL_CONSTASCII_USTRINGPARAM("GrFeatureTreeControl"));
const ::rtl::OUString osg::FeatureDialogEventHandler::UPDATE_STYLE_CHECKBOX(RTL_CONSTASCII_USTRINGPARAM("UpdateStyleCheckBox"));
const ::rtl::OUString osg::FeatureDialogEventHandler::FONT_PROPERTY_NAME[NUM_SCRIPTS] =  {  
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontName")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameComplex")),
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameAsian"))
};
const ::rtl::OUString osg::FeatureDialogEventHandler::ENABLED_ICON = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/icons/enabled.png"));
const ::rtl::OUString osg::FeatureDialogEventHandler::DISABLED_ICON = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/icons/disabled.png"));

org::sil::graphite::FeatureDialogEventHandler::FeatureDialogEventHandler(css::uno::Reference< css::uno::XComponentContext > const & context, 
                                                                              css::uno::Reference< css::frame::XModel > const & model,
                                                                            const ::rtl::OUString & location) :
    m_xContext(context), m_xFactory(context.get()->getServiceManager()), m_xModel(model),
    m_xResource(getResource(context, ::rtl::OUString::createFromAscii("GraphiteMessages")),
               css::uno::UNO_QUERY),
    m_extensionBase(location)
{
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler constructor\n");
#endif
    m_fonts[0] = m_fonts[1] = m_fonts[2] = NULL;
}

// ::com::sun::star::awt::XDialogEventHandler:
::sal_Bool SAL_CALL org::sil::graphite::FeatureDialogEventHandler::callHandlerMethod(const css::uno::Reference< css::awt::XDialog > & xDialog, const ::com::sun::star::uno::Any & EventObject, const ::rtl::OUString & MethodName) throw (css::uno::RuntimeException, css::lang::WrappedTargetException)
{
#ifdef GROOO_DEBUG
    rtl::OString aMethodName;
    MethodName.convertToString(&aMethodName, RTL_TEXTENCODING_UTF8, 128);
    logMsg("FeatureDialogEventHandler callHandlerMethod(%s)\n", aMethodName.getStr());
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
    methodNames[3] = DIALOG_FOCUS_EVENT;
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::getSupportedMethodNames\n");
#endif
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return methodNames;
}


// ::com::sun::star::lang::XEventListener:
void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::disposing(const css::lang::EventObject & Source) throw (css::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::disposing\n");
#endif
    GraphiteFontInfo & grInfo = GraphiteFontInfo::getFontInfo();
    for (int i = 0; i < 3; i++)
        if (m_fonts[i])
        {
            grInfo.unloadFont(m_fonts[i]);
            m_fonts[i] = NULL;
        }
}

/**
* Parses the feature description into a hash table of featureId:featureSetting
* @param fontScript
* @param featureDesc
*/
void org::sil::graphite::FeatureDialogEventHandler::initFeatureMap(FontScript fontScript, const ::rtl::OUString & featureDesc)
{
    sal_Int32 sep = featureDesc.indexOf(GraphiteFontInfo::FEAT_SEPARATOR);
    sal_Int32 equals = featureDesc.indexOf(GraphiteFontInfo::FEAT_ID_VALUE_SEPARATOR);
    sal_Int32 pos = 0;
    while (sep > -1 && equals + 1 < sep && equals > -1)
    {
        ::rtl::OUString featIdString = featureDesc.copy(pos, equals - pos);
        ::rtl::OUString featSettingString = featureDesc.copy(equals + 1, sep - equals - 1);
        try
        {
            sal_uInt32 featId = GraphiteFontInfo::ouString2FeatId(featIdString);
            sal_Int32 featSetting = GraphiteFontInfo::ouString2FeatSetting(featSettingString);
            m_featureSettings[fontScript][featId] = featSetting;
        }
        catch (css::lang::IllegalArgumentException e)
        {
            ::rtl::OString msg;
            e.Message.convertToString(&msg, RTL_TEXTENCODING_UTF8, 128);
#ifdef GROOO_DEBUG
            logMsg("initFeatureMap illegal feature %s\n", msg.getStr());
#endif
        }
        pos = sep + 1;
        if (pos == featureDesc.getLength()) break;
        sep = featureDesc.indexOf(GraphiteFontInfo::FEAT_SEPARATOR, pos);
        equals = featureDesc.indexOf(GraphiteFontInfo::FEAT_ID_VALUE_SEPARATOR, pos);
    }
    // last feature
    if (equals > pos)
    {
        sep = featureDesc.getLength();
        ::rtl::OUString featIdString = featureDesc.copy(pos, equals - pos);
        ::rtl::OUString featSettingString = featureDesc.copy(equals + 1, sep - equals - 1);
        try
        {
            sal_uInt32 featId = GraphiteFontInfo::ouString2FeatId(featIdString);
            sal_Int32 featSetting = GraphiteFontInfo::ouString2FeatSetting(featSettingString);
            m_featureSettings[fontScript][featId] = featSetting;
#ifdef GROOO_DEBUG
            logMsg("Feature %lx = %ld\n", featId, featSetting);
#endif
        }
        catch (css::lang::IllegalArgumentException e)
        {
            ::rtl::OString msg;
            e.Message.convertToString(&msg, RTL_TEXTENCODING_UTF8, msg.getLength());
#ifdef GROOO_DEBUG
            logMsg("initFeatureMap illegal feature %s\n", msg.getStr());
#endif
        }
    }
}

/**
* Adds the font features for a given font to the tree
* @param xMutableDataModel - tree model
* @param rootNode of tree
* @param fontScript western/ctl/asian
* @param fontName full name of font as stored in OTF
* @param fontDesc description of type of font
*/
void org::sil::graphite::FeatureDialogEventHandler::addFontFeatures(
    css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel,
    css::uno::Reference<css::awt::tree::XMutableTreeNode> rootNode, 
    FontScript fontScript, const ::rtl::OUString & fontName, const ::rtl::OUString & fontDesc)
{
    sal_Int32 fontFeatStart = fontName.indexOf(GraphiteFontInfo::FEAT_PREFIX);
    ::rtl::OUString fontNameOnly;
    if (fontFeatStart > -1 && (fontName.getLength() > fontFeatStart + 1))
    {
        ::rtl::OUString features = fontName.copy(fontFeatStart + 1);
        initFeatureMap(fontScript, features);
        fontNameOnly = fontName.copy(0, fontFeatStart);
    }
    else
    {
        fontNameOnly = fontName;
    }
    m_fontNamesWithFeatures[fontScript] = fontNameOnly;
    css::uno::Reference<css::awt::tree::XMutableTreeNode> fontNode = xMutableDataModel.get()->createNode(css::uno::Any(fontNameOnly + fontDesc), sal_False);
    css::uno::Reference<css::awt::tree::XTreeNode> xFontTreeNode(fontNode, css::uno::UNO_QUERY);
    fontNode.get()->setDataValue(css::uno::Any(fontNameOnly));
    ::rtl::OUString defaultLabel =
        getResourceString(m_xResource, "GraphiteFeatures.DefaultLabel");
    GraphiteFontInfo & grInfo = GraphiteFontInfo::getFontInfo();
    rootNode.get()->appendChild(fontNode);
    m_xTree.get()->makeNodeVisible(xFontTreeNode);

    if (grInfo.isGraphiteFont(fontNameOnly))
    {
        gr::Font * grFont = grInfo.loadFont(fontNameOnly);
        m_fonts[rootNode.get()->getChildCount() - 1] = grFont;
        if (grFont)
        {
            gr::lgid en_US(0x0409);
            std::pair<gr::FeatureIterator, gr::FeatureIterator>iFeats = grFont->getFeatures();
            std::pair<gr::FeatLabelLangIterator, gr::FeatLabelLangIterator > iFeatLangs = grFont->getFeatureLabelLanguages();
            gr::lgid lang = en_US;
            gr::FeatureIterator iFeat = iFeats.first;
#ifdef GROOO_DEBUG
            logMsg("Loaded font with %d features\n", iFeats.second - iFeats.first);
#endif
            while (iFeat != iFeats.second)
            {
                gr::utf16 featLabel[128];
                bool hasLabel = grFont->getFeatureLabel(iFeat, en_US, featLabel);
                // if we don't have English, randomly pick the first language for now
                if (!hasLabel && (iFeatLangs.first != iFeatLangs.second))
                {
                    hasLabel = grFont->getFeatureLabel(iFeat, *(iFeatLangs.first), featLabel);
                    lang = *(iFeatLangs.first);
#ifdef GROOO_DEBUG
					logMsg("Feature label for en_US not found using lang %x\n", lang);
#endif
                }
                ::rtl::OUString featName(reinterpret_cast<sal_Unicode*>(featLabel));
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
                    sal_Int32 value = *defaultValue;
                    if (m_featureSettings[fontScript].find(*iFeat) != m_featureSettings[fontScript].end())
                        value = m_featureSettings[fontScript][*iFeat];
#ifdef GROOO_DEBUG
                    logMsg("Feature %d has %d settings\n", iFeat - iFeats.first, iSettings.second - iSettings.first);
#endif
                    while (iSetting != iSettings.second)
                    {
                        hasLabel = grFont->getFeatureSettingLabel(iSetting, lang, featLabel);
						assert(sizeof(gr::utf16) == sizeof(sal_Unicode));
                        ::rtl::OUString settingName(reinterpret_cast<sal_Unicode*>(featLabel));
                        if (*iSetting == *defaultValue)
                            settingName += defaultLabel;
                        css::uno::Reference<css::awt::tree::XMutableTreeNode> settingNode = xMutableDataModel.get()->createNode(css::uno::Any(settingName), sal_False);
                        settingNode.get()->setDataValue(css::uno::Any(static_cast<sal_Int32>(*iSetting)));
                        featNode.get()->appendChild(settingNode);
                        if (*iSetting == value)
                        {
                            settingNode.get()->setNodeGraphicURL(m_extensionBase + ENABLED_ICON);
                            // make non-default settings visible
                            if (value != *defaultValue)
                            {
                                css::uno::Reference<css::awt::tree::XTreeNode> xFeatSettingTreeNode(settingNode, css::uno::UNO_QUERY);
                                m_xTree.get()->makeNodeVisible(xFeatSettingTreeNode);
                            }
                        }
                        else
                        {
                            settingNode.get()->setNodeGraphicURL(m_extensionBase + DISABLED_ICON);
                        }
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

void org::sil::graphite::FeatureDialogEventHandler::setupTreeModel(
    css::uno::Reference<css::awt::tree::XMutableTreeDataModel> xMutableDataModel)
{
    assert(xMutableDataModel.is());
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::setupTreeModel\n");
#endif
    m_xController.set(m_xModel->getCurrentController());
    if (!m_xController.is()) return;

    //css::uno::Reference<css::text::XTextViewCursorSupplier> xTextCursorSupplier;
    //css::uno::Reference<css::text::XTextViewCursor> xTextCursor;
    css::uno::Reference< css::awt::XWindow> xCheckBoxWindow(m_xUpdateStyle, css::uno::UNO_QUERY);

   // xTextCursorSupplier.set(m_xController, css::uno::UNO_QUERY);
    //if (xTextCursorSupplier.is())
    //    xTextCursor.set(xTextCursorSupplier->getViewCursor());
    ::rtl::OUString rootName = getResourceString(m_xResource, "GraphiteFeatures.Fonts");
    css::uno::Reference<css::awt::tree::XMutableTreeNode> rootNode = xMutableDataModel.get()->createNode(css::uno::Any(rootName), sal_False);
    xMutableDataModel.get()->setRoot(rootNode);

    css::uno::Reference<css::style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(m_xModel, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameContainer> xCharStylesContainer;
    css::uno::Reference<css::container::XNameContainer> xParaStylesContainer;
    css::uno::Reference<css::container::XNameContainer> xCellStylesContainer;
    css::uno::Reference<css::container::XNameContainer> xGraphicStylesContainer;
    if (xStyleFamiliesSupplier.is())
    {
        css::uno::Reference<css::container::XNameAccess> xStyleNameAccess(xStyleFamiliesSupplier.get()->getStyleFamilies());
#ifdef GROOO_DEBUG
        css::uno::Sequence< ::rtl::OUString > styleFamilyNames = xStyleNameAccess->getElementNames();
        for (int i = 0; i < styleFamilyNames.getLength(); i++)
        {
            ::rtl::OString familyName;
            styleFamilyNames[i].convertToString(&familyName, RTL_TEXTENCODING_UTF8, 128);
            logMsg("StyleFamily: %s\n", familyName.getStr());
        }
#endif
        ::rtl::OUString charStylesName = ::rtl::OUString::createFromAscii("CharacterStyles");
        if (xStyleNameAccess->hasByName(charStylesName))
        {
            css::uno::Any aCharacterStyles = xStyleNameAccess.get()->getByName(charStylesName);
            if (aCharacterStyles.has<css::uno::Reference<css::container::XNameContainer> >())
            {
                xCharStylesContainer.set(aCharacterStyles.get<css::uno::Reference<css::container::XNameContainer> >());
            }
            else
            {
#ifdef GROOO_DEBUG
                logMsg("CharacterStyles name container not found\n");
#endif
            }
        }
        else
        {
#ifdef GROOO_DEBUG
            logMsg("CharacterStyles not found\n");
#endif
        }
        ::rtl::OUString paraStylesName = ::rtl::OUString::createFromAscii("ParagraphStyles");
        if (xStyleNameAccess->hasByName(paraStylesName))
        {
            css::uno::Any aParagraphStyles = xStyleNameAccess.get()->getByName(paraStylesName);
            if (aParagraphStyles.has<css::uno::Reference<css::container::XNameContainer> >())
            {
                xParaStylesContainer.set(aParagraphStyles.get<css::uno::Reference<css::container::XNameContainer> >());
            }
        }
        else
        {
#ifdef GROOO_DEBUG
            logMsg("ParagraphStyles not found\n");
#endif
        }
        // CellStyles
        ::rtl::OUString cellStylesName = ::rtl::OUString::createFromAscii("CellStyles");
        if (xStyleNameAccess->hasByName(cellStylesName))
        {
            css::uno::Any aCellStyles = xStyleNameAccess.get()->getByName(cellStylesName);
            if (aCellStyles.has<css::uno::Reference<css::container::XNameContainer> >())
            {
                xCellStylesContainer.set(aCellStyles.get<css::uno::Reference<css::container::XNameContainer> >());
            }
        }
        else
        {
#ifdef GROOO_DEBUG
            logMsg("CellStyles not found\n");
#endif
        }
        // GraphicStyles
        ::rtl::OUString graphicsStylesName = ::rtl::OUString::createFromAscii("graphics");
        if (xStyleNameAccess->hasByName(graphicsStylesName))
        {
            css::uno::Any aGraphicsStyles = xStyleNameAccess.get()->getByName(graphicsStylesName);
            if (aGraphicsStyles.has<css::uno::Reference<css::container::XNameContainer> >())
            {
                xGraphicStylesContainer.set(aGraphicsStyles.get<css::uno::Reference<css::container::XNameContainer> >());
            }
        }
        else
        {
#ifdef GROOO_DEBUG
            logMsg("graphics not found\n");
#endif
        }
    }

    // Note: the document selection supplier is not the same as the dialog one
    css::uno::Reference<css::view::XSelectionSupplier> xDocSelectionSupplier(m_xController, css::uno::UNO_QUERY);
    m_xTextProperties.set(getTextPropertiesFromModel(m_xModel, xDocSelectionSupplier));

    if (m_xTextProperties.is())
    {
#ifdef GROOO_DEBUG
        logMsg("Have text properties\n");
#endif

        css::uno::Any aFontName;
        css::uno::Any aFontNameComplex;
        css::uno::Any aFontNameAsian;
        ::rtl::OUString ouWestern;
        ::rtl::OUString ouCtl;
        ::rtl::OUString ouAsian;

        ::rtl::OUString westernFontDesc = getResourceString(m_xResource, "GraphiteFeatures.Western");
        ::rtl::OUString complexFontDesc = getResourceString(m_xResource, "GraphiteFeatures.CTL");
        ::rtl::OUString asianFontDesc = getResourceString(m_xResource, "GraphiteFeatures.Asian");

        css::uno::Reference< css::beans::XPropertySetInfo >
            xTextPropSetInfo(m_xTextProperties->getPropertySetInfo());

        if (xTextPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[WESTERN_SCRIPT]))
        {
            aFontName = m_xTextProperties.get()->
                getPropertyValue(FONT_PROPERTY_NAME[WESTERN_SCRIPT]);
            ouWestern = aFontName.get< ::rtl::OUString>();
            addFontFeatures(xMutableDataModel, rootNode, WESTERN_SCRIPT, ouWestern, westernFontDesc);
        }
        if (xTextPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[CTL_SCRIPT]))
        {
            aFontNameComplex = m_xTextProperties.get()->
                getPropertyValue(FONT_PROPERTY_NAME[CTL_SCRIPT]);
            ouCtl = aFontNameComplex.get< ::rtl::OUString>();
            addFontFeatures(xMutableDataModel, rootNode, CTL_SCRIPT, ouCtl, complexFontDesc);
        }
        if (xTextPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[ASIAN_SCRIPT]))
        {
            aFontNameAsian = m_xTextProperties.get()->
                getPropertyValue(FONT_PROPERTY_NAME[ASIAN_SCRIPT]);
            ouAsian =  aFontNameAsian.get< ::rtl::OUString>();
            addFontFeatures(xMutableDataModel, rootNode, ASIAN_SCRIPT, ouAsian, asianFontDesc);
        }
        //printPropertyNames(m_xTextProperties);
#ifdef GROOO_DEBUG
        logMsg("Check style fonts\n");
#endif
        css::uno::Any charStyle;
        static const ::rtl::OUString charStyleName = ::rtl::OUString::createFromAscii("CharStyleName");
        static const ::rtl::OUString paraStyleName = ::rtl::OUString::createFromAscii("ParaStyleName");
        static const ::rtl::OUString cellStyleName = ::rtl::OUString::createFromAscii("CellStyle");
        static const ::rtl::OUString graphicsStyle =
            ::rtl::OUString::createFromAscii("Style");
        if (xCharStylesContainer.is() && m_xTextProperties.get()->getPropertySetInfo()->hasPropertyByName(charStyleName) &&
            m_xTextProperties.get()->getPropertySetInfo()->hasPropertyByName(paraStyleName) )
        {
            css::uno::Any aCharStyle = m_xTextProperties.get()->getPropertyValue(charStyleName);
            css::uno::Any aParaStyle =  m_xTextProperties.get()->getPropertyValue(paraStyleName);
            ::rtl::OUString styleName = aCharStyle.get< ::rtl::OUString>();
            if (xCharStylesContainer.get()->hasByName(styleName))
            {
                charStyle = xCharStylesContainer.get()->getByName(styleName);
            }
            else
            {
                styleName = aParaStyle.get< ::rtl::OUString>();
                charStyle = xParaStylesContainer.get()->getByName(styleName);
            }
        }
        else if (xCellStylesContainer.is() && m_xTextProperties.get()->getPropertySetInfo()->hasPropertyByName(cellStyleName))
        {
            css::uno::Any aCellStyleName = m_xTextProperties.get()->getPropertyValue(cellStyleName);
            ::rtl::OUString styleName = aCellStyleName.get< ::rtl::OUString>();
            if (xCellStylesContainer.get()->hasByName(styleName))
            {
                charStyle = xCellStylesContainer.get()->getByName(styleName);
            }
        }
        else if (xGraphicStylesContainer.is() &&
            m_xTextProperties.get()->getPropertySetInfo()->hasPropertyByName(graphicsStyle))
        {
            // the shape's style property contains the Style itself, not the name
            charStyle = m_xTextProperties->getPropertyValue(graphicsStyle);
        }

        if (charStyle.hasValue() &&
            charStyle.has<css::uno::Reference<  css::uno::XInterface > >())
        {
            css::uno::Reference< css::beans::XPropertySet> xStyleTextProperties(
                charStyle.get<css::uno::Reference< css::uno::XInterface> >(), css::uno::UNO_QUERY);
            assert(xStyleTextProperties.is());
            //printPropertyNames(xStyleTextProperties);
            css::uno::Any aStyleFontName;
            css::uno::Any aStyleFontNameComplex;
            css::uno::Any aStyleFontNameAsian;
            ::rtl::OUString ouWesternStyle;
            ::rtl::OUString ouCtlStyle;
            ::rtl::OUString ouAsianStyle;
            css::uno::Reference< css::beans::XPropertySetInfo> xPropSetInfo(xStyleTextProperties->getPropertySetInfo());
            if (xPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[WESTERN_SCRIPT]))
            {
                aStyleFontName = xStyleTextProperties.get()->
                    getPropertyValue(FONT_PROPERTY_NAME[WESTERN_SCRIPT]);
                ouWesternStyle = aStyleFontName.get< ::rtl::OUString >();
            }
            if (xPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[CTL_SCRIPT]))
            {
                aStyleFontNameComplex = xStyleTextProperties.get()->
                    getPropertyValue(FONT_PROPERTY_NAME[CTL_SCRIPT]);
                ouCtlStyle = aStyleFontNameComplex.get< ::rtl::OUString >();
            }
            if (xPropSetInfo->hasPropertyByName(FONT_PROPERTY_NAME[ASIAN_SCRIPT]))
            {
                aStyleFontNameAsian = xStyleTextProperties.get()->
                    getPropertyValue(FONT_PROPERTY_NAME[ASIAN_SCRIPT]);
                ouAsianStyle = aStyleFontNameAsian.get< ::rtl::OUString >();
            }
            if (ouWestern.compareTo(ouWesternStyle) != 0 ||
                ouCtl.compareTo(ouCtlStyle) != 0 ||
                ouAsian.compareTo(ouAsianStyle) != 0)
            {
                // fonts don't match the style, so don't offer sync option
                xCheckBoxWindow.get()->setVisible(sal_False);
            }
            else
            {
                m_xStyleTextProperties.set(xStyleTextProperties);
            }
        }
        else
        {
            xCheckBoxWindow.get()->setVisible(sal_False);
        }

        m_xTree.get()->addTreeEditListener(this);
    }
}

// ::com::sun::star::awt::XTopWindowListener:
void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowOpened(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    try
    {
        css::uno::Reference< css::awt::XControlContainer > xControlContainer(e.Source, css::uno::UNO_QUERY);
        assert(xControlContainer.is());
        css::uno::Reference< css::awt::XControl > xControl = xControlContainer.get()->getControl(TREE_CONTROL);
        m_xTree.set(xControl, css::uno::UNO_QUERY);
        css::uno::Reference< css::awt::XControl > xCheckBoxControl = xControlContainer.get()->getControl(UPDATE_STYLE_CHECKBOX);
        m_xUpdateStyle.set(xCheckBoxControl, css::uno::UNO_QUERY);
        m_xSelectionSupplier.set(xControl, css::uno::UNO_QUERY);
        if (m_xSelectionSupplier.is())
            m_xSelectionSupplier.get()->addSelectionChangeListener(this);
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

        assert(m_xTree.is());
    }
    catch (css::beans::UnknownPropertyException e)
    {
#ifdef GROOO_DEBUG
        rtl::OString msg;
        e.Message.convertToString(&msg, RTL_TEXTENCODING_UTF8, 128);
        logMsg("UnknownPropertyException %s\n", msg.getStr());
#endif
    }
    catch (css::uno::Exception e)
    {
#ifdef GROOO_DEBUG
        rtl::OString msg;
        e.Message.convertToString(&msg, RTL_TEXTENCODING_UTF8, 128);
        logMsg("Exception %s\n", msg.getStr());
#endif
    }
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::windowOpened\n");
#endif
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowClosing(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    // this is only called when the user closes the dialog from the window title bar
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::windowClosing\n");
#endif
}

void SAL_CALL org::sil::graphite::FeatureDialogEventHandler::windowClosed(const css::lang::EventObject & e) throw (css::uno::RuntimeException)
{
    storeFeatures();
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::windowClosed\n");
#endif
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
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::nodeEditing\n");
#endif
}

void SAL_CALL 
org::sil::graphite::FeatureDialogEventHandler::nodeEdited( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node, const ::rtl::OUString& NewText ) throw (::com::sun::star::uno::RuntimeException)
{
#ifdef GROOO_DEBUG
    logMsg("FeatureDialogEventHandler::nodeEdited\n");
#endif
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
#ifdef GROOO_DEBUG
                    logMsg("FeatureDialogEventHandler::selectionChanged font %d node %d\n", fontIndex, settingNode.is());
#endif
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

void org::sil::graphite::FeatureDialogEventHandler::setFontNames(void)
{
    css::uno::Reference<css::view::XSelectionSupplier> xDocSelectionSupplier(m_xController, css::uno::UNO_QUERY);
    m_xTextProperties.set(getTextPropertiesFromModel(m_xModel, xDocSelectionSupplier));

    if (m_xTextProperties.is())
    {
        for (int i = 0; i < NUM_SCRIPTS; i++)
        {
            if (m_fontNamesWithFeatures[i].getLength() > 0)
            {
                m_xTextProperties->setPropertyValue(FONT_PROPERTY_NAME[i], css::uno::Any(m_fontNamesWithFeatures[i]));
                // set it on the style as well if requested
                if (m_xStyleTextProperties.is() && m_xUpdateStyle.get()->getState())
                    m_xStyleTextProperties.get()->setPropertyValue(FONT_PROPERTY_NAME[i], css::uno::Any(m_fontNamesWithFeatures[i]));
#ifdef GROOO_DEBUG
                ::rtl::OString msg;
                ::rtl::OString setName;
                m_fontNamesWithFeatures[i].convertToString(&msg, RTL_TEXTENCODING_UTF8, msg.getLength());
                css::uno::Any aSetName =
                    m_xTextProperties->getPropertyValue(FONT_PROPERTY_NAME[i]);
                aSetName.get< ::rtl::OUString >().convertToString(&setName, RTL_TEXTENCODING_UTF8, 128);
                css::beans::Property prop = m_xTextProperties->getPropertySetInfo()->
                    getPropertyByName(FONT_PROPERTY_NAME[i]);
                logMsg("set font %d to %s requested %s updated style %d property has type %x\n",
                       i, setName.getStr(), msg.getStr(),
                       m_xUpdateStyle.get()->getState(), prop.Attributes);
#endif
            }
        }
    }
}

void org::sil::graphite::FeatureDialogEventHandler::storeFeatures(void)
{
    for (int i = 0; i < 3; i++)
    {
        ::rtl::OUStringBuffer featBuilder;
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
                        if (featBuilder.getLength() > 0)
                            featBuilder.append(GraphiteFontInfo::FEAT_SEPARATOR);
                        featBuilder.append(GraphiteFontInfo::featId2OUString(*iFeat));
                        featBuilder.append(GraphiteFontInfo::FEAT_ID_VALUE_SEPARATOR);
                        featBuilder.append(GraphiteFontInfo::featSetting2OUString(iSetFeat->second));
                    }
                }
                ++iSetFeat;
            }
        }
        if (featBuilder.getLength()>0)
            m_fontNamesWithFeatures[i] += GraphiteFontInfo::FEAT_PREFIX + featBuilder.makeStringAndClear();
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
    logMsg("FeatureDialogEventHandler _getSupportedServiceNames\n");
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

