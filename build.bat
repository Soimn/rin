@echo off

setlocal

cd %~dp0

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

set "common_compile_options= /nologo /W3"
set "common_link_options= /incremental:no /opt:ref /subsystem:console libvcruntime.lib"

set "compile_options=%common_compile_options% /Od /Zo /Z7 /RTC1 /MTd"
set "link_options=%common_link_options% libucrtd.lib libvcruntimed.lib"

if "%1" neq "" goto invalid_arguments

cl %compile_options% ..\src\main.c /link %link_options% /pdb:main.pdb /out:main.exe

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build
goto end

:end
endlocal
