set(SRC_MYSYA_PATH ${PROJECT_SOURCE_DIR}/src/mysya)
set(SRC_MYSYA_UTIL_PATH ${SRC_MYSYA_PATH}/util)

set(MYSYA_UTIL_SOURCES
    ${SRC_MYSYA_UTIL_PATH}/exception.cc
    ${SRC_MYSYA_UTIL_PATH}/timestamp.cc)

add_subdirectory(util)
add_subdirectory(ioevent)
add_subdirectory(qservice)
add_subdirectory(codec)
