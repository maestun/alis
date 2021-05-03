cmake_minimum_required(VERSION 3.0.0)
project(alis VERSION 0.1.0)

include(CTest)
enable_testing()

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


add_executable(alis alis/main.c alis/depack/asm.c alis/sys/sdl2/sys_sdl2.c alis/addnames.c
               alis/addnames.c alis/alis.c alis/debug.c alis/escnames.c alis/opcodes.c alis/opernames.c 
               alis/platform.c alis/script.c alis/storenames.c alis/utils.c)

target_link_libraries(alis ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
