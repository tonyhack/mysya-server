set(PROTO_SOURCES
  building.proto
  warrior.proto
  message.proto
  )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${ORCAS_SOURCE_DIR}/protocol/meta/
      ARGS "--cpp_out=../cc ${script}")
endforeach (script)
