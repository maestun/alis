cmake_minimum_required(VERSION 3.0.0)
project(alis)

include(CTest)
enable_testing()

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


add_executable(alis src/main.c src/depack/asm.c src/sys/sdl2/sys_sdl2.c src/addnames.c
               src/addnames.c src/alis.c src/debug.c src/escnames.c src/opcodes.c src/opernames.c 
               src/platform.c src/script.c src/storenames.c src/utils.c)

target_link_libraries(alis ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
