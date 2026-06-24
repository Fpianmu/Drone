@echo off
echo ========================================
echo  Drone Light Show - Building...
echo ========================================

g++ -std=c++11 -Wall -o drone_show.exe ^
    main.cpp ^
    src/drone.cpp ^
    src/light.cpp ^
    src/formation.cpp ^
    src/trajectory.cpp ^
    src/safety.cpp ^
    src/graphics.cpp ^
    src/ui.cpp ^
    src/file_io.cpp ^
    src/controller.cpp ^
    -I include -lm -lgdi32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo  Build SUCCESS - drone_show.exe ready
    echo ========================================
) else (
    echo.
    echo ========================================
    echo  Build FAILED - check errors above
    echo ========================================
)

pause
