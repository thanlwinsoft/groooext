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

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"
#include "cppuhelper/implementationentry.hxx"

#ifdef WIN32
#include <windows.h>
#endif

#include "groooDebug.hxx"
#include "graphiteooo.hxx"
#include "GraphiteAddOn.hxx"

#include "DialogEventHandler.hxx"
#include "FeatureDialogEventHandler.hxx"
#include "SetupContextMenu.hxx"

const char * ::org::sil::graphite::SAL_DISABLE_GRAPHITE = "SAL_DISABLE_GRAPHITE";

namespace css = ::com::sun::star;
namespace osg = ::org::sil::graphite;

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

extern "C"
{

	SAL_DLLPUBLIC_EXPORT 
	void SAL_CALL component_getImplementationEnvironment(
		const char ** envTypeName, uno_Environment **)
	{
#ifdef _MSC_VER
		OutputDebugStringA("getImplementationEnvironment\n");
#endif
		*envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
	}

	SAL_DLLPUBLIC_EXPORT
	void * SAL_CALL component_getFactory(
		const char * implName, void * serviceManager, void * registryKey)
	{
#ifdef GROOO_DEBUG
		osg::logMsg("component_getFactory %s\n", implName);
#endif
		return ::cppu::component_getFactoryHelper(
			implName, serviceManager, registryKey, entries);
	}

	SAL_DLLPUBLIC_EXPORT
	sal_Bool SAL_CALL component_writeInfo(
		void * serviceManager, void * registryKey)
	{
		return ::cppu::component_writeInfoHelper(serviceManager, registryKey, entries);
	}

}
