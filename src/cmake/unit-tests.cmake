enable_testing ()

set( TEST_COMMON_EXCEPTION  "test-common-exception" )
set( ${TEST_COMMON_EXCEPTION}_objects  tests/common/${TEST_COMMON_EXCEPTION}.cpp )
add_executable(${TEST_COMMON_EXCEPTION} ${${TEST_COMMON_EXCEPTION}_objects} )
target_link_libraries( ${TEST_COMMON_EXCEPTION} ${LIB_DODO} )
add_test (NAME "common::Exception=${TEST_COMMON_EXCEPTION}" COMMAND ${TEST_COMMON_EXCEPTION} )

set( TEST_COMMON_SYSTEMERROR  "test-common-systemerror" )
set( ${TEST_COMMON_SYSTEMERROR}_objects  tests/common/${TEST_COMMON_SYSTEMERROR}.cpp )
add_executable(${TEST_COMMON_SYSTEMERROR} ${${TEST_COMMON_SYSTEMERROR}_objects} )
target_link_libraries( ${TEST_COMMON_SYSTEMERROR} ${LIB_DODO} )
add_test (NAME "common::SystemError=${TEST_COMMON_SYSTEMERROR}" COMMAND ${TEST_COMMON_SYSTEMERROR} )

set( TEST_COMMON_PUTS  "test-common-puts" )
set( ${TEST_COMMON_PUTS}_objects  tests/common/${TEST_COMMON_PUTS}.cpp )
add_executable(${TEST_COMMON_PUTS} ${${TEST_COMMON_PUTS}_objects} )
target_link_libraries( ${TEST_COMMON_PUTS} ${LIB_DODO} )
add_test (NAME "common::Puts=${TEST_COMMON_PUTS}" COMMAND ${TEST_COMMON_PUTS} )

set( TEST_COMMON_BYTES  "test-common-bytes" )
set( ${TEST_COMMON_BYTES}_objects  tests/common/${TEST_COMMON_BYTES}.cpp )
add_executable(${TEST_COMMON_BYTES} ${${TEST_COMMON_BYTES}_objects} )
target_link_libraries( ${TEST_COMMON_BYTES} ${LIB_DODO} )
add_test (NAME "common::Bytes=${TEST_COMMON_BYTES}" COMMAND ${TEST_COMMON_BYTES} )

set( TEST_COMMON_DATACRYPT  "test-common-datacrypt" )
set( ${TEST_COMMON_DATACRYPT}_objects  tests/common/${TEST_COMMON_DATACRYPT}.cpp )
add_executable(${TEST_COMMON_DATACRYPT} ${${TEST_COMMON_DATACRYPT}_objects} )
target_link_libraries( ${TEST_COMMON_DATACRYPT} ${LIB_DODO} )
add_test (NAME "common::DataCrypt=${TEST_COMMON_DATACRYPT}" COMMAND ${TEST_COMMON_DATACRYPT} )

add_test (NAME "common::DataCrypt::cryptstr" COMMAND test "$(echo $(echo 'secret' | bin/cryptstr -e -k val:password) | bin/cryptstr -d -k val:password ) == 'secret'" )

set( TEST_NETWORK_ADDRESS  "test-network-address" )
set( ${TEST_NETWORK_ADDRESS}_objects  tests/network/${TEST_NETWORK_ADDRESS}.cpp )
add_executable(${TEST_NETWORK_ADDRESS} ${${TEST_NETWORK_ADDRESS}_objects} )
target_link_libraries( ${TEST_NETWORK_ADDRESS} ${LIB_DODO} )
add_test (NAME "network::Address=${TEST_NETWORK_ADDRESS}" COMMAND ${TEST_NETWORK_ADDRESS} )

set( TEST_NETWORK_SOCKET  "test-network-socket" )
set( ${TEST_NETWORK_SOCKET}_objects  tests/network/${TEST_NETWORK_SOCKET}.cpp )
add_executable(${TEST_NETWORK_SOCKET} ${${TEST_NETWORK_SOCKET}_objects} )
target_link_libraries( ${TEST_NETWORK_SOCKET} ${LIB_DODO} )
add_test (NAME "network::Socket=${TEST_NETWORK_SOCKET}" COMMAND ${TEST_NETWORK_SOCKET} )

set( TEST_NETWORK_X509  "test-network-x509" )
set( ${TEST_NETWORK_X509}_objects  tests/network/${TEST_NETWORK_X509}.cpp )
add_executable(${TEST_NETWORK_X509} ${${TEST_NETWORK_X509}_objects} )
target_link_libraries( ${TEST_NETWORK_X509} ${LIB_DODO} )
add_test (NAME "network::X509Certificate=${TEST_NETWORK_X509}" COMMAND ${TEST_NETWORK_X509} )

set( TEST_NETWORK_URI  "test-network-uri" )
set( ${TEST_NETWORK_URI}_objects  tests/network/${TEST_NETWORK_URI}.cpp )
add_executable(${TEST_NETWORK_URI} ${${TEST_NETWORK_URI}_objects} )
target_link_libraries( ${TEST_NETWORK_URI} ${LIB_DODO} )
add_test (NAME "network::URI=${TEST_NETWORK_URI}" COMMAND ${TEST_NETWORK_URI} )

set( TEST_NETWORK_PROTOCOL_HTTP  "test-network-protocol-http" )
set( ${TEST_NETWORK_PROTOCOL_HTTP}_objects  tests/network/${TEST_NETWORK_PROTOCOL_HTTP}.cpp )
add_executable(${TEST_NETWORK_PROTOCOL_HTTP} ${${TEST_NETWORK_PROTOCOL_HTTP}_objects} )
target_link_libraries( ${TEST_NETWORK_PROTOCOL_HTTP} ${LIB_DODO} )
add_test (NAME "network::protocol:http=${TEST_NETWORK_PROTOCOL_HTTP}" COMMAND ${TEST_NETWORK_PROTOCOL_HTTP} )

set( TEST_NETWORK_PROTOCOL_STOMP  "test-network-protocol-stomp" )
set( ${TEST_NETWORK_PROTOCOL_STOMP}_objects  tests/network/${TEST_NETWORK_PROTOCOL_STOMP}.cpp )
add_executable(${TEST_NETWORK_PROTOCOL_STOMP} ${${TEST_NETWORK_PROTOCOL_STOMP}_objects} )
target_link_libraries( ${TEST_NETWORK_PROTOCOL_STOMP} ${LIB_DODO} )
add_test (NAME "network::protocol:stomp=${TEST_NETWORK_PROTOCOL_STOMP}" COMMAND ${TEST_NETWORK_PROTOCOL_STOMP} )

add_test (NAME "persist::sqlite" COMMAND bin/sqlite )

add_test (NAME "persist::kvstore" COMMAND bin/kvstore )

set( TEST_NETWORK_TLS  "test-network-tls" )
add_test (NAME "network::TLSContext+TLSSocket=${TEST_NETWORK_TLS}"
          COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests/network/${TEST_NETWORK_TLS}.sh" "${CMAKE_CURRENT_BINARY_DIR}/bin" )