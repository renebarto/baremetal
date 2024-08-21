set script_dir=%~dp0
set script_drive=%script_dir:~0,2%
echo %script_dir%
cd %script_dir%

%script_drive%
tools\startQEMU-image-uart1.bat demo