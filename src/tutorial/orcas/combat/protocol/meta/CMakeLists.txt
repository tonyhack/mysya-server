set(PROTO_SOURCES
  ${COMBAT_SOURCE_DIR}/protocol/meta/combat_message.proto
  )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${COMBAT_SOURCE_DIR}/protocol/meta/
      ARGS "--proto_path=${ORCAS_SOURCE_DIR}/protocol/meta --proto_path=${COMBAT_SOURCE_DIR}/protocol/meta --cpp_out=${COMBAT_SOURCE_DIR}/protocol/cc ${script}")
endforeach (script)
