set script_dir=%~dp0
set script_drive=%script_dir:~0,2%
echo %script_dir%

%script_drive%
cd %script_dir%
python tools\fix-links.py
call tools\create-links.bat
call tools\run-doxygen.bat