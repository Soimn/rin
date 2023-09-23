@echo off

setlocal

cd %~dp0

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

set "ignored_warnings= -Wno-c2x-compat -Wno-strict-prototypes"
set "warnings= -Wall -Wextra -Wshadow -Wconversion -Werror"
set "common_compile_options= -std=c99 -fsanitize=address,undefined %warnings% %ignored_warnings%"
set "common_ignored_warnings= -Wno-missing-prototypes"

set "compile_options=%common_compile_options% %common_ignored_warnings%"

if "%1" neq "" goto invalid_arguments

clang %compile_options% ../src/main.c -fuse-ld=lld-link.exe -lclang_rt.builtins-x86_64.lib -g -gcodeview -Wl,/opt:icf,/fixed,/incremental:no,/opt:ref,/subsystem:console,/debug,/pdb:main.pdb

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build
goto end

:end
endlocal
