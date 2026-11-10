@echo off
REM ============================================================
REM  ejecutar_todo.bat - Compila y corre TODO en orden.
REM  Uso (terminal de VS Code, en la carpeta raiz del proyecto):
REM      .\ejecutar_todo.bat
REM  Para guardar las salidas en archivos de texto (para el informe):
REM      .\ejecutar_todo.bat > salidas.txt
REM ============================================================
echo ##### C++ : compilar #####
pushd cpp
call compilar.bat
if errorlevel 1 (popd & exit /b 1)
echo.
echo ##### C++ : rn.exe (pruebas) #####
.\rn.exe
echo.
echo ##### C++ : actividad1_traza.exe #####
.\actividad1_traza.exe
popd

echo.
echo ##### PYTHON : pruebas_rn.py #####
pushd python
python pruebas_rn.py
echo.
echo ##### PYTHON : experimento.py #####
python experimento.py
echo.
echo ##### PYTHON : siga_rn.py #####
python siga_rn.py
popd
