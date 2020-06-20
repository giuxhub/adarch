@echo off

REM For Visual Studio 2017

REM Create folder and generate VS solution
mkdir vstudio > NUL 2>&1
cd vstudio
cmake .. -G "Visual Studio 15 2017"

REM Open solution in VS
start adarch.sln

pause