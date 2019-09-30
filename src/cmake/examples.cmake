set( EXAMPLE_PING  "ping" )
set( ${EXAMPLE_PING}_objects  src/examples/${EXAMPLE_PING}/${EXAMPLE_PING}.cpp )
add_executable(${EXAMPLE_PING} ${${EXAMPLE_PING}_objects} )
target_link_libraries( ${EXAMPLE_PING} ${LIB_DODO} )