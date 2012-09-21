@echo off

::Change to batch's dir in case run as admin
cd /D "%~dp0"

::Setup paths
set k_originalPath=%PATH%
set k_projectFolder=%CD%\Adjutant

if EXIST "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC" set k_vsDir=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC
if EXIST "C:\Program Files\Microsoft Visual Studio 9.0\VC" set k_vsDir=C:\Program Files\Microsoft Visual Studio 9.0\VC

if NOT EXIST "%k_vsDir%" (
	echo Could not find Visual Studio 2008 directory
	goto error
)

::Setup Environment
call "%k_vsDir%\vcvarsall.bat"
if NOT DEFINED BWAPI_DIR set BWAPI_DIR=%CD%\BWAPI

::Build Adjutant
devenv "%k_projectFolder%\Adjutant.sln" /build Release
if NOT "%ERRORLEVEL%" == "0" (
	echo Unable to compile Adjutant solution
	goto error
)

echo Build Successful

::Error handling
goto end

:error
echo ************************
echo Errors have occurred.
echo ************************
:end
set PATH=%k_originalPath%
pause
exit /B %ERRORLEVEL%