add_subdirectory(gen)

set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)
set(DECODE_HDR ${GEN_DIR}/decode.hpp)

add_custom_command(
  OUTPUT ${DECODE_HDR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}
  COMMAND fastdumpgen ${DECODE_HDR}
  DEPENDS fastdumpgen
  COMMENT "Genered decode header"
)

add_executable(fastdump fastdump.cpp ${DECODE_HDR})

target_include_directories(fastdump PRIVATE ${GEN_DIR})
