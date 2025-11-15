@echo off
setlocal

set BDIR=%~dp0project

cmake --build "%BDIR%" --config Debug

endlocal