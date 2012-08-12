@echo off

::Change to batch's dir in case run as admin
cd /D "%~dp0"

::Check for admin permissions
set k_adminTest=%windir%\AdminTestDir_569238105
rmdir /Q "%k_adminTest%" > nul 2>&1
mkdir "%k_adminTest%" > nul 2>&1

if "%ERRORLEVEL%" == "0" (
	rmdir /Q "%k_adminTest%" > nul
) else (
	echo You must have admin privleges to run this. For windows 7 use Run As Administrator.
	goto error
)

::Setup paths
set k_originalDir=%CD%
set k_originalPath=%PATH%
set k_projectFolder=%CD%\Adjutant

if EXIST "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC" set k_vsDir=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC
if EXIST "C:\Program Files \Microsoft Visual Studio 9.0\VC" set k_vsDir=C:\Program Files \Microsoft Visual Studio 9.0\VC

if EXIST "C:\Program Files (x86)\Chaoslauncher" set k_chaosLauncherDir=C:\Program Files (x86)\Chaoslauncher
if EXIST "C:\Program Files\Chaoslauncher" set k_chaosLauncherDir=C:\Program Files\Chaoslauncher

if EXIST "C:\Program Files (x86)\Starcraft" set k_starcraftDir=C:\Program Files (x86)\Starcraft
if EXIST "C:\Program Files\Starcraft" set k_starcraftDir=C:\Program Files\Starcraft

if NOT EXIST "%k_vsDir%" (
	echo Could not find Visual Studio 2008 directory
	goto error
)

if NOT EXIST "%k_chaosLauncherDir%" (
	echo Could not find Chaoslauncher directory
	goto error
)

if NOT EXIST "%k_starcraftDir%" (
	echo Could not find Starcraft directory
	goto error
)

::Setup Environment
call "%k_vsDir%\vcvarsall.bat"
set PATH=%PATH%;%CD%\BWAPI\WINDOWS
if NOT DEFINED BWAPI_LIB set BWAPI_DIR=%CD%\BWAPI

::Build Adjutant
devenv "%k_projectFolder%\Adjutant.sln" /build Debug
if NOT "%ERRORLEVEL%" == "0" (
	echo Unable to compile Adjutant solution
	goto error
)

::Copy built AI module and bwapi.ini
XCOPY /Y "%k_projectFolder%\Debug\AdjutantD.dll" "%k_starcraftDir%\bwapi-data\AI\"
if NOT "%ERRORLEVEL%" == "0" goto error
XCOPY /Y "%k_projectFolder%\bwapi.ini" "%k_starcraftDir%\bwapi-data\"
if NOT "%ERRORLEVEL%" == "0" goto error

::Execute chaos launcher
cd /D "%k_chaosLauncherDir%"
"%k_chaosLauncherDir%\Chaoslauncher.exe"

::Error handling
goto end

:error
echo ************************
echo Errors have occurred.
echo ************************

:end
cd /D "%k_originalDir%"
set PATH=%k_originalPath%
if NOT "%ERRORLEVEL%" == "0" pause
exit /B %ERRORLEVEL%