@echo off
rem Script that coverts built Pixet from _build to installation package of choice located in _build 
rem Chosen License is generated, supplied to the installation package along with pixet.ini settings (edu, default save location) and with configs that are taken from devices_configs

setlocal EnableDelayedExpansion
rem take arguments zip edu -lic

SET WORKDIR=%CD%
SET BUILD_DIR=_build
SET PACKAGE_DIR=%BUILD_DIR%\package

SET "api=false"
SET "zip=false"
SET "edu=false"
SET "inno=false"
SET "defLic=Advacam s.r.o."
SET DISTRIBUTION_VERSION="internal"
SET PIXET_VERSION=""

IF EXIST "%WORKDIR%\src\common\ipixet.h" (
   FOR /f tokens^=^2^ usebackq^ delims^=^" %%i IN (`
      powershell -c "Select-String -Path src\common\ipixet.h -Pattern 'PX_PIXET_VERSION'"
   `) DO SET PIXET_VERSION=%%i
)

SET "selectedOne=false"

:Loop
   IF "%1"=="api" (
      SET "api=true" 
      SHIFT
      SET "selectedOne=true"
      GOTO Loop 
   )
   IF "%1"=="zip" (
      SET "zip=true"
      SHIFT 
      SET "selectedOne=true"
      GOTO Loop 
   )
   IF "%1"=="inno" (
      SET "inno=true"
      SHIFT 
      SET "selectedOne=true"
      GOTO Loop 
   )
   IF "%1"=="edu" (
      SET "edu=true"
      SHIFT 
      SET "selectedOne=true"
      SET "defLic=EDU"
      GOTO Loop 
   )     
   IF "%1"=="-lic" ( 
      SET defLic=%2
      SHIFT
      SHIFT
      SET "selectedOne=true"
      GOTO Loop 
   )
   IF "%1"=="-pversion" ( 
      SET PIXET_VERSION=%2
      SHIFT
      SHIFT
      GOTO Loop 
   )
   IF "%1"=="-distribution_version" ( 
      SET DISTRIBUTION_VERSION=%2
      SHIFT
      SHIFT
      GOTO Loop 
   )
:EndLoop
rem help response

IF "%selectedOne%"=="false" (
	echo Did not catch any valid package arguments. 
	echo Optional arguments are [zip][edu][inno][api][-lic "x"]
	EXIT /B
)

IF [%PIXET_VERSION%] == [] (
   echo.
   echo "Pixet version has to be set by -pversion parameter"
   echo.
   EXIT /B
)

powershell -Command "(gc win\win64.iss.template) -replace '#PIXET_VERSION#', '%PIXET_VERSION%' | Out-file -encoding ASCII win\win64.iss"

if exist %PACKAGE_DIR% rmdir %PACKAGE_DIR% /s /q
mkdir %PACKAGE_DIR%\Pixet64

rem copy actual pixet build to %PACKAGE_DIR%\Pixet
for /f "tokens=* USEBACKQ" %%i in (`powershell -Command "Get-ChildItem '_build' -Filter '*.zip' | Where-Object {$_.Name -cmatch '^Pixet_Pro.*Windows_x64\.zip$'} | Select-Object -ExpandProperty Name"`) do set BUILD_NAME=%%i
for /f "tokens=* USEBACKQ" %%i in (`powershell -Command "Get-ChildItem '_build' -Filter '*.zip' | Where-Object {$_.Name -cmatch '^Pixet_API.*Windows_x64\.zip$'} | Select-Object -ExpandProperty Name"`) do set API_NAME=%%i
echo found pixet build: %BUILD_NAME%
echo found api build: %API_NAME%

powershell -Command "Expand-Archive '%BUILD_DIR%\%BUILD_NAME%' '%PACKAGE_DIR%\Pixet64'"
rem xcopy "%BUILD_DIR%\Pixet_Pro*Windows_x64.zip" "%PACKAGE_DIR%\Pixet64" /e /i /q /y
rem generate license, call generate_license.py
python generate_license.py %defLic%
copy /Y lic.info %PACKAGE_DIR%
move lic.info %PACKAGE_DIR%\Pixet64

rem alter pixet.ini  -zip/edu
if "%edu%"=="true" powershell -Command "(gc %PACKAGE_DIR%\Pixet64\pixet.ini) -replace ';MainUi=devcontrol', 'MainUi=eduview' | Out-file -encoding ASCII %PACKAGE_DIR%\Pixet64\pixet.ini

rem copy configs to factory and config from device_configs
mkdir %PACKAGE_DIR%\Pixet64\configs
mkdir %PACKAGE_DIR%\Pixet64\factory

echo "config dir" > %PACKAGE_DIR%\Pixet64\configs\info.txt
echo "factory config dir" > %PACKAGE_DIR%\Pixet64\factory\info.txt

rem 7 args for robocopy just to make it silent
robocopy "_build\devices_configs" "%PACKAGE_DIR%\Pixet64\configs" /xf .gitignore  /NFL /NDL /NJH /NJS /nc /ns /np
robocopy "_build\devices_configs" "%PACKAGE_DIR%\Pixet64\factory" /xf .gitignore  /NFL /NDL /NJH /NJS /nc /ns /np

REM Remove unwanted plugins
python purge_pixet.py --build-dir "%PACKAGE_DIR%\Pixet64" --xml-config "plugin_cookbook.xml" --distrib-version %DISTRIBUTION_VERSION% --platform Windows_x64

IF "%inno%"=="true" (
   echo Running innosetup
   rem run inno64
   iscc /q win\win64.iss
)

rem zip if tagged
IF "%zip%"=="true" (
    echo zipping pixet
	powershell -Command "(gc %PACKAGE_DIR%\Pixet64\pixet.ini) -replace 'UseAppDataDir=true', 'UseAppDataDir=false' | Out-file -encoding ASCII %PACKAGE_DIR%\Pixet64\pixet.ini"

	7z a -tzip %BUILD_DIR%\PixetWin64.zip "%WORKDIR%\%PACKAGE_DIR%\Pixet64"
)

IF "%api%"=="true" (
   echo zipping api
   mkdir %PACKAGE_DIR%\api64
   powershell -Command "Expand-Archive '%BUILD_DIR%\%API_NAME%' '%PACKAGE_DIR%\api64'"
   copy /Y %PACKAGE_DIR%\lic.info %PACKAGE_DIR%\api64
   7z a -tzip %BUILD_DIR%\PixetAPIWin64.zip "%WORKDIR%\%PACKAGE_DIR%\api64\*.*"

   rem mkdir %PACKAGE_DIR%\api32
   rem powershell -Command "Expand-Archive %BUILD_DIR%\PIXet_API*x32.zip %PACKAGE_DIR%\api32"
   rem copy /Y %PACKAGE_DIR%\lic.info %PACKAGE_DIR%\api32
   rem 7z a -tzip %BUILD_DIR%\PixetAPIWin32.zip "%WORKDIR%\%PACKAGE_DIR%\api32"
 
)
