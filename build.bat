@echo off
chcp 65001 >nul
title PAVLOV JB - Derleme

echo ============================================
echo   PAVLOV JB - CS2 Internal Cheat Derleme
echo ============================================
echo.

:: VS yolu kontrol
set VS_PATH=
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools

if "%VS_PATH%"=="" (
    echo [HATA] Visual Studio 2022/2019 bulunamadi!
    echo.
    echo Lutfen Visual Studio 2022 Community'yi yukleyin:
    echo https://visualstudio.microsoft.com/vs/community/
    echo.
    echo Kurulumda "C++ ile masaustu gelistirme" secenegini isaretleyin.
    echo.
    echo Veya su komutla kurun:
    echo winget install Microsoft.VisualStudio.2022.Community ^
    echo   --override "--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --passive"
    echo.
    pause
    exit /b 1
)

echo [OK] Visual Studio bulundu: %VS_PATH%
echo.

:: CMake kontrol
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [HATA] CMake bulunamadi!
    echo Lutfen su komutla kurun: winget install Kitware.CMake
    pause
    exit /b 1
)
echo [OK] CMake bulundu
echo.

:: Ortami hazirla
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64
echo [OK] MSVC ortami hazir
echo.

:: Build klasorunu temizle
if exist build rmdir /s /q build
mkdir build
cd build

:: CMake ile derle
echo [*] CMake ayarlaniyor...
cmake .. -G "Visual Studio 17 2022" >nul 2>nul
if %errorlevel% neq 0 (
    echo [HATA] CMake ayarlari basarisiz!
    pause
    exit /b 1
)

echo [*] Derleniyor...
cmake --build . --config Release -- /nologo /verbosity:quiet
if %errorlevel% neq 0 (
    echo [HATA] Derleme basarisiz!
    pause
    exit /b 1
)

cd ..

echo.
echo ============================================
echo   DERLEME BASARILI!
echo ============================================
echo.
echo Cikti: build\bin\Release\CS2_Internal_Local.dll
echo.
echo Kullanim:
echo   1. CS2'yi baslat
echo   2. Injector ile DLL'yi enjekte et
echo   3. Saga Shift ile menuyu ac/kapa
echo.
pause
