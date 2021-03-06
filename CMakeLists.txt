#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2010 ThanLwinSoft.org & SIL International
#
# This file is part of the Graphite extension for OpenOffice.org (GraphiteOOo).
#
# The GraphiteOOo Extension is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# The GraphiteOOo Extension is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
project (graphiteooo)
enable_language(CXX C)

set(OOO_VERSION 3.2) # used for basis directory

# The version number. This should be matched by a tag in mercurial
set (graphiteooo_VERSION_MAJOR 0)
set (graphiteooo_VERSION_MINOR 2)
set (graphiteooo_VERSION_PATCH 2)

if (EXISTS ${PROJECT_SOURCE_DIR}/.hg_archival.txt)
    # that the directory name contains the version as for a debian package
    string(REGEX REPLACE ".*openoffice.org-graphite_[0-9]+[.][0-9]+[.]([0-9]+)$" "\\1" HG_PATCH_VERSION ${PROJECT_SOURCE_DIR})
    if (${HG_PATCH_VERSION} MATCHES "[0-9]+")
        set(graphiteooo_VERSION_PATCH ${HG_PATCH_VERSION})
    else (${HG_PATCH_VERSION} MATCHES "[0-9]+")
        message(FATAL_ERROR "Version=${graphiteooo_VERSION_MAJOR}.${graphiteooo_VERSION_MINOR}.${graphiteooo_VERSION_PATCH}")
    endif (${HG_PATCH_VERSION} MATCHES "[0-9]+")
else (EXISTS ${PROJECT_SOURCE_DIR}/.hg_archival.txt)
    # use the hg id number as the patch version
    execute_process(COMMAND hg id -n OUTPUT_VARIABLE MERCURIAL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE "+" "" MERCURIAL_VERSION ${MERCURIAL_VERSION})
    set(graphiteooo_VERSION_PATCH ${graphiteooo_VERSION_PATCH}.${MERCURIAL_VERSION})
endif (EXISTS ${PROJECT_SOURCE_DIR}/.hg_archival.txt)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/graphiteoooConfig.h.in"
  "${PROJECT_BINARY_DIR}/graphiteoooConfig.h"
  )

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")

# OO_SDK_URE_HOME is setup using the setsdkenv_* scripts
find_path(OO_SDK_IDL com/sun/star/udk-modules.idl
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES idl DOC "OO IDL directory")
find_path(OO_SDK_INCLUDE udkversion.mk
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES include DOC "OO SDK include directory")
find_library(OO_SDK_UNO_SAL NAMES uno_sal isal 
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES lib DOC "SAL library")
find_library(OO_SDK_UNO_CPPU NAMES uno_cppu icppu 
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES lib DOC "CPPU library")
find_library(OO_SDK_UNO_CPPUHELPER NAMES uno_cppuhelpergcc3 icppuhelper 
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES lib DOC "CPPU helper library")

find_program(OO_SDK_CPPUMAKER cppumaker 
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/sdk
    PATH_SUFFIXES bin DOC "IDL compiler")
find_path(OO_SDK_RDB types.rdb 
    PATHS ENV OO_SDK_URE_HOME ENV OO_SDK_HOME /usr/lib/ure 
    PATH_SUFFIXES share/misc misc DOC "URE RDB directory")
find_path(OO_BASIS_PROGRAM offapi.rdb
    PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis${OOO_VERSION}/program
    PATH_SUFFIXES ../program DOC "Basis Program directory")
find_path(OO_SDK_URE_BIN version.ini versionrc 
    PATHS ENV OO_SDK_URE_HOME /usr/lib/ure 
    PATH_SUFFIXES bin DOC "URE bin directory")
find_file(OO_SDK_VERSION version.ini versionrc
    PATHS ENV OO_SDK_URE_HOME /usr/lib/ure
    PATH_SUFFIXES bin DOC "URE version file")
find_program(UNOPKG unopkg PATHS ENV OFFICE_PROGRAM_PATH /usr/bin
    DOC "OpenOffice.org Extension Manager")
find_program(ZIP zip)
find_program(NATIVE2ASCII native2ascii PATHS ENV JAVA_HOME PATH_SUFFIXES bin)

# check if unopkg supports the -s option
file(STRINGS ${OO_SDK_VERSION} OO_VERSION_STRING REGEX "OOOBaseVersion=[0-9.]+")
file(STRINGS ${OO_SDK_VERSION} OO_PRODUCT_MAJOR_STRING REGEX "ProductMajor=[0-9.]+")
file(STRINGS ${OO_SDK_VERSION} OO_PRODUCT_MINOR_STRING REGEX "ProductMinor=[0-9.]+")
string(REPLACE "OOOBaseVersion=" "" OO_VERSION ${OO_VERSION_STRING})
string(REPLACE "ProductMajor=" "" OO_MAJOR_VERSION ${OO_PRODUCT_MAJOR_STRING})
string(REPLACE "ProductMinor=" "" OO_MINOR_VERSION ${OO_PRODUCT_MINOR_STRING})
if (${OO_VERSION} GREATER 3.2)
    set(UNOPKG_OPTIONS -s)
else (${OO_VERSION} GREATER 3.2) # not sure about the exact minor version to use
	if ((${OO_MAJOR_VERSION} EQUAL 320) AND (${OO_MINOR_VERSION} GREATER 16))
		set(UNOPKG_OPTIONS -s)
	else ((${OO_MAJOR_VERSION} EQUAL 320) AND (${OO_MINOR_VERSION} GREATER 16))
		set(UNOPKG_OPTIONS "")
	endif ((${OO_MAJOR_VERSION} EQUAL 320) AND (${OO_MINOR_VERSION} GREATER 16))
endif (${OO_VERSION} GREATER 3.2)

include_directories ("${OO_SDK_INCLUDE}" ${FREETYPE_INCLUDE_DIRS})

# Set the OOo Platform defines
# Beware: "cannot get uno environments!" - the logic in uno/lbnames.h doesn't
# work unless CPPU_ENV is set beforehand
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DUNX=1 -DLINUX=1 -DCPPU_ENV=gcc3 -Wall)
    set(BUILD_SHARED_LIBS on)
    find_package(Freetype)
    find_package(PkgConfig)
    pkg_check_modules(FC REQUIRED fontconfig)
    pkg_check_modules(GR REQUIRED silgraphite)
    add_definitions(${FC_CFLAGS} ${GR_CFLAGS} -I${FREETYPE_INCLUDE_DIR_freetype2})
    set(GR_DEBUG_LIBRARY ${GR_LIBRARIES})
    set(GR_RELEASE_LIBRARY ${GR_LIBRARIES})
    set(GROOO_OUTPUT_DIR "")
    set(PLATFORM_LIBRARIES "")
    # uname -p $(CMAKE_SYSTEM_PROCESSOR) gives unknown on Ubuntu
    execute_process(COMMAND uname -m
        OUTPUT_VARIABLE SYSTEM_MACHINE OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${SYSTEM_MACHINE} STREQUAL "i686")
        set(EXTENSION_PLATFORM linux_x86)
    elseif (${SYSTEM_MACHINE} STREQUAL "x86_64")
        set(EXTENSION_PLATFORM linux_x86_64)
    endif (${SYSTEM_MACHINE} STREQUAL "i686")
    message( "Platform=${EXTENSION_PLATFORM}" )

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DWNT=1 -DCPPU_ENV=msci /wd4290 /wd4800 /W4)
    set(BUILD_SHARED_LIBS on)
    set(CMAKE_CXX_FLAGS_DEBUG /MTd)
    set(CMAKE_CXX_FLAGS_RELEASE /MT)
    set(CMAKE_CXX_FLAGS_MINSIZEREL /MT)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO /MT)
#    if (${MSVC_VERSION} EQUAL 1500)
#        set(PLATFORM_LIBRARIES msvcr90.dll msvcm90.dll msvcp90.dll Microsoft.VC90.CRT.manifest)
#    else (${MSVC_VERSION} EQUAL 1500)
#        message(SEND_ERROR "Unknown version of MSC ${MSVC_VERSION}")
#    endif (${MSVC_VERSION} EQUAL 1500)

    find_path(GR_INCLUDE graphite/GrClient.h PATHS ENV SILGRAPHITE_HOME ../silgraphite-2.3.1 ../silgraphite-2.4.0 PATH_SUFFIXES engine/include)
    find_path(GR_WINFONT_INCLUDE WinFont.h PATHS ENV SILGRAPHITE_HOME ../silgraphite-2.3.1 ../silgraphite-2.4.0 PATH_SUFFIXES wrappers/win32)
    include_directories (${GR_INCLUDE} ${GR_INCLUDE}/graphite ${GR_WINFONT_INCLUDE})
    find_library(GR_DEBUG_LIBRARY graphite PATHS ENV SILGRAPHITE_HOME ../silgraphite-2.3.1 ../silgraphite-2.4.0     PATH_SUFFIXES engine/debug)
    find_library(GR_RELEASE_LIBRARY graphite PATHS ENV SILGRAPHITE_HOME ../silgraphite-2.3.1 ../silgraphite-2.4.0     PATH_SUFFIXES engine/release)
    find_path(MSVCDLL_PATH msvcr90.dll PATHS ENV VCINSTALLDIR PATH_SUFFIXES redist/x86/Microsoft.VC90.CRT)
    set(GROOO_OUTPUT_DIR ${CMAKE_BUILD_TYPE}/)
    set(EXTENSION_PLATFORM windows_${CMAKE_SYSTEM_PROCESSOR})

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

set_directory_properties(PROPERTY COMPILE_DEFINITIONS_DEBUG GROOO_DEBUG=1)

# add the library
add_library(graphiteooo
    src/graphiteooo.cxx src/graphiteooo.hxx
    src/groooDebug.cxx src/groooDebug.hxx
    src/UnixEnvironmentSetter.cxx src/UnixEnvironmentSetter.hxx
    src/GraphiteAddOn.cxx src/GraphiteAddOn.hxx
    src/OptionsDialogEventHandler.cxx src/OptionsDialogEventHandler.hxx
    src/FeatureDialogEventHandler.cxx src/FeatureDialogEventHandler.hxx
    src/GraphiteFontInfo.cxx src/GraphiteFontInfo.hxx
    src/GraphiteConfiguration.cxx src/GraphiteConfiguration.hxx
    src/SetupContextMenu.cxx src/SetupContextMenu.hxx)

set_source_files_properties(src/graphiteooo.hxx src/UnixEnvironmentSetter.hxx src/GraphiteAddOn.hxx
    src/OptionsDialogEventHandler.hxx src/FeatureDialogEventHandler.hxx src/GraphiteFontInfo.hxx
    src/GraphiteConfiguration.hxx src/SetupContextMenu.hxx PROPERTIES HEADER_FILE_ONLY true)

# the environment test is only relevant on Linux
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(test_unixenvset src/UnixEnvironmentSetter.cxx src/test_UnixEnvironmentSetter.cxx
        src/groooDebug.cxx src/groooDebug.hxx)
    add_test(NAME "test_unixenvset" COMMAND "$<TARGET_FILE:test_unixenvset>")
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

# test graphite font info
add_executable(test_graphitefontinfo src/GraphiteFontInfo.cxx src/test_GraphiteFontInfo.cxx
    src/groooDebug.cxx src/groooDebug.hxx)
add_test(NAME test_graphitefontinfo COMMAND "$<TARGET_FILE:test_graphitefontinfo>" 
    ENVIRONMENT PATH=${OO_SDK_URE_BIN} )

# very basic uno test for exported symbols
add_executable(test_graphiteooouno src/test_graphiteooouno.cxx
    src/groooDebug.cxx src/groooDebug.hxx)
add_test(NAME test_graphiteooouno COMMAND "$<TARGET_FILE:test_graphiteooouno>" 
    ENVIRONMENT PATH=${OO_SDK_URE_BIN} )
target_link_libraries(test_graphiteooouno graphiteooo)

add_executable(test_locale2lcid src/test_locale2lcid.cxx)
add_test(NAME test_locale2lcid COMMAND "$<TARGET_FILE:test_locale2lcid>"
    ENVIRONMENT PATH=${OO_SDK_URE_BIN} )
target_link_libraries(test_locale2lcid ${OO_SDK_UNO_SAL})

enable_testing()

# just use one sample IDL header since we compile them in one go
set_source_files_properties(src/GraphiteAddOn.cxx OBJECT_DEPENDS
    ${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp)

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true) # Leave the IDL files

# Using a relative path is a work around for nmake, which doesn't like
# pathnames with spaces even when quoted
file(RELATIVE_PATH OO_SDK_CPPUMAKER_NATIVE ${OO_SDK_URE_BIN} ${OO_SDK_CPPUMAKER} )

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp
    COMMAND ${OO_SDK_CPPUMAKER_NATIVE} -O${PROJECT_BINARY_DIR} -Gc -BUCR ${OO_SDK_RDB}/types.rdb ${OO_SDK_RDB}/services.rdb ${OO_BASIS_PROGRAM}/offapi.rdb
    WORKING_DIRECTORY ${OO_SDK_URE_BIN}
    COMMENT "Generate the OOo API header files from the IDL files - may be slow"
    VERBATIM)


target_link_libraries(graphiteooo ${OO_SDK_UNO_SAL} ${OO_SDK_UNO_CPPU}
 ${OO_SDK_UNO_CPPUHELPER} ${FREETYPE_LIBRARIES} ${FC_LIBRARIES})
target_link_libraries(graphiteooo debug ${GR_DEBUG_LIBRARY})
target_link_libraries(graphiteooo optimized ${GR_RELEASE_LIBRARY})

target_link_libraries(test_graphitefontinfo ${FREETYPE_LIBRARIES}
 ${OO_SDK_UNO_SAL} ${OO_SDK_UNO_CPPU} ${FC_LIBRARIES} ${GR_LIBRARIES} )
target_link_libraries(test_graphitefontinfo debug ${GR_DEBUG_LIBRARY})
target_link_libraries(test_graphitefontinfo optimized ${GR_RELEASE_LIBRARY})

file(GLOB L10N_FILES RELATIVE ${PROJECT_SOURCE_DIR} dialogs/*.properties l10n/*.txt)
foreach (l10nFile ${L10N_FILES} )
    add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/${l10nFile}
            COMMAND ${NATIVE2ASCII} -encoding UTF-8
            ${PROJECT_SOURCE_DIR}/${l10nFile}
            ${PROJECT_BINARY_DIR}/${l10nFile}
            DEPENDS ${PROJECT_SOURCE_DIR}/${l10nFile})
    set(L10N_ESCAPED_FILES ${L10N_ESCAPED_FILES} ${PROJECT_BINARY_DIR}/${l10nFile})
endforeach (l10nFile ${L10N_FILES})

set(ZIP_FILES
    dialogs/GraphiteOptions_en_US.default dialogs/GraphiteOptions.xdl
    dialogs/GraphiteFeatures_en_US.default dialogs/GraphiteFeatures.xdl
    ${L10N_FILES}
    registry/schema/org/sil/graphite/GraphiteOptions.xcs
    registry/data/org/openoffice/Office/Addons.xcu
    registry/data/org/openoffice/Office/Jobs.xcu
    registry/data/org/openoffice/Office/OptionsDialog.xcu
    registry/data/org/openoffice/Office/ProtocolHandler.xcu
    registry/data/org/sil/graphite/GraphiteOptions.xcu
    META-INF/manifest.xml description.xml icons/enabled.png icons/disabled.png
    graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
    graphiteooo.pdb
    ${PLATFORM_LIBRARIES}
)

set(UNO_SHARED_EXTENSION ${CMAKE_SHARED_LIBRARY_SUFFIX})
configure_file(src/uno-extension-manifest.xml ${PROJECT_BINARY_DIR}/META-INF/manifest.xml)
configure_file(src/description.xml ${PROJECT_BINARY_DIR}/description.xml)

add_custom_target(copy_files
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/dialogs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/l10n
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/registry/data/org/sil/graphite
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/icons/enabled.png
        ${PROJECT_BINARY_DIR}/icons/enabled.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/icons/disabled.png
        ${PROJECT_BINARY_DIR}/icons/disabled.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions.xdl
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions.xdl
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions_en_US.default
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions_en_US.default
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteFeatures.xdl
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteFeatures.xdl
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteFeatures_en_US.default
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteFeatures_en_US.default
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/Addons.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/Addons.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/Jobs.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/Jobs.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/ProtocolHandler.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/ProtocolHandler.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/OptionsDialog.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/OptionsDialog.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/sil/graphite/GraphiteOptions.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/sil/graphite/GraphiteOptions.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/schema/org/sil/graphite/GraphiteOptions.xcs
        ${PROJECT_BINARY_DIR}/registry/schema/org/sil/graphite/GraphiteOptions.xcs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${PROJECT_BINARY_DIR}/graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphiteooo.pdb
#        ${PROJECT_BINARY_DIR}/graphiteooo.pdb
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/l10n/ ${PROJECT_BINARY_DIR}/l10n/
    )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    
#    add_custom_target(redist_files
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcr90.dll ${PROJECT_BINARY_DIR}/msvcr90.dll
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcp90.dll ${PROJECT_BINARY_DIR}/msvcp90.dll
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcm90.dll ${PROJECT_BINARY_DIR}/msvcm90.dll
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/Microsoft.VC90.CRT.manifest ${PROJECT_BINARY_DIR}/Microsoft.VC90.CRT.manifest
#        COMMENT "Redistributable Microsoft DLL files")
#    add_dependencies(copy_files redist_files)
#    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcr90.dll)
#    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcp90.dll)
#    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcm90.dll)
#    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo.pdb)
#    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/Microsoft.VC90.CRT.manifest)

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/dialogs)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/l10n)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/registry)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/META-INF)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/description.xml)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo_${EXTENSION_PLATFORM}.oxt)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX})

set(OOO_EXTENSION_FILE ${PROJECT_BINARY_DIR}/graphiteooo_${graphiteooo_VERSION_MAJOR}.${graphiteooo_VERSION_MINOR}.${graphiteooo_VERSION_PATCH}_${EXTENSION_PLATFORM}.oxt)

add_custom_target(ooextension ALL COMMAND ${ZIP} ${OOO_EXTENSION_FILE} ${ZIP_FILES}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR} DEPENDS ${L10N_ESCAPED_FILES})

# clean the headers generated from IDL
add_custom_target(idlclean
    COMMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/com
    COMMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/drafts
    COMMENT clean the IDL generated headers)

add_custom_target(install
    COMMAND ${UNOPKG} add ${UNOPKG_OPTIONS} --shared ${OOO_EXTENSION_FILE}
    COMMENT install the extension system wide - requires admin permissions)

add_custom_target(uninstall
    COMMAND ${UNOPKG} remove --shared org.sil.graphite.GraphiteOptions
    COMMENT install the extension system wide - requires admin permissions)

add_custom_target(install_user
    COMMAND ${UNOPKG} add ${UNOPKG_OPTIONS} ${OOO_EXTENSION_FILE}
    COMMENT install the extension system wide - requires admin permissions)

add_custom_target(uninstall_user
    COMMAND ${UNOPKG} remove org.sil.graphite.GraphiteOptions
    COMMENT install the extension system wide - requires admin permissions)

add_custom_target(reinstall_user
    COMMAND ${UNOPKG} remove org.sil.graphite.GraphiteOptions
    COMMAND ${UNOPKG} add ${UNOPKG_OPTIONS} ${OOO_EXTENSION_FILE}
    COMMENT reinstall the extension for current user)

add_dependencies(copy_files graphiteooo)
add_dependencies(ooextension copy_files)
