cmake_minimum_required(VERSION 3.0.0)
project(alis)

include(CTest)
enable_testing()

# SDL2
find_package(SDL2 REQUIRED)

include_directories(src/emu2149)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)
include_directories(src/audio)
include_directories(src/ayumi)
# add_library()

add_executable(alis
               src/emu2149/emu2149.c
               src/sys/sdl2/sys_sdl2.c
               src/audio/audio.c src/audio/music_v1.c src/audio/music_v2.c
               src/main.c
               src/addnames.c src/alis.c src/channel.c src/debug.c src/escnames.c src/image.c 
               src/opcodes.c src/opernames.c src/platform.c src/screen.c src/video.c
               src/script.c src/storenames.c src/unpack.c src/utils.c)


target_link_libraries(alis m)
target_link_libraries(alis ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(src/emu2149)
