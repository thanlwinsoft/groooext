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

#include <stdarg.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#ifdef WIN32
#include <windows.h>
#endif

#include "sal/config.h"
#include "uno/lbnames.h"
#include "rtl/string.hxx"

#include "groooDebug.hxx"

namespace org { namespace sil { namespace graphite {

#ifdef GROOO_DEBUG
FILE * grLogFile = NULL;
FILE * grLog()
{
#ifdef _MSC_VER
	if (grLogFile == NULL) {
		size_t envBufSize;
		char * envBuf;
		if (_dupenv_s(&envBuf, &envBufSize, "TEMP"))
			return NULL;
		::std::string logFileName(envBuf);
		logFileName.append("\\graphiteoooext.log");
		//if (fopen_s(&grLogFile, logFileName.c_str(),"w"))
		//	grLogFile = NULL;
#pragma warning(disable: 4996)
		// don't use fopen_s - it will lock the file
		grLogFile = fopen(logFileName.c_str(), "w");
	}
    else fflush(grLogFile);
    return grLogFile;
#else
    return stdout;
#endif
}
#endif

	void logMsg(const char * msg, ...)
	{
#ifdef GROOO_DEBUG
		va_list args;
		va_start(args, msg);
#ifdef _MSC_VER
		char buffer[1024];
		if (vsnprintf_s(buffer, 1024, 1024, msg, args) > 0)
			OutputDebugStringA(buffer);
#endif
		if (grLog())
			vfprintf(grLog(), msg, args);
		va_end(args);
#endif
	}

}}}

