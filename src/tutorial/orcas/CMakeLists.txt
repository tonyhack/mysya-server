set(ORCAS_SOURCE_DIR ${TUTORIAL_SOURCE_DIR}/orcas)

set(UTIL_SOURCES
  ${ORCAS_SOURCE_DIR}/util/system_functions.cc
  )

set(ORCAS_PROTOCOL_SOURCES
  ${ORCAS_SOURCE_DIR}/protocol/cc/building.pb.cc
  ${ORCAS_SOURCE_DIR}/protocol/cc/combat.pb.cc
  ${ORCAS_SOURCE_DIR}/protocol/cc/message.pb.cc
  ${ORCAS_SOURCE_DIR}/protocol/cc/position.pb.cc
  ${ORCAS_SOURCE_DIR}/protocol/cc/role.pb.cc
  ${ORCAS_SOURCE_DIR}/protocol/cc/warrior.pb.cc
  )

include_directories(${ORCAS_SOURCE_DIR}/protocol/cc)

add_subdirectory(protocol)
add_subdirectory(deps)
add_subdirectory(combat)
add_subdirectory(gateway)
# add_subdirectory(gateway)
