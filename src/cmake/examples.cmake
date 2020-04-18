set( EXAMPLE_PING  "ping" )
set( ${EXAMPLE_PING}_objects  src/examples/${EXAMPLE_PING}/${EXAMPLE_PING}.cpp )
add_executable(${EXAMPLE_PING} ${${EXAMPLE_PING}_objects} )
target_link_libraries( ${EXAMPLE_PING} ${LIB_DODO} )

set( EXAMPLE_X509_INFO  "x509-info" )
set( ${EXAMPLE_X509_INFO}_objects  src/examples/tls/${EXAMPLE_X509_INFO}/${EXAMPLE_X509_INFO}.cpp )
add_executable(${EXAMPLE_X509_INFO} ${${EXAMPLE_X509_INFO}_objects} )
target_link_libraries( ${EXAMPLE_X509_INFO} ${LIB_DODO} )

set( EXAMPLE_TLS_SERVER  "tls-server" )
set( ${EXAMPLE_TLS_SERVER}_objects  src/examples/tls/${EXAMPLE_TLS_SERVER}/${EXAMPLE_TLS_SERVER}.cpp )
add_executable(${EXAMPLE_TLS_SERVER} ${${EXAMPLE_TLS_SERVER}_objects} )
target_link_libraries( ${EXAMPLE_TLS_SERVER} ${LIB_DODO} )

set( EXAMPLE_TLS_CLIENT  "tls-client" )
set( ${EXAMPLE_TLS_CLIENT}_objects  src/examples/tls/${EXAMPLE_TLS_CLIENT}/${EXAMPLE_TLS_CLIENT}.cpp )
add_executable(${EXAMPLE_TLS_CLIENT} ${${EXAMPLE_TLS_CLIENT}_objects} )
target_link_libraries( ${EXAMPLE_TLS_CLIENT} ${LIB_DODO} )
