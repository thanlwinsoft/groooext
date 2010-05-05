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

# The version number.
set (graphiteooo_VERSION_MAJOR 0)
set (graphiteooo_VERSION_MINOR 0)

execute_process(COMMAND hg id -n OUTPUT_VARIABLE MERCURIAL_VERSION)
string(REPLACE "+" "" MERCURIAL_VERSION ${MERCURIAL_VERSION})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/graphiteoooConfig.h.in"
  "${PROJECT_BINARY_DIR}/graphiteoooConfig.h"
  )

# add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")

# OO_SDK_URE_HOME is setup using the setsdkenv_* scripts
find_path(OO_SDK_IDL com/sun/star/udk-modules.idl PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES idl DOC "OO IDL directory")
find_path(OO_SDK_INCLUDE udkversion.mk PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES include DOC "OO SDK include directory")
find_library(OO_SDK_UNO_SAL NAMES uno_sal isal PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES lib DOC "SAL library")
find_library(OO_SDK_UNO_CPPU NAMES uno_cppu icppu PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES lib DOC "CPPU library")
find_library(OO_SDK_UNO_CPPUHELPER NAMES uno_cppuhelpergcc3 icppuhelper PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES lib DOC "CPPU helper library")

find_program(OO_SDK_CPPUMAKER cppumaker PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES bin DOC "IDL compiler")
find_path(OO_SDK_RDB types.rdb PATHS ENV OO_SDK_URE_HOME ENV OO_SDK_HOME PATH_SUFFIXES share/misc misc DOC "URE RDB directory")
find_path(OO_BASIS_PROGRAM offapi.rdb PATHS ENV OO_SDK_HOME PATH_SUFFIXES ../program DOC "Basis Program directory")
find_path(OO_SDK_URE_BIN version.ini versionrc PATHS ENV OO_SDK_URE_HOME PATH_SUFFIXES bin DOC "URE bin directory")
find_program(ZIP zip)

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
    add_definitions(${FC_CFLAGS} -I${FREETYPE_INCLUDE_DIR_freetype2})
    set(GR_DEBUG_LIBRARY ${GR_LIBRARIES})
    set(GR_RELEASE_LIBRARY ${GR_LIBRARIES})
    set(GROOO_OUTPUT_DIR "")
    set(PLATFORM_LIBRARIES "")
    if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
        set(EXTENSION_PLATFORM linux_x86)
    elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64")
        set(EXTENSION_PLATFORM linux_x86-64)
    endif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DWNT=1 -DCPPU_ENV=msci /wd4290 /wd4800 /W4)
    set(BUILD_SHARED_LIBS on)

    if (${MSVC_VERSION} EQUAL 1500)
        set(PLATFORM_LIBRARIES msvcr90.dll msvcm90.dll msvcp90.dll Microsoft.VC90.CRT.manifest)
    else (${MSVC_VERSION} EQUAL 1500)
        message(SEND_ERROR "Unknown version of MSC ${MSVC_VERSION}")
    endif (${MSVC_VERSION} EQUAL 1500)

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
    src/DialogEventHandler.cxx src/DialogEventHandler.hxx
    src/FeatureDialogEventHandler.cxx src/FeatureDialogEventHandler.hxx
    src/GraphiteFontInfo.cxx src/GraphiteFontInfo.hxx
    src/GraphiteConfiguration.cxx src/GraphiteConfiguration.hxx
    src/SetupContextMenu.cxx src/SetupContextMenu.hxx)

set_source_files_properties(src/graphiteooo.hxx src/UnixEnvironmentSetter.hxx src/GraphiteAddOn.hxx
    src/DialogEventHandler.hxx src/FeatureDialogEventHandler.hxx src/GraphiteFontInfo.hxx
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



set(ZIP_FILES dialogs/GraphiteOptions_en_US.default dialogs/GraphiteOptions_en_US.properties dialogs/GraphiteOptions.xdl
    dialogs/GraphiteFeatures_en_US.default dialogs/GraphiteFeatures_en_US.properties dialogs/GraphiteFeatures.xdl
    registry/schema/org/sil/graphite/GraphiteOptions.xcs registry/data/org/openoffice/Office/Addons.xcu
    registry/data/org/openoffice/Office/Jobs.xcu
    registry/data/org/openoffice/Office/OptionsDialog.xcu registry/data/org/openoffice/Office/ProtocolHandler.xcu
    META-INF/manifest.xml description.xml icons/enabled.png icons/disabled.png
    graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
    graphiteooo.pdb
    l10n/license_en-US.txt
    l10n/description_en-US.txt
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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions_en_US.properties 
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions_en_US.properties
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions_en_US.default
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions_en_US.default
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteFeatures.xdl
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteFeatures.xdl
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteFeatures_en_US.properties 
        ${PROJECT_BINARY_DIR}/dialogs/GraphiteFeatures_en_US.properties
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
    add_custom_target(redist_files
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcr90.dll ${PROJECT_BINARY_DIR}/msvcr90.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcp90.dll ${PROJECT_BINARY_DIR}/msvcp90.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/msvcm90.dll ${PROJECT_BINARY_DIR}/msvcm90.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MSVCDLL_PATH}/Microsoft.VC90.CRT.manifest ${PROJECT_BINARY_DIR}/Microsoft.VC90.CRT.manifest
        COMMENT "Redistributable Microsoft DLL files")
    add_dependencies(copy_files redist_files)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcr90.dll)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcp90.dll)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/msvcm90.dll)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo.pdb)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/Microsoft.VC90.CRT.manifest)

endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/dialogs)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/l10n)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/registry)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/META-INF)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/description.xml)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo_${EXTENSION_PLATFORM}.oxt)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX})

add_custom_target(ooextension ALL COMMAND ${ZIP} ${PROJECT_BINARY_DIR}/graphiteooo_${EXTENSION_PLATFORM}.oxt ${ZIP_FILES}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# clean the headers generated from IDL
add_custom_target(idlclean
    COMMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/com
    COMMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/drafts
    COMMENT clean the IDL generated headers)

add_dependencies(copy_files graphiteooo)
add_dependencies(ooextension copy_files)
