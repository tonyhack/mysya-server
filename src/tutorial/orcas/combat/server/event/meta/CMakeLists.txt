set(PROTO_SOURCES
  ${COMBAT_SOURCE_DIR}/server/event/meta/event.proto
  ${COMBAT_SOURCE_DIR}/server/event/meta/event_combat.proto
    )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${COMBAT_SOURCE_DIR}/protocol/meta/
      ARGS "--proto_path=${COMBAT_SOURCE_DIR}/server/event/meta --cpp_out=${COMBAT_SOURCE_DIR}/server/event/cc ${script}")
endforeach (script)
