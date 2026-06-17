# Drone Formation Light Show Simulator

HUST · MSE · Instrumentation — C Course Design Project

**Topic 18: UAV Formation Light Show Simulation**

---

## Features

| Module | Description |
|--------|-------------|
| **Formation** | 15 geometric patterns + text + BMP image rendering |
| **Trajectory** | Waypoint-based linear interpolation with random departure delays |
| **Lighting** | 8 colors, steady/blink, 4 dynamic FX (wave/flow/alternate/rainbow-flow) |
| **Safety** | Boundary check + proximity detection + warning log |
| **History** | Last 5 formations quick recall |
| **Image** | Load BMP and convert to drone pixel grid |

## Keys

| Key | Action |
|-----|--------|
| `S` | Start simulation |
| `P` | Pause / Resume |
| `Q` | Stop |
| `← →` | Cycle patterns (15 geometric + text + image) |
| `↑ ↓` | Adjust speed |
| `C` | Cycle light color |
| `B` | Toggle blink |
| `E` | Cycle light FX (None/Wave/Flow/Alternate/Rainbow) |
| `T` | Input text formation (Chinese/English) |
| `I` | Load BMP image formation |
| `H` | Recall formation history |
| `ESC` | Quit |

## Build

```bash
g++ -std=c++11 -Wall -o drone_show.exe \
    main.cpp \
    src/drone.cpp src/light.cpp src/formation.cpp \
    src/trajectory.cpp src/safety.cpp \
    src/graphics.cpp src/ui.cpp src/file_io.cpp \
    src/controller.cpp \
    -I include -lm -lgdi32
```

**Requirements:** GCC / MinGW-w64, Windows Console API only (no external libs)

## Project Structure

```
├── main.cpp              Entry point
├── commands.txt           Course design task description
├── DESIGN_REPORT.md       Design report
├── include/
│   ├── common.h           Shared types, constants, console colors
│   ├── drone.h            Drone entity module
│   ├── light.h            Lighting control module
│   ├── formation.h        Formation & pattern generation
│   ├── trajectory.h       Trajectory & interpolation
│   ├── safety.h           Safety detection & collision avoidance
│   ├── graphics.h         Console rendering module
│   ├── ui.h               User input module
│   ├── file_io.h          File I/O module
│   └── controller.h       Main controller module
└── src/                   Source implementations
```

## Technical Notes

- **Rendering:** CHAR_INFO framebuffer + WriteConsoleOutputW, zero flicker
- **Text:** GDI per-character rendering to 12x12 pixel grid for Chinese/English
- **Image:** GDI LoadImage + StretchBlt + pixel sampling (PCtoLCD2002-style)
- **Trajectory:** Random departure delay (0-500ms) reduces mid-air overlaps
- **UI:** ASCII-only panel to prevent CJK double-width border misalignment
