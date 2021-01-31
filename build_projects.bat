@echo off

set BUILDER_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBUILD.EXE
set SUBMODULE_UPDATE=git submodule update --init
set DIRECTXTEX_SLN=./DirectXTex.sln
set DIRECTXTEX_PATH=./DirectXTex
set EASTL_SLN=./EASTL.sln
set EASTL_PATH=./EASTL

echo ::%SUBMODULE_UPDATE%
For /f "tokens=*" %%A in ('%SUBMODULE_UPDATE%') do echo %%A

if  %ERRORLEVEL% NEQ 0 GOTO :error 
echo. 

if exist %DIRECTXTEX_PATH% (
	echo ::cd %DIRECTXTEX_PATH%
	cd %DIRECTXTEX_PATH%
	echo ::cmake ./
	For /f "tokens=*" %%A in ('cmake ./') do echo %%A
	echo ::%DEVENV_PATH% %DIRECTXTEX_SLN%
	For /f "tokens=*" %%A in ('"%BUILDER_PATH%" %DIRECTXTEX_SLN%') do echo %%A
	cd ../
) else (
	echo ::dxtex not exist..
)
echo. 

if exist %EASTL_PATH% (
	echo ::cd %EASTL_PATH%
	cd %EASTL_PATH%
	echo ::cmake ./
	For /f "tokens=*" %%A in ('cmake ./') do echo %%A
	echo ::%DEVENV_PATH% %EASTL_SLN%
	For /f "tokens=*" %%A in ('"%BUILDER_PATH%" %EASTL_SLN%') do echo %%A
	cd ../
) else (
	echo ::eastl not exist..
)
echo. 

echo ::ALL LIB SETUP END

:error
pause