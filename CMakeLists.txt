cmake_minimum_required(VERSION 3.1.0)

set (CMAKE_CXX_STANDARD 17)

set( PROJECT "dodo" )
project(${PROJECT})

set( PROJECT_VERSION_MAJOR "0" )
set( PROJECT_VERSION_MINOR "0" )
set( PROJECT_VERSION_PATCH "1" )

set( PROJECT_VERSION_STR "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# compiler settings
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wconversion -pthread")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib" )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin" )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/include/common/buildenv.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/buildenv.hpp @ONLY)

# targets
set( LIB_DODO  "dodo" )
set( LIB_DODO_STATIC  "dodo-static" )

set( ${LIB_DODO}_objects
  src/lib/common/exception.cpp
  src/lib/common/unittest.cpp
  src/lib/common/util.cpp
  src/lib/network/address.cpp
  src/lib/network/basesocket.cpp
  src/lib/network/socket.cpp
  src/lib/network/x509cert.cpp
  src/lib/network/tlscontext.cpp
  src/lib/network/tlssocket.cpp
  src/lib/threads/mutex.cpp
  src/lib/threads/thread.cpp
)

add_library( ${LIB_DODO} SHARED ${${LIB_DODO}_objects} )
add_library( ${LIB_DODO_STATIC} ${${LIB_DODO}_objects} )
target_link_libraries( ${LIB_DODO} crypto ssl pthread )
target_link_libraries( ${LIB_DODO_STATIC} crypto ssl pthread )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/include )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# openssl
include(src/cmake/findopenssl.cmake)

# test
include(src/cmake/unit-tests.cmake)

# examples
include(src/cmake/examples.cmake)

site_name( SITE_NAME )
message( STATUS "PROJECT                    : ${PROJECT}-${PROJECT_VERSION_STR}")
message( STATUS "SITE_NAME                  : ${SITE_NAME}")
message( STATUS "CMAKE_SYSTEM               : ${CMAKE_SYSTEM}")
message( STATUS "CMAKE_SYSTEM_NAME          : ${CMAKE_SYSTEM_NAME}")
message( STATUS "CMAKE_SYSTEM_PROCESSOR     : ${CMAKE_SYSTEM_PROCESSOR}")
message( STATUS "CMAKE_HOST_SYSTEM          : ${CMAKE_HOST_SYSTEM}")
message( STATUS "CMAKE_VERSION              : ${CMAKE_VERSION}")
message( STATUS "CMAKE_C_COMPILER           : ${CMAKE_C_COMPILER}")
message( STATUS "CMAKE_C_COMPILER_ID        : ${CMAKE_C_COMPILER_ID}")
message( STATUS "CMAKE_C_COMPILER_VERSION   : ${CMAKE_C_COMPILER_VERSION}")
message( STATUS "CMAKE_CXX_COMPILER         : ${CMAKE_CXX_COMPILER}")
message( STATUS "CMAKE_CXX_COMPILER_ID      : ${CMAKE_CXX_COMPILER_ID}")
message( STATUS "CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

get_property(include_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
get_property(library_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
message( STATUS "include directories: ${include_dirs}")
message( STATUS "library directories: ${library_dirs}")

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message( STATUS "Doxygen configured")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doxygen && ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)