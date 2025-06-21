@echo off

setlocal

cd %~dp0

if "%1"=="" (
	goto invalid_arguments
) else if "%2" neq "" (
	goto invalid_arguments
)

if exist "%1" (
	echo Reptest already exists
	goto end
)

mkdir "%1"

copy ..\src\memory.h %1\.
copy ..\src\strings.h %1\.
copy ..\src\virtual_array.h %1\.
copy ..\src\u128.h %1\.
copy ..\src\tokens.h %1\.
copy ..\src\lexer.h %1\.

goto end

:invalid_arguments
echo Invalid arguments. Expected: %0 ^[name of reptest^]
goto end

:end
endlocal
