cmake_minimum_required(VERSION 3.0.0...3.30.4)
project(alis)

include(CTest)
enable_testing()

# Comment "set..." to disable output to console for alis on Windows
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()

# SDL2
# If SDL2 package is not found and cannot be installed for any reason, a quick fix can be made:
#    place the devel-version of SDL2 (https://github.com/libsdl-org/SDL/releases) in the src/SDL2 folder
#    (make sure the cmake folder is available in the package extracted to the src/SDL2 folder)
#    and enable the following command:
# set(SDL2_DIR "src/SDL2/cmake")
find_package(SDL2 REQUIRED)

include_directories(src/emu2149)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)
include_directories(src/audio)
# add_library()

list(APPEND alis_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/emu2149/emu2149.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sys/sdl2/sys_sdl2.c
    ${CMAKE_CURRENT_LIST_DIR}/src/audio/audio.c
    ${CMAKE_CURRENT_LIST_DIR}/src/audio/music_v1.c
    ${CMAKE_CURRENT_LIST_DIR}/src/audio/music_v2.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/addnames.c
    ${CMAKE_CURRENT_LIST_DIR}/src/alis.c
    ${CMAKE_CURRENT_LIST_DIR}/src/channel.c
    ${CMAKE_CURRENT_LIST_DIR}/src/debug.c
    ${CMAKE_CURRENT_LIST_DIR}/src/escnames.c
    ${CMAKE_CURRENT_LIST_DIR}/src/image.c
    ${CMAKE_CURRENT_LIST_DIR}/src/opcodes.c
    ${CMAKE_CURRENT_LIST_DIR}/src/opernames.c
    ${CMAKE_CURRENT_LIST_DIR}/src/platform.c
    ${CMAKE_CURRENT_LIST_DIR}/src/screen.c
    ${CMAKE_CURRENT_LIST_DIR}/src/video.c
    ${CMAKE_CURRENT_LIST_DIR}/src/script.c
    ${CMAKE_CURRENT_LIST_DIR}/src/storenames.c
    ${CMAKE_CURRENT_LIST_DIR}/src/unpack.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils.c
    ${CMAKE_CURRENT_LIST_DIR}/src/newalis.c
    ${CMAKE_CURRENT_LIST_DIR}/src/export.c
#   ${CMAKE_CURRENT_LIST_DIR}/src/font.c
)

# Icon for Windows
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
   list(APPEND alis_SOURCES
       ${CMAKE_CURRENT_LIST_DIR}/src/icons/alis.rc)
endif()

add_executable(alis ${alis_SOURCES})

target_link_libraries(alis m)
target_link_libraries(alis ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(src/emu2149)
