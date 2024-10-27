@ECHO ###############################################################################
@ECHO Building/Compiling system:   x86_64-pc-cygwin (cygwin64)
@ECHO ###############################################################################
@ECHO Kit:                         GCC 11.3.0 x86_64-pc-cygwin
@ECHO Compilers:                   C = c:\cygwin64\bin\gcc.exe,
@ECHO                              CXX = c:\cygwin64\bin\g++.exe
@ECHO Name:                        GCC 11.3.0 x86_64-pc-cygwin
@ECHO ###############################################################################
@ECHO Cmake:                       Windows x64, version 3.30.4
@ECHO Path:                        C:\CMake\bin\cmake.exe
@ECHO ###############################################################################
@SET SPATH=%~dp0
@SET BPATH=%~dp0build
@SET CMLPATH=%~dp0CMakeLists.txt
@SET APATH=%BPATH%\alis_cygwin.exe
@IF NOT EXIST "%CMLPATH%" (
    @ECHO:
    @ECHO ERROR: CMakeLists.txt is not found in the folder: %SPATH%.
    @EXIT /b 1
)
@ECHO Deleting old cmake files...
@RD  "build\.cmake" /S /Q
@RD  "build\CMakeFiles" /S /Q
@RD  "build\src" /S /Q
@RD  "build\Testing" /S /Q
@DEL "build\CMakeCache.txt"
@DEL "build\cmake_install.cmake"
@DEL "build\compile_commands.json"
@DEL "build\CPackConfig.cmake"
@DEL "build\CPackSourceConfig.cmake"
@DEL "build\CTestTestfile.cmake"
@DEL "build\DartConfiguration.tcl"
@DEL "build\Makefile"
@ECHO Deleting old executable file %APATH%...
@IF EXIST "%APATH%" ( @DEL "%APATH%" )
@ECHO ###############################################################################
@ECHO Configuring the building with Cmake...
@ECHO ###############################################################################
@C:\CMake\bin\cmake.exe "-DCMAKE_PREFIX_PATH=c:\cygwin64\bin" -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=c:\cygwin64\bin\gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=c:\cygwin64\bin\g++.exe --no-warn-unused-cli -S%SPATH% -B%BPATH% -G "Unix Makefiles"
@IF ERRORLEVEL 1 (
    @ECHO:
    @ECHO ERROR: CMake has finished with an error.
    @EXIT /b 1
)
@ECHO ###############################################################################
@ECHO Compilation...
@ECHO ###############################################################################
@CD %BPATH%
@C:\CMake\bin\cmake.exe --build %BPATH%
