@echo off
setlocal

set "input_file=%~f2"

cd %~dp0
cd ..

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

if "%1"=="" (
	goto invalid_arguments
) else if "%input_file%"=="" (
	goto invalid_arguments
) else if "%3" neq "" (
	goto invalid_arguments
)

if not exist "..\tools\%1" (
	echo Reptest does not exist
	goto end
)

if not exist "%input_file%" (
	echo Reptest does not exist
	goto end
)

set "common_compile_options= /nologo /W4 /arch:AVX2 /wd4201"
set "common_link_options= /incremental:no /opt:ref /subsystem:console"

set "compile_options=%common_compile_options% /O2 /Z7 /Zo"
set "link_options=%common_link_options% libvcruntime.lib"

cl %compile_options% ..\tools\lexer_reptester.c /I"..\tools\%1" /link %link_options% /pdb:lexer_reptester.pdb /out:lexer_reptester.exe

call lexer_reptester %input_file%

goto end

:invalid_arguments
echo Invalid arguments^. Expected: %0 ^[reptest name^] ^[input file^]
goto end

:end
endlocal
