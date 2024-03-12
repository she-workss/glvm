@echo off

@REM -d - Debug
@REM -r - Release
set COMPILE_MODE_FLAG=%1

if "%COMPILE_MODE_FLAG%" == "-d" (
    set COMPILE_MODE=debug
) else if "%COMPILE_MODE_FLAG%" == "-r" (
    set COMPILE_MODE=release
) else (
    echo [ERROR] Invalid compile mode.
    echo [ERROR] Use -d for debug or -r for release.
    exit /b 1
)

set PROJECT_DIR=%cd%
set BUILD_FOLDER=build
set C_COMPILER_PATH=gcc.exe
set CXX_COMPILER_PATH=g++.exe

echo [INFO] Compiling in %COMPILE_MODE% mode...
cmake --no-warn-unused-cli ^
      -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE ^
      -DCMAKE_BUILD_TYPE:STRING="%COMPILE_MODE%" ^
      -DCMAKE_C_COMPILER:FILEPATH=%C_COMPILER_PATH% ^
      -DCMAKE_CXX_COMPILER:FILEPATH=%CXX_COMPILER_PATH% ^
      "-S%PROJECT_DIR%" ^
      "-B%PROJECT_DIR%/%BUILD_FOLDER%/%COMPILE_MODE%" ^
      -G Ninja
cmake --build "%PROJECT_DIR%/%BUILD_FOLDER%/%COMPILE_MODE%" ^
      --config %COMPILE_MODE% ^
      --target all ^
      -j 8

rd /q bin 2>nul
