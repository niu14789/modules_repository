@echo off
set 
UV=D:/Keil_v5/UV4/UV4.exe

set UV_PRO_PATH = template.uvprojx
echo building ...

%UV% -j0 -b %UV_PRO_PATH% -o build_log.txt

type build_log.txt

echo Done.

pause
