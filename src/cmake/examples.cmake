set( EXAMPLE_PING  "ping" )
set( ${EXAMPLE_PING}_objects  src/examples/${EXAMPLE_PING}/${EXAMPLE_PING}.cpp )
add_executable(${EXAMPLE_PING} ${${EXAMPLE_PING}_objects} )
target_link_libraries( ${EXAMPLE_PING} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_PING} RUNTIME DESTINATION bin )

set( EXAMPLE_CRYPTSTR  "cryptstr" )
set( ${EXAMPLE_CRYPTSTR}_objects  src/examples/${EXAMPLE_CRYPTSTR}/${EXAMPLE_CRYPTSTR}.cpp )
add_executable(${EXAMPLE_CRYPTSTR} ${${EXAMPLE_CRYPTSTR}_objects} )
target_link_libraries( ${EXAMPLE_CRYPTSTR} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_CRYPTSTR} RUNTIME DESTINATION bin )

set( EXAMPLE_SYSINFO  "sysinfo" )
set( ${EXAMPLE_SYSINFO}_objects  src/examples/${EXAMPLE_SYSINFO}/${EXAMPLE_SYSINFO}.cpp )
add_executable(${EXAMPLE_SYSINFO} ${${EXAMPLE_SYSINFO}_objects} )
target_link_libraries( ${EXAMPLE_SYSINFO} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_SYSINFO} RUNTIME DESTINATION bin )

set( EXAMPLE_SERVICE  "service" )
set( ${EXAMPLE_SERVICE}_objects  src/examples/${EXAMPLE_SERVICE}/${EXAMPLE_SERVICE}.cpp )
add_executable(${EXAMPLE_SERVICE} ${${EXAMPLE_SERVICE}_objects} )
target_link_libraries( ${EXAMPLE_SERVICE} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_SERVICE} RUNTIME DESTINATION bin )

set( EXAMPLE_X509_INFO  "x509-info" )
set( ${EXAMPLE_X509_INFO}_objects  src/examples/tls/${EXAMPLE_X509_INFO}/${EXAMPLE_X509_INFO}.cpp )
add_executable(${EXAMPLE_X509_INFO} ${${EXAMPLE_X509_INFO}_objects} )
target_link_libraries( ${EXAMPLE_X509_INFO} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_X509_INFO} RUNTIME DESTINATION bin )

set( EXAMPLE_TLS_SERVER  "tls-server" )
set( ${EXAMPLE_TLS_SERVER}_objects  src/examples/tls/${EXAMPLE_TLS_SERVER}/${EXAMPLE_TLS_SERVER}.cpp )
add_executable(${EXAMPLE_TLS_SERVER} ${${EXAMPLE_TLS_SERVER}_objects} )
target_link_libraries( ${EXAMPLE_TLS_SERVER} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_TLS_SERVER} RUNTIME DESTINATION bin )

set( EXAMPLE_TLS_CLIENT  "tls-client" )
set( ${EXAMPLE_TLS_CLIENT}_objects  src/examples/tls/${EXAMPLE_TLS_CLIENT}/${EXAMPLE_TLS_CLIENT}.cpp )
add_executable(${EXAMPLE_TLS_CLIENT} ${${EXAMPLE_TLS_CLIENT}_objects} )
target_link_libraries( ${EXAMPLE_TLS_CLIENT} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_TLS_CLIENT} RUNTIME DESTINATION bin )

set( EXAMPLE_KVSTORE  "kvstore" )
set( ${EXAMPLE_KVSTORE}_objects  src/examples/${EXAMPLE_KVSTORE}/${EXAMPLE_KVSTORE}.cpp )
add_executable(${EXAMPLE_KVSTORE} ${${EXAMPLE_KVSTORE}_objects} )
target_link_libraries( ${EXAMPLE_KVSTORE} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_KVSTORE} RUNTIME DESTINATION bin )

set( EXAMPLE_FINGER  "finger" )
set( ${EXAMPLE_FINGER}_objects  src/examples/${EXAMPLE_FINGER}/${EXAMPLE_FINGER}.cpp )
add_executable(${EXAMPLE_FINGER} ${${EXAMPLE_FINGER}_objects} )
target_link_libraries( ${EXAMPLE_FINGER} ${LIB_DODO} )
install( TARGETS ${EXAMPLE_FINGER} RUNTIME DESTINATION bin )
