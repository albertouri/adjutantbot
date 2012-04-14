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

if EXIST "C:\Program Files (x86)\Chaoslauncher" set k_chaosLauncherDir=C:\Program Files (x86)\Chaoslauncher
if EXIST "C:\Program Files\Chaoslauncher" set k_chaosLauncherDir=C:\Program Files\Chaoslauncher

if NOT EXIST "%k_chaosLauncherDir%" (
	echo Could not find Chaoslauncher directory
)

::Execute chaos launcher
set PATH=%PATH%;%CD%\BWAPI 3.7.2\WINDOWS
cd /D "%k_chaosLauncherDir%"
"%k_chaosLauncherDir%\Chaoslauncher - MultiInstance.exe"

:error
echo ************************
echo Errors have occurred.
echo ************************

:end
if NOT "%ERRORLEVEL%" == "0" pause
exit /B %ERRORLEVEL%
