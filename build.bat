@echo off
setlocal

set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
set "SOURCE_DIR=%~dp0."
set "BUILD_DIR=%~dp0build"

if not exist "%CMAKE_EXE%" (
  echo CMake not found at "%CMAKE_EXE%"
  exit /b 1
)

"%CMAKE_EXE%" -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" --build "%BUILD_DIR%" --config Release
if errorlevel 1 exit /b 1

echo Build completed. Executable: "%BUILD_DIR%\bin\Release\renderlab.exe"
