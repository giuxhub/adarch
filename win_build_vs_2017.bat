@echo off

REM For Windows + Visual Studio 2017


REM Project name
set "VS_PROJ=adarch"

REM Set VS installation folder (edit it if the your is different)
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\"

REM Set VS build type ("Debug" or "Release"), because VS ignores CMake settings
set VS_BUILD=Release


REM Create folder and generate VS solution by CMake
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017"

REM Remember the build directory
set "VS_BUILD_DIR=%CD%"

REM Prepare to build VS solution and return into build directory
@call "%VS_PATH%Common7\Tools\VsDevCmd.bat"
cd /d %VS_BUILD_DIR%

REM Build VS solution
devenv /build %VS_BUILD% "%VS_PROJ%.sln"

pause