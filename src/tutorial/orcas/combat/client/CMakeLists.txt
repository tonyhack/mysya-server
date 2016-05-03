set(COMBAT_CLIENT_SOURCES
  ${TUTORIAL_SOURCE_DIR}/orcas/combat/client/combat_session.cc
  ${TUTORIAL_SOURCE_DIR}/orcas/combat/client/combat_sessions.cc
  )

add_library(mysya-orcas-combat-client SHARED ${COMBAT_CLIENT_SOURCES})
