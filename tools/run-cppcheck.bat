@echo off
set this_dir=%~dp0
set root_dir=%this_dir%..
set log_dir=%root_dir%\output\log

set build_dir=%root_dir%\cppcheck-build-dir
mkdir %build_dir%
mkdir %log_dir%
cppcheck --cppcheck-build-dir=%build_dir% --enable=all --check-level=normal --output-file=%log_dir%\cppcheck.log --checkers-report=%log_dir%\cppcheck.chk -DPLATFORM_BAREMETAL -DBAREMETAL_RPI_TARGET=3 --inline-suppr --platform=native --project=%root_dir%\cmake-BareMetal-RPI3-Debug\compile_commands.json --std=c++17
