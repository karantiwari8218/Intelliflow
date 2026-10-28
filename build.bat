@echo off
g++ -O2 main.cpp Vehicle.cpp TrafficLight.cpp DataStructures.cpp Simulation.cpp -o IntelliFlow.exe -lgdi32 -luser32 -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful. Starting IntelliFlow...
    start IntelliFlow.exe
) else (
    echo Compilation failed.
)

