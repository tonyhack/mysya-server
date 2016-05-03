set(COMBAT_SOURCE_DIR ${ORCAS_SOURCE_DIR}/combat)

set(COMBAT_SOURCES
  ${TUTORIAL_SOURCE_DIR}/orcas/combat/message_dispatcher.cc
  ${TUTORIAL_SOURCE_DIR}/orcas/combat/transport_channel.cc
  )

set(COMBAT_PROTOCOL_SOURCES
  ${TUTORIAL_SOURCE_DIR}/orcas/combat/protocol/cc/message.pb.cc
  )

add_subdirectory(protocol)
add_subdirectory(server)
add_subdirectory(client)
