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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "sal/config.h"
#include "uno/lbnames.h"
#include "uno/environment.hxx"
#include "rtl/string.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "groooDebug.hxx"

extern "C" {
SAL_DLLPUBLIC_IMPORT
	void SAL_CALL component_getImplementationEnvironment(
		const char ** envTypeName, uno_Environment **);
}

int main(int /*argc*/, char ** /*argv*/)
{
	const char * envTypeName = NULL;
	uno_Environment * unoEnvironment = NULL;
	component_getImplementationEnvironment(&envTypeName, &unoEnvironment);

	if (envTypeName)
	{
		if (strcmp(envTypeName, "TMP_CPPU_ENV") == 0)
		{
			printf("Invalid Environment: %s\n", envTypeName);
			return 2;
		}
		printf("Environment: %s\n", envTypeName);
		::rtl::OUString ouEnvTypeName = ::rtl::OUString::createFromAscii(envTypeName);
		return 0;
	}
	else
	{
		printf("No environment\n");
	}
	return 1;
}
