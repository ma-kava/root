@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM =====================================================
REM Initialize MSVC environment
REM =====================================================
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment.
    exit /b 1
)

cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSVC compiler not usable.
    exit /b 1
)

REM =====================================================
REM Read OpenSSL version from lib_versions
REM =====================================================
set "VERSION_FILE=lib_versions"

for /f "usebackq tokens=1,2 delims==" %%A in ("%VERSION_FILE%") do (
    if "%%A"=="openssl" (
        set "OPENSSL_VERSION=%%B"
    )
)

if not defined OPENSSL_VERSION (
    echo [ERROR] OpenSSL version not found in lib_versions
    exit /b 1
)

REM =====================================================
REM Configuration
REM =====================================================
set ROOT_DIR=%cd%
set TMP_DIR=%ROOT_DIR%\_tmp
set DIST_DIR=%TMP_DIR%\dist\openssl
set OPENSSL_SRC=openssl-%OPENSSL_VERSION%
set OPENSSL_TAR=openssl-%OPENSSL_VERSION%.tar.gz

set OPENSSL_REPO=https://www.openssl.org/source/%OPENSSL_TAR%
set INSTALL_PREFIX=%DIST_DIR%

REM =====================================================
REM Prepare temp directory
REM =====================================================
mkdir %TMP_DIR% >nul 2>&1
cd %TMP_DIR%

REM =====================================================
REM Download OpenSSL
REM =====================================================
if not exist %OPENSSL_TAR% (
    echo [*] Downloading OpenSSL %OPENSSL_VERSION% ...
    curl -LO %OPENSSL_REPO%
    if errorlevel 1 exit /b 1
)

REM =====================================================
REM Extract OpenSSL
REM =====================================================
if not exist %OPENSSL_SRC% (
    tar -xf %OPENSSL_TAR%
    if errorlevel 1 exit /b 1
)

cd %OPENSSL_SRC%

REM =====================================================
REM Clean previous build
REM =====================================================
if exist Makefile del /f /q Makefile
if exist build rmdir /s /q build

REM =====================================================
REM Configure OpenSSL (shared, MSVC)
REM =====================================================
perl Configure VC-WIN64A ^
    --prefix=%INSTALL_PREFIX% ^
    --openssldir=%INSTALL_PREFIX%\ssl ^
    shared ^
    no-tests

if errorlevel 1 (
    echo [ERROR] OpenSSL configuration failed
    exit /b 1
)

REM =====================================================
REM Build & install
REM =====================================================
nmake
if errorlevel 1 exit /b 1

nmake install_sw install_ssldirs
if errorlevel 1 exit /b 1

REM =====================================================
REM Copy result to repo root (fmt/spdlog style)
REM =====================================================
cd %ROOT_DIR%

if exist openssl3.0 rmdir /s /q openssl3.0
mkdir openssl3.0
mkdir openssl3.0\win64

xcopy %INSTALL_PREFIX%\include openssl3.0\include /E /I /Y
xcopy %INSTALL_PREFIX%\ssl openssl3.0\ssl /E /I /Y

copy %INSTALL_PREFIX%\lib\libssl.lib openssl3.0\win64\libssl.lib
copy %INSTALL_PREFIX%\lib\libcrypto.lib openssl3.0\win64\libcrypto.lib

copy %INSTALL_PREFIX%\bin\libssl-3.dll openssl3.0\win64\libssl-3.dll
copy %INSTALL_PREFIX%\bin\libcrypto-3.dll openssl3.0\win64\libcrypto-3.dll

echo [*] OpenSSL %OPENSSL_VERSION% ready (win64)
echo Done
