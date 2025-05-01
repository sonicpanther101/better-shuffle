@echo off
SET SCRIPT_DIR=%~dp0

cd /d "%SCRIPT_DIR%"
IF NOT EXIST "build" (
    mkdir build
)
cd build
cmake ..