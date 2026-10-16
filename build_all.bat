@echo off
REM ============================================================================
REM DSA Summative Project - Automated Build Script
REM Compiles all 5 tasks using available C compiler (MSVC cl.exe or GCC)
REM ============================================================================

echo ========================================================
echo        DSA SUMMATIVE PROJECT: BUILDING ALL 5 TASKS
echo ========================================================

REM Check if GCC is available
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] GCC compiler detected. Using GCC...
    set CC=gcc
    set CFLAGS=-Wall -Wextra -std=c99
    goto BUILD_GCC
)

REM Check if MSVC Build Tools are installed
set MSVC_VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if exist %MSVC_VCVARS% (
    echo [FOUND] MSVC Build Tools detected. Initializing environment...
    call %MSVC_VCVARS% >nul
    set CC=cl
    set CFLAGS=/W4 /MD /O2 /nologo /D_CRT_SECURE_NO_WARNINGS
    goto BUILD_MSVC
)

echo [ERROR] Neither GCC nor MSVC Build Tools were found in PATH.
echo Please compile manually or install a C compiler.
pause
exit /b 1

:BUILD_MSVC
echo.
echo [1/5] Compiling Task 1: Event Logger (Doubly Linked List)...
cd /d "%~dp0task1_event_logger"
%CC% %CFLAGS% main.c /Fe:event_logger.exe

echo.
echo [2/5] Compiling Task 2: Command Registry (Binary Search Tree)...
cd /d "%~dp0task2_command_registry"
%CC% %CFLAGS% main.c /Fe:command_registry.exe

echo.
echo [3/5] Compiling Task 3: Server Dependency Network (Directed Graph)...
cd /d "%~dp0task3_server_network"
%CC% %CFLAGS% main.c /Fe:server_network.exe

echo.
echo [4/5] Compiling Task 4: Medical Supply Routing (Dijkstra Graph)...
cd /d "%~dp0task4_medical_routing"
%CC% %CFLAGS% main.c /Fe:medical_routing.exe

echo.
echo [5/5] Compiling Task 5: Log Compression (Huffman Coding)...
cd /d "%~dp0task5_log_compression"
%CC% %CFLAGS% main.c /Fe:log_compression.exe

goto FINISH

:BUILD_GCC
echo.
echo [1/5] Compiling Task 1: Event Logger (Doubly Linked List)...
cd /d "%~dp0task1_event_logger"
%CC% %CFLAGS% main.c -o event_logger.exe

echo.
echo [2/5] Compiling Task 2: Command Registry (Binary Search Tree)...
cd /d "%~dp0task2_command_registry"
%CC% %CFLAGS% main.c -o command_registry.exe

echo.
echo [3/5] Compiling Task 3: Server Dependency Network (Directed Graph)...
cd /d "%~dp0task3_server_network"
%CC% %CFLAGS% main.c -o server_network.exe

echo.
echo [4/5] Compiling Task 4: Medical Supply Routing (Dijkstra Graph)...
cd /d "%~dp0task4_medical_routing"
%CC% %CFLAGS% main.c -o medical_routing.exe

echo.
echo [5/5] Compiling Task 5: Log Compression (Huffman Coding)...
cd /d "%~dp0task5_log_compression"
%CC% %CFLAGS% main.c -o log_compression.exe

goto FINISH

:FINISH
cd /d "%~dp0"
echo.
echo ========================================================
echo        ALL 5 TASKS HAVE BEEN BUILT SUCCESSFULLY!
echo ========================================================
echo You can run any task by navigating to its directory:
echo   - cd task1_event_logger   ^&^& event_logger.exe
echo   - cd task2_command_registry ^&^& command_registry.exe
echo   - cd task3_server_network ^&^& server_network.exe
echo   - cd task4_medical_routing ^&^& medical_routing.exe
echo   - cd task5_log_compression ^&^& log_compression.exe
echo ========================================================
