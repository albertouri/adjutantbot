@echo off

::Change to batch's dir in case run as admin
cd /D "%~dp0"

::Setup paths
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
set PATH=%PATH%;%CD%\BWAPI 3.7.2\WINDOWS

::Build Adjutant
devenv "%k_projectFolder%\Adjutant.sln" /build Release
if NOT "%ERRORLEVEL%" == "0" (
	echo Unable to compile Adjutant solution
	goto error
)

::Copy built AI module and bwapi.ini
XCOPY /Y "%k_projectFolder%\Release\AdjutantAIModule.dll" "%k_starcraftDir%\bwapi-data\AI\"
XCOPY /Y "%k_projectFolder%\bwapi.ini" "%k_starcraftDir%\bwapi-data\"

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
exit /B %ERRORLEVEL%