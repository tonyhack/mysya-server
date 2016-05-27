set(PROTO_SOURCES
    ${COMBAT_SOURCE_DIR}/server/vote/meta/vote.proto
    ${COMBAT_SOURCE_DIR}/server/vote/meta/vote_combat.proto
    )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${COMBAT_SOURCE_DIR}/protocol/meta/
      ARGS "--proto_path=${COMBAT_SOURCE_DIR}/server/vote/meta --proto_path=${ORCAS_SOURCE_DIR}/protocol/meta --cpp_out=${COMBAT_SOURCE_DIR}/server/vote/cc ${script}")
endforeach (script)

