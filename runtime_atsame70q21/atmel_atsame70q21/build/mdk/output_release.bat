@echo off
set exe_path=%cd%
cd %exe_path%
cd ../../../../common/tools
UV4.exe %exe_path%\debug\debug.hex %exe_path%\..\..\output\release.bin -b