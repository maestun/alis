cmake_minimum_required(VERSION 3.10)
project(alis_disassembler)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(ALIS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
else()
    set(ALIS_ROOT_DIR ${CMAKE_SOURCE_DIR})
endif()

add_custom_command(
    OUTPUT disasm_generated.c
    COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_disasm.py ${ALIS_ROOT_DIR}/src/opcodes.c disasm_generated.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen_disasm.py ${ALIS_ROOT_DIR}/src/opcodes.c
)

add_executable(disassembler
    main.c
    disasm_formatter.c
    disasm_generated.c
)

target_include_directories(disassembler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
