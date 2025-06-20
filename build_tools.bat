@echo off

setlocal

cd %~dp0

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

set "common_compile_options= /nologo /W4 /arch:AVX2 /wd4201"
set "common_link_options= /incremental:no /opt:ref /subsystem:console"

if "%1"=="debug" (
	set "compile_options=%common_compile_options% /Od /Z7 /Zo /RTC1"
	set "link_options=%common_link_options% /DEBUG:FULL libucrtd.lib libvcruntimed.lib"
) else if "%1"=="release" (
	set "compile_options=%common_compile_options% /O2 /Z7 /Zo"
	set "link_options=%common_link_options% libvcruntime.lib"
) else (
	goto invalid_arguments
)

if "%2" neq "" goto invalid_arguments

cl %compile_options% ..\tools\lex_crawler.c /link %link_options% /pdb:lex_crawler.pdb /out:lex_crawler.exe
cl %compile_options% ..\tools\lexer_perf_test.c /link %link_options% /pdb:lexer_perf_test.pdb /out:lexer_perf_test.exe

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build ^[debug/release^]
goto end

:end
endlocal
