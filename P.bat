
SETLOCAL EnableDelayedExpansion
@echo off
cls
del build\*.o

::DEBUG only works if you have the map and readelf (which is part of MinGW)
SET "debug="
::SET "cwDWARF="
::if "%1" equ "-d" SET "debug=-debug=0x804992E0 -map=^"Dolphin Emulator\Maps\RMCP01.map^" -readelf=^"C:\MinGW\bin\readelf.exe^""
::if "%1" equ "-d" SET "cwDWARF=-g"

:: Sources and Compiler
SET "ENGINE=C:\Users\ruben\MKWii-CT-distribution\MKWG-main\KamekInclude"
SET "GAMESOURCE=C:\Users\ruben\MKWii-CT-distribution\MKWG-main\GameSource"
SET "PULSAR=C:\Users\ruben\MKWii-CT-distribution\MKWG-main\PulsarEngine"

:: Change this as necessary depending on where you put CodeWarrior
SET "CC=C:\Users\ruben\MKWii-CT-distribution\CodeWarrior\mwcceppc.exe"

:: Riivolution Destination (change as necessary)
SET "RIIVO=C:\Users\ruben\AppData\Roaming\Dolphin Emulator\Load\Riivolution"

:: Compiler flags and folder
SET CFLAGS=-I- -i %ENGINE% -i %GAMESOURCE% -i PulsarEngine ^ -i Code^
  -opt all -inline auto -enum int -proc gekko -fp hard -sdata 0 -sdata2 0 -maxerrors 1 -func_align 4 %cwDWARF%
SET DEFINE=

if "!CC!" == "" (
    echo %CC%
    exit /b 1
)

:: CPP Sources
SET CPPFILES=
for /R %PULSAR% %%f in (*.cpp) do SET "CPPFILES=%%f !CPPFILES!"

:: Compile CPP
%CC% %CFLAGS% -c -o "build/kamek.o" "%ENGINE%\kamek.cpp"
%CC% %CFLAGS% -c -o "build/RuntimeWrite.o" "%ENGINE%\RuntimeWrite.cpp"

SET OBJECTS=
FOR %%H IN (%CPPFILES%) DO (
    ::echo "Compiling %%H..."
    %CC% %CFLAGS% %DEFINE% -c -o "build/%%~nH.o" "%%H"
    SET "OBJECTS=build/%%~nH.o !OBJECTS!"
)

:: Link
echo Linking... %time%
".\KamekLinker\Kamek.exe" "build/kamek.o" "build/RuntimeWrite.o" %OBJECTS% %debug% -dynamic -externals="%GAMESOURCE%/symbols.txt" -versions="%GAMESOURCE%/versions.txt" -output-combined=Code.pul

if %ErrorLevel% equ 0 if NOT "!RIIVO!" == "" (
    xcopy /Y build\*.pul "%RIIVO%\Binaries" /i /q
    echo Binaries copied
)

:end
ENDLOCAL