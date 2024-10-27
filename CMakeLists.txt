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
#    place the devel-version of SDL2 in the src/SDL2 folder (make sure the cmake folder is available
#    in the package extracted to the src/SDL2 folder). Correct paths in sdl2-config.cmake if neccessary.
#
# Example when Cmake for Windows is used and the compiler is Cygwin/MinGW/MinGW UCRT64, arch x86_64:
# Uncomment the next command (SDL2_MANUAL_SET = 1) and fix the path(s) below in the section between START and END.
#  set(SDL2_MANUAL_SET "1")

#  set(SDL2_DIR "src/SDL2/cmake")

# START
if(SDL2_MANUAL_SET)
if(WIN32)
    execute_process(
       COMMAND ${CMAKE_C_COMPILER} -dumpmachine
       OUTPUT_VARIABLE ALIS_COMP
    )
endif()

string(TOLOWER "${CMAKE_C_COMPILER}" UCRT_COMP)
string(REGEX MATCH "cygwin" CYGWIN_COMP   "${ALIS_COMP}")
string(REGEX MATCH "mingw"  MINGW_COMP    "${ALIS_COMP}")
string(REGEX MATCH "msys"   MSYS_COMP     "${ALIS_COMP}")
string(REGEX MATCH "ucrt64" UCRT_COMP     "${UCRT_COMP}")

if(CYGWIN_COMP)
   # Devel-version of SDL2 ver.2.30.8 for Cygwin
   # https://mirrors.evoluso.com/cygwin/x86_64/release/SDL2/libSDL2-devel/
   set(SDL2_DIR "src/SDL2/libSDL2-devel-2.30.8-1/lib/cmake/SDL2")
   message(STATUS "CYGWIN => ${ALIS_COMP}")
   set(ALIS_COMP ${CYGWIN_COMP})
elseif(MINGW_COMP)
   # Devel-version of SDL2 ver.2.30.8 for MinGW
   # https://github.com/libsdl-org/SDL/releases
   set(SDL2_DIR "src/SDL2/x86_64-w64-mingw32/lib/cmake/SDL2")
   if(UCRT_COMP)
      message(STATUS "MINGW (UCRT64) => ${ALIS_COMP}")
      set(ALIS_COMP ${MINGW_COMP}_${UCRT_COMP})
   else(UCRT_COMP)
      message(STATUS "MINGW => ${ALIS_COMP}")
      set(ALIS_COMP ${MINGW_COMP})
   endif(UCRT_COMP)
else()
   set(SDL2_MANUAL_SET "0")
endif()
endif(SDL2_MANUAL_SET)
# END

find_package(SDL2 REQUIRED)

include_directories(src/emu2149)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src)
include_directories(src/audio)
# add_library()

list(APPEND alis_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/emu2149/emu2149.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sys/sys_sdl2.c
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

add_executable(${PROJECT_NAME} ${alis_SOURCES})

target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

if(SDL2_MANUAL_SET)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_${ALIS_COMP})
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(src/emu2149)
