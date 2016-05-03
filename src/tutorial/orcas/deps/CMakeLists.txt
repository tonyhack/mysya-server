set(DEPS_TINYXML_SOURCES
  ${ORCAS_SOURCE_DIR}/deps/tinyxml/tinystr.cpp
  ${ORCAS_SOURCE_DIR}/deps/tinyxml/tinyxml.cpp
  ${ORCAS_SOURCE_DIR}/deps/tinyxml/tinyxmlerror.cpp
  ${ORCAS_SOURCE_DIR}/deps/tinyxml/tinyxmlparser.cpp)

set(ALL_DEPS_FILE
  ${DEPS_TINYXML_SOURCES}
  )

add_library(mysya-orcas-deps SHARED ${ALL_DEPS_FILE})

