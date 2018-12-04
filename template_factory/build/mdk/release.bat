@echo off
set exe_path=%cd%
cd %exe_path%
cd ../../../../common/tools
stack_allocate_release.exe %exe_path%\..\..\output\debug.bin %exe_path%\..\..\output\release.bin %exe_path%\..\..\output\factory.ok 1