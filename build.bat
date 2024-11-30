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
set "common_link_options= /incremental:no /opt:ref /subsystem:console"

if "%1"=="debug" (
	set "compile_options=%common_compile_options% /Od /Zo /Z7 /RTC1 /MTd"
	set "link_options=%common_link_options% /DEBUG:FULL libucrtd.lib libvcruntimed.lib"
) else if "%1"=="release" (
	set "compile_options=%common_compile_options% /O2"
	set "link_options=%common_link_options% libvcruntime.lib"
) else (
  goto invalid_arguments
)

if "%2" neq "" goto invalid_arguments

cl %compile_options% ..\src\main.c /link %link_options% /pdb:rin.pdb /out:rin.exe

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build ^[debug or release^]
goto end

:end
endlocal
