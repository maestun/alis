@echo off
echo ###############################################################################
echo Building/Compiling system:   x86_64-pc-cygwin (cygwin64)
echo ###############################################################################
echo Kit:                         GCC 11.3.0 x86_64-pc-cygwin
echo Compilers:                   C = c:\cygwin64\bin\gcc.exe,
echo                              CXX = c:\cygwin64\bin\g++.exe
echo Name:                        GCC 11.3.0 x86_64-pc-cygwin
echo ###############################################################################
echo Cmake:                       Windows x64, version 3.30.4
echo Path:                        C:\CMake\bin\cmake.exe
echo ###############################################################################

for %%Q in ("%~dp0\.") do set "PROJECT_PATH=%%~fQ"
set BUILD_PATH=%PROJECT_PATH%\build
set CMAKELIST_PATH=%PROJECT_PATH%\CMakeLists.txt
set OLD_ALIS_EXE_PATH=%BUILD_PATH%\alis_cygwin.exe
rem ###############################################################################
rem   Configure your paths for CMake:
rem ###############################################################################
set PREFIX_PATH=c:\cygwin64\bin
set C_COMPILER=%PREFIX_PATH%\gcc.exe
set CXX_COMPILER=%PREFIX_PATH%\g++.exe
set GENERATOR=Unix Makefiles
set CMAKE_PATH=C:\CMake\bin\cmake.exe
rem ###############################################################################

if not exist "%CMAKELIST_PATH%" (
   echo:
   echo ERROR: CMakeLists.txt is not found in the folder: %PROJECT_PATH%.
   exit /b 1
)

echo Deleting old cmake files...
rd  "build\.cmake" /S /Q
rd  "build\CMakeFiles" /S /Q
rd  "build\src" /S /Q
rd  "build\Testing" /S /Q
del "build\CMakeCache.txt"
del "build\cmake_install.cmake"
del "build\compile_commands.json"
del "build\CPackConfig.cmake"
del "build\CPackSourceConfig.cmake"
del "build\CTestTestfile.cmake"
del "build\DartConfiguration.tcl"
del "build\Makefile"

echo Deleting old executable file %OLD_ALIS_EXE_PATH%...
if exist "%OLD_ALIS_EXE_PATH%" ( del "%OLD_ALIS_EXE_PATH%" )

echo ###############################################################################
echo   Configuring the building with Cmake...
echo ###############################################################################
%CMAKE_PATH% "-DCMAKE_PREFIX_PATH=%PREFIX_PATH%"^
 -DCMAKE_BUILD_TYPE:STRING=Debug^
 -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE^
 "-DCMAKE_C_COMPILER:FILEPATH=%C_COMPILER%"^
 "-DCMAKE_CXX_COMPILER:FILEPATH=%CXX_COMPILER%"^
 --no-warn-unused-cli^
 "-S%PROJECT_PATH%"^
 "-B%BUILD_PATH%"^
 -G "%GENERATOR%"
rem ###############################################################################
if errorlevel 1 (
   echo:
   echo ERROR: CMake has finished with an error.
   exit /b 1
)

echo ###############################################################################
echo   Building / Compiling...
echo ###############################################################################
cd %BUILD_PATH%
%CMAKE_PATH% --build "%BUILD_PATH%"
if errorlevel 1 (
   echo:
   echo ERROR: CMake has finished with an error.
   exit /b 1
)
