set(PROTO_SOURCES
  ${COMBAT_SOURCE_DIR}/server/require/meta/require.proto
  ${COMBAT_SOURCE_DIR}/server/require/meta/require_scene.proto
  ${COMBAT_SOURCE_DIR}/server/require/meta/require_formula.proto
  )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${COMBAT_SOURCE_DIR}/protocol/meta/
      ARGS "--proto_path=${COMBAT_SOURCE_DIR}/server/require/meta --proto_path=${ORCAS_SOURCE_DIR}/protocol/meta --cpp_out=${COMBAT_SOURCE_DIR}/server/require/cc ${script}")
endforeach (script)
