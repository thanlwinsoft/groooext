CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
project (graphiteooo)
enable_language(CXX C)

# The version number.
set (graphiteooo_VERSION_MAJOR 0)
set (graphiteooo_VERSION_MINOR 0)

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

include_directories ("${OO_SDK_INCLUDE}")

# Set the OOo Platform defines
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DSAL_UNX=1 -DCPPU_ENV=gcc3)
	set(BUILD_SHARED_LIBS on)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-DSAL_W32=1)
	set(BUILD_SHARED_LIBS on)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

# add the library
add_library(graphiteooo src/graphiteooo.cxx src/GraphiteAddOn.cxx)

# just use one sample IDL header since we compile them in one go
set_source_files_properties(src/GraphiteAddOn.cxx OBJECT_DEPENDS
	${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/com/sun/star/lang/XInitialization.hpp
							COMMAND ${OO_SDK_CPPUMAKER} "-O${PROJECT_BINARY_DIR}" -Gc -BUCR ${OO_SDK_RDB}/types.rdb ${OO_SDK_RDB}/services.rdb ${OO_BASIS_PROGRAM}/offapi.rdb
							VERBATIM)

# clean the headers generated from IDL
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_BINARY_DIR}/com;${PROJECT_BINARY_DIR}/cppu;${PROJECT_BINARY_DIR}/msforms;${PROJECT_BINARY_DIR}/ooo;${PROJECT_BINARY_DIR}/drafts")

target_link_libraries(graphiteooo ${OO_SDK_UNO_SAL} ${OO_SDK_UNO_SAL})
