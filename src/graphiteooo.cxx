/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2010 ThanLwinSoft.org
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

#include "sal/typesizes.h"
#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "graphiteooo.hxx"
#include "GraphiteAddOn.hxx"
#include "DialogEventHandler.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "SetupContextMenu.hxx"

const char * org::sil::graphite::SAL_DISABLE_GRAPHITE = "SAL_DISABLE_GRAPHITE";

namespace css = ::com::sun::star;

void org::sil::graphite::printPropertyNames(css::uno::Reference<css::beans::XPropertySet > propSet)
{
    css::uno::Reference< css::beans::XPropertySetInfo>xPropSetInfo(propSet.get()->getPropertySetInfo());
    css::uno::Sequence< css::beans::Property> properties = xPropSetInfo.get()->getProperties();
    for (int i = 0; i < properties.getLength(); i++)
    {
        ::rtl::OString propName(128);
        ::rtl::OString propValue(128);
        properties[i].Name.convertToString(&propName, RTL_TEXTENCODING_UTF8, propName.getLength());
        try
        {
            ::css::uno::Any aValue = propSet.get()->getPropertyValue(properties[i].Name);
            if (aValue.hasValue() && aValue.has< ::rtl::OUString>())
            {
                ::rtl::OUString value = aValue.get< ::rtl::OUString>();
                value.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
            }
            else
            {
                propValue = "?";
            }
        }
        catch (::com::sun::star::lang::WrappedTargetException we)
        {
            css::uno::Exception e = we.TargetException.get<css::uno::Exception>();
            e.Message.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
        }
        catch (::com::sun::star::uno::Exception e)
        {
            e.Message.convertToString(&propValue, RTL_TEXTENCODING_UTF8, propValue.getLength());
        }
        fprintf(stderr, "Property name:%s value:%s\n", propName.getStr(), propValue.getStr());
    }
}

// Define the supported services
static ::cppu::ImplementationEntry const entries[] = {
    { &org::sil::graphite::graphiteaddon::_create,
      &org::sil::graphite::graphiteaddon::_getImplementationName,
      &org::sil::graphite::graphiteaddon::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &org::sil::graphite::dialogeventhandler::_create,
      &org::sil::graphite::dialogeventhandler::_getImplementationName,
      &org::sil::graphite::dialogeventhandler::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { &org::sil::graphite::setupcontextmenu::_create,
      &org::sil::graphite::setupcontextmenu::_getImplementationName,
      &org::sil::graphite::setupcontextmenu::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};


extern "C" void SAL_CALL component_getImplementationEnvironment(
    const char ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" void * SAL_CALL component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
#ifdef GROOO_DEBUG
    printf("component_getFactory %s\n", implName);
#endif
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return ::cppu::component_writeInfoHelper(serviceManager, registryKey, entries);
}

