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
find_library(OO_SDK_UNO_SAL NAMES uno_sal PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES lib DOC "SAL library")
find_library(OO_SDK_UNO_CPPU NAMES uno_cppu PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES lib DOC "CPPU library")
find_program(OO_SDK_CPPUMAKER cppumaker PATHS ENV OO_SDK_HOME /usr/lib/openoffice/basis3.2/sdk PATH_SUFFIXES bin DOC "IDL compiler")
find_path(OO_SDK_RDB types.rdb PATHS ENV OO_SDK_URE_HOME PATH_SUFFIXES share/misc DOC "URE RDB directory")
find_path(OO_BASIS_PROGRAM offapi.rdb PATHS ENV OO_SDK_HOME PATH_SUFFIXES ../program DOC "Basis Program directory")
find_program(ZIP zip)

include_directories ("${OO_SDK_INCLUDE}")

# Set the OOo Platform defines
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DSAL_UNX=1 -DCPPU_ENV=gcc3)
	set(BUILD_SHARED_LIBS on)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-DSAL_W32=1)
	set(BUILD_SHARED_LIBS on)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DGROOO_DEBUG=1)
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

# add the library
add_library(graphiteooo src/UnixEnvironmentSetter.cxx
    src/graphiteooo.cxx src/GraphiteAddOn.cxx
    src/DialogEventHandler.cxx
    src/GraphiteConfiguration.cxx
    src/SetEnvJob.cxx)

add_executable(test_unixenvset src/UnixEnvironmentSetter.cxx src/test_UnixEnvironmentSetter.cxx)
add_test(NAME "Test test_unixenvset" COMMAND "$<TARGET_FILE:test_unixenvset>")
enable_testing()

# just use one sample IDL header since we compile them in one go
set_source_files_properties(src/GraphiteAddOn.cxx OBJECT_DEPENDS
	${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp
							COMMAND ${OO_SDK_CPPUMAKER} "-O${PROJECT_BINARY_DIR}" -Gc -BUCR ${OO_SDK_RDB}/types.rdb ${OO_SDK_RDB}/services.rdb ${OO_BASIS_PROGRAM}/offapi.rdb
							VERBATIM)

# clean the headers generated from IDL
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_BINARY_DIR}/com;${PROJECT_BINARY_DIR}/cppu;${PROJECT_BINARY_DIR}/msforms;${PROJECT_BINARY_DIR}/ooo;${PROJECT_BINARY_DIR}/drafts")

target_link_libraries(graphiteooo ${OO_SDK_UNO_SAL} ${OO_SDK_UNO_SAL})

set(ZIP_FILES dialogs/GraphiteOptions_en_US.default dialogs/GraphiteOptions_en_US.properties dialogs/GraphiteOptions.xdl
    registry/schema/org/sil/graphite/GraphiteOptions.xcs registry/data/org/openoffice/Office/Addons.xcu
    registry/data/org/openoffice/Office/Events.xcu
    registry/data/org/openoffice/Office/Jobs.xcu
    registry/data/org/openoffice/Office/OptionsDialog.xcu registry/data/org/openoffice/Office/ProtocolHandler.xcu
    META-INF/manifest.xml description.xml
    graphiteooo.uno${CMAKE_SHARED_LIBRARY_SUFFIX}
#    ${CMAKE_SHARED_LIBRARY_PREFIX}graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
)

set(UNO_SHARED_EXTENSION ${CMAKE_SHARED_LIBRARY_SUFFIX})
configure_file(src/uno-extension-manifest.xml ${PROJECT_BINARY_DIR}/META-INF/manifest.xml)
configure_file(src/description.xml ${PROJECT_BINARY_DIR}/description.xml)

add_custom_target(copy_files
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/dialogs
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/registry/data/org/sil/graphite
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions.xdl
		${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions.xdl
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions_en_US.properties 
		${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions_en_US.properties
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/dialogs/GraphiteOptions_en_US.default
		${PROJECT_BINARY_DIR}/dialogs/GraphiteOptions_en_US.default
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/Addons.xcu
		${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/Addons.xcu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/registry/data/org/openoffice/Office/Events.xcu
        ${PROJECT_BINARY_DIR}/registry/data/org/openoffice/Office/Events.xcu
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
		${PROJECT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}graphiteooo${CMAKE_SHARED_LIBRARY_SUFFIX}
		${PROJECT_BINARY_DIR}/graphiteooo.uno${CMAKE_SHARED_LIBRARY_SUFFIX}
	)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/dialogs)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/registry)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/META-INF)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${PROJECT_BINARY_DIR}/description.xml)

add_custom_target(ooextension ALL COMMAND ${ZIP} ${PROJECT_BINARY_DIR}/graphiteooo.oxt ${ZIP_FILES}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_dependencies(ooextension graphiteooo copy_files)
