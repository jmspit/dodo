set( EXAMPLE_PING  "ping" )
set( ${EXAMPLE_PING}_objects  src/examples/${EXAMPLE_PING}/${EXAMPLE_PING}.cpp )
add_executable(${EXAMPLE_PING} ${${EXAMPLE_PING}_objects} )
target_link_libraries( ${EXAMPLE_PING} ${LIB_DODO} )

set( EXAMPLE_SSL_SERVER  "ssl-server" )
set( ${EXAMPLE_SSL_SERVER}_objects  src/examples/ssl/${EXAMPLE_SSL_SERVER}/${EXAMPLE_SSL_SERVER}.cpp )
add_executable(${EXAMPLE_SSL_SERVER} ${${EXAMPLE_SSL_SERVER}_objects} )
target_link_libraries( ${EXAMPLE_SSL_SERVER} ${LIB_DODO} )

set( EXAMPLE_SSL_CLIENT  "ssl-client" )
set( ${EXAMPLE_SSL_CLIENT}_objects  src/examples/ssl/${EXAMPLE_SSL_CLIENT}/${EXAMPLE_SSL_CLIENT}.cpp )
add_executable(${EXAMPLE_SSL_CLIENT} ${${EXAMPLE_SSL_CLIENT}_objects} )
target_link_libraries( ${EXAMPLE_SSL_CLIENT} ${LIB_DODO} )
