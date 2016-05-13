set(PROTO_SOURCES
  building.proto
  combat.proto
  message.proto
  position.proto
  warrior.proto
  )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${ORCAS_SOURCE_DIR}/protocol/meta/
      ARGS "--cpp_out=../cc ${script}")
endforeach (script)
