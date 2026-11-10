@echo off
REM ============================================================
REM  compilar.bat - Compila los programas C++ con g++ (MinGW/MSYS2)
REM  Uso (terminal de VS Code, dentro de la carpeta cpp):
REM      .\compilar.bat
REM ============================================================
where g++ >nul 2>nul
if errorlevel 1 (
    echo [ERROR] No se encontro g++ en el PATH.
    echo         Instala MSYS2/MinGW-w64 o usa la "Developer Command Prompt for VS"
    echo         y compila con:  cl /EHsc /std:c++17 /utf-8 rn.cpp
    exit /b 1
)

echo Compilando rn.cpp ...
g++ -std=c++17 -Wall -Wextra -O2 rn.cpp -o rn.exe
if errorlevel 1 goto :error

echo Compilando actividad1_traza.cpp ...
g++ -std=c++17 -Wall -Wextra -O2 actividad1_traza.cpp -o actividad1_traza.exe
if errorlevel 1 goto :error

echo.
echo Compilacion OK. Ejecuta:  .\rn.exe    y    .\actividad1_traza.exe
exit /b 0

:error
echo.
echo [ERROR] Fallo la compilacion. Revisa los mensajes de arriba.
exit /b 1
