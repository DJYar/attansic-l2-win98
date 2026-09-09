@echo off
setlocal

set "REPOROOT=%~dp0.."
if defined L2_DDK_ROOT (
    set "DDKROOT=%L2_DDK_ROOT%"
) else (
    set "DDKROOT=C:\NTDDK"
)
if defined L2_VC6_ROOT (
    set "VCROOT=%L2_VC6_ROOT%"
) else (
    set "VCROOT=C:\Tools\VC6_Toolchain\VC98"
)
if defined L2_VC6_COMMON (
    set "VCCOMMON=%L2_VC6_COMMON%"
) else (
    set "VCCOMMON=C:\Tools\VC6_Toolchain\Common\MSDev98"
)
set "BUILDTYPE=checked"
set "OBJDIR=objchk"
set "DDKLIB=libchk"

if "%~1"=="" goto buildtype_ready
if /i "%~1"=="checked" goto buildtype_ready
if /i "%~1"=="free" goto free_build
echo ERROR: usage: build-win10.cmd [checked or free]
exit /b 2

:free_build
set "BUILDTYPE=free"
set "OBJDIR=objfre"
set "DDKLIB=libfre"

:buildtype_ready

if not exist "%VCROOT%\Bin\cl.exe" (
    echo ERROR: VC6 compiler not found at %VCROOT%.
    exit /b 1
)

if not exist "%DDKROOT%\bin\build.exe" (
    echo ERROR: DDK build.exe not found at %DDKROOT%.
    echo Set L2_DDK_ROOT to the external Windows 2000 DDK directory.
    exit /b 1
)

call "%DDKROOT%\bin\setenv.bat" %DDKROOT% %BUILDTYPE%

rem The archived DDK's ddkvars.bat contains paths from its original machine.
rem Override them after setenv while preserving the DDK variables it supplied.
set "MSVCDIR=%VCROOT%"
set "PATH=%VCCOMMON%\Bin;%VCROOT%\Bin;%DDKROOT%\bin;%SystemRoot%\System32;%SystemRoot%"
set "INCLUDE=%VCROOT%\ATL\INCLUDE;%VCROOT%\INCLUDE;%VCROOT%\MFC\INCLUDE;%DDKROOT%\inc"
set "LIB=%VCROOT%\LIB;%VCROOT%\MFC\LIB;%DDKROOT%\%DDKLIB%"

cd /d "%REPOROOT%\driver\src"
if exist "%OBJDIR%\i386\l2ndis.sys" del /q "%OBJDIR%\i386\l2ndis.sys"

"%DDKROOT%\bin\build.exe" -cZ

if not exist "%OBJDIR%\i386\l2ndis.sys" (
    echo ERROR: build completed without producing l2ndis.sys.
    exit /b 1
)

echo SUCCESS: %CD%\%OBJDIR%\i386\l2ndis.sys
exit /b 0
