if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/gateway/server)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/gateway/server)

set(GATEWAY_SERVER_SOURCES
    ${GATEWAY_SOURCE_DIR}/server/actor.cc
    ${GATEWAY_SOURCE_DIR}/server/app_server.cc
    ${GATEWAY_SOURCE_DIR}/server/config.cc
    ${GATEWAY_SOURCE_DIR}/server/main.cc
    ${GATEWAY_SOURCE_DIR}/server/message_dispatcher.cc
    )

add_executable(gateway_server ${UTIL_SOURCES} ${GATEWAY_SOURCES} ${GATEWAY_SERVER_SOURCES} ${ORCAS_PROTOCOL_SOURCES})
target_link_libraries(gateway_server mysya-orcas-combat-client mysya-ioevent mysya-codec mysya-orcas-deps protobuf pthread rt dl)
