set(PROTO_SOURCES
  warrior.proto
  )

foreach (script ${PROTO_SOURCES})
  exec_program(protoc ${ORCAS_SOURCE_DIR}/protocol/meta/
      ARGS "--cpp_out=../cc ${script}")
endforeach (script)
