set(GATEWAY_SOURCE_DIR ${ORCAS_SOURCE_DIR}/gateway)

set(GATEWAY_SOURCES
  ${GATEWAY_SOURCE_DIR}/aes.cc
  ${GATEWAY_SOURCE_DIR}/codec.cc
  )

add_subdirectory(server)
add_subdirectory(robot)
