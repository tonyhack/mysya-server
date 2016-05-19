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
  exec_program(protoc ${ORCAS_SOURCE_DIR}/protocol/meta/
        ARGS "--lua_out=${ORCAS_SOURCE_DIR}/protocol/lua/ ${script}"
              "--plugin=protoc-gen-lua=${TUTORIAL_SOURCE_DIR}/tools/protoc-gen-lua/protoc-gen-lua")
endforeach (script)
