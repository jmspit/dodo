
find_library( SSL_LIB ssl [ DOC "OpenSSL shared library" ] )
if ( NOT SSL_LIB )
  message( FATAL_ERROR "openssl library not found" )
endif()


find_file( SSL_INCLUDE_DIR opensslv.h
           [ HINTS /usr/include/openssl ]
           [ NO_DEFAULT_PATH ]
           [ DOC "OpenSSL include directory" ] )
if ( NOT SSL_INCLUDE_DIR )
  message( FATAL_ERROR "openssl include dir not found" )
endif()

get_filename_component( SSL_INCLUDE_DIR ${SSL_INCLUDE_DIR} DIRECTORY )
get_filename_component( SSL_LIB_DIR ${SSL_LIB} DIRECTORY )

message( STATUS "Check for openssl include dir: ${SSL_INCLUDE_DIR}" )
message( STATUS "Check for openssl library dir: ${SSL_LIB_DIR}" )

include_directories(${SSL_INCLUDE_DIR})
link_directories(${SSL_LIB_DIR})


