@echo off

::Change to batch's dir in case run as admin
cd /D "%~dp0"

::Setup paths
set k_originalDir=%CD%
set k_originalPath=%PATH%
set k_ahkPath=%CD%\AutoHotKeyScripts
set k_bwamPath=%CD%\BroodwarAutoMatchup
set k_outClient=Output\Client
set k_outHost=Output\Host

if EXIST "C:\Program Files (x86)\AutoHotkey\Compiler\Ahk2Exe.exe" set k_ahkCompiler=C:\Program Files (x86)\AutoHotkey\Compiler\Ahk2Exe.exe
if EXIST "C:\Program Files\AutoHotkey\Compiler\Ahk2Exe.exe" set k_ahkCompiler=C:\Program Files\AutoHotkey\Compiler\Ahk2Exe.exe

if NOT EXIST "%k_ahkCompiler%" (
	echo Could not find AutoHotkey compiler
	goto error
)

javac > nul 2>&1
if "%ERRORLEVEL%" == "9009" (
	echo Java SDK must be installed
	goto error
)

::Build BroodwarAutoMatchup
javac "%k_bwamPath%\bwam\controller.java" "%k_bwamPath%\bwam\host.java" "%k_bwamPath%\bwam\client.java"
if NOT "%ERRORLEVEL%" == "0" goto error

::Build AutoHotKey Scripts
"%k_ahkCompiler%" /in "%k_ahkPath%\StartChaoslauncher.ahk"
if NOT "%ERRORLEVEL%" == "0" goto error
"%k_ahkCompiler%" /in "%k_ahkPath%\StartGame.ahk"
if NOT "%ERRORLEVEL%" == "0" goto error

::Copy built files
XCOPY /Y "%k_bwamPath%\bwam\*.class" "%k_outHost%\bwam\"
XCOPY /Y "%k_bwamPath%\bwam\*.class" "%k_outClient%\bwam\"
XCOPY /Y "%k_ahkPath%\*.exe" "%k_outHost%\"
XCOPY /Y "%k_ahkPath%\*.exe" "%k_outClient%\"
XCOPY /Y "bwapi.ini" "%k_outHost%\"
XCOPY /Y "bwapi.ini" "%k_outClient%\"

XCOPY /Y "TestConfiguration.txt" "%k_outHost%\"
XCOPY /Y "%k_bwamPath%\StartHost.bat" "%k_outHost%\"

XCOPY /Y "%k_bwamPath%\StartClient.bat" "%k_outClient%\"

::Error handling
goto end

:error
echo ************************
echo Errors have occurred.
echo ************************
pause
:end
cd /D "%k_originalDir%"
set PATH=%k_originalPath%
if NOT "%ERRORLEVEL%" == "0" pause
exit /B %ERRORLEVEL%