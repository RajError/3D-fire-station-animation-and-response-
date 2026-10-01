# 🚒 Smart Fire Station – Emergency Response Simulation

A 3D city simulation built with **C++ and OpenGL (FreeGLUT)** that demonstrates how a smart fire station detects a fire, dispatches a fire truck, extinguishes the fire, and returns to base, all in real time.

> Single-file project: `Smart_Fire_Station_Protected.cpp`

---

## ✨ Features

- **Full emergency response workflow** with a state machine:
  `NORMAL → FIRE DETECTED → TRUCK DISPATCHED → RESPONDING → WATER SPRAY ACTIVE → MISSION COMPLETED → RETURNING`
- **Autonomous fire truck** that follows waypoints to the burning building and back to the station
- **Manual driving mode** for the truck (when it is not auto-moving)
- **Fire, smoke and water spray** animations, with fire and smoke levels decreasing as water is applied
- **Live HUD** showing status, fire %, smoke %, and response time
- **Day / Night mode** with sky gradient, ambient lighting change and night fog
- **Living city:** traffic cars, pedestrians, commuters, flying birds and a plane with blinking lights
- **Detailed buildings:** Fire Station, Hospital and School (with interiors), plain buildings, trees, street lights, traffic lights (turn green for the emergency truck), fire hydrant and barriers
- **Multiple camera views**, including interior walkthrough views

---

## 🎮 Controls

### Simulation

| Key | Action |
|-----|--------|
| `E` | Start emergency (set the building on fire) |
| `R` | Reset simulation |
| `SPACE` | Toggle manual water spray (works when the truck is within range of the burning building) |
| `N` | Toggle Day / Night |

### Truck (manual mode)

| Key | Action |
|-----|--------|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Turn left / right |

### Camera

| Key | View |
|-----|------|
| `1` | Front overview |
| `2` | Top-down view |
| `3` | Side view |
| `4` | Truck chase camera |
| `5` | Free orbit camera |
| `I` | School interior |
| `H` | Hospital interior |
| `F` | Fire station interior |
| `←` `→` `↑` `↓` | Rotate / look around (modes 5, I, H, F) |
| `Page Up` / `Page Down` | Zoom in / out (free orbit mode) |

---

## 🛠️ Requirements

- A C++ compiler (g++, MSVC, or clang)
- **OpenGL**, **GLU** and **FreeGLUT**

### Install FreeGLUT

**Ubuntu / Debian**
```bash
sudo apt install freeglut3-dev
```

**macOS (Homebrew)**
```bash
brew install freeglut
```

**Windows**
- Visual Studio: install FreeGLUT via **vcpkg** (`vcpkg install freeglut`) or download the binaries from the [FreeGLUT website](https://freeglut.sourceforge.net/) and link `freeglut.lib` / `opengl32.lib` / `glu32.lib`.
- MinGW: install a FreeGLUT build and link with `-lfreeglut -lopengl32 -lglu32`.

---

## ⚙️ Build & Run

**Linux**
```bash
g++ Smart_Fire_Station_Protected.cpp -o smart_fire_station -lglut -lGLU -lGL -lm
./smart_fire_station
```

**macOS**
```bash
g++ Smart_Fire_Station_Protected.cpp -o smart_fire_station -I/opt/homebrew/include -L/opt/homebrew/lib -lglut -framework OpenGL
./smart_fire_station
```

**Windows (MinGW)**
```bash
g++ Smart_Fire_Station_Protected.cpp -o smart_fire_station.exe -lfreeglut -lopengl32 -lglu32
smart_fire_station.exe
```

---

## 🚀 How to Use

1. Run the program. You will see the city in daylight with the fire truck parked at the station.
2. Press **`E`** to start an emergency. The building catches fire and the station dispatches the truck after a short delay.
3. Watch the truck drive to the scene (press **`4`** for the chase camera) while traffic lights turn green for it.
4. Water spray starts automatically on arrival and puts out the fire.
5. After the mission is completed, the truck returns to the station.
6. Press **`R`** to reset, or **`N`** to switch to night mode and try again.

---

## 🧱 Code Overview

| Section | Description |
|---------|-------------|
| `SimState` enum | State machine for the emergency workflow |
| `update()` | Main simulation loop (~60 FPS timer): truck movement, fire/smoke levels, timers |
| `display()` | Renders the whole scene each frame |
| `setCameraView()` | Handles all 8 camera modes |
| `drawFireStation()`, `drawHospital()`, `drawSchool()` | Buildings and their interiors |
| `drawTruck()`, `drawFireAnimation()`, `drawSmoke()`, `drawWaterSpray()` | Truck and effects |
| `initCityLife()` / `updateCityLife()` | Traffic, pedestrians, commuters, birds and plane |
| `drawStatusText()` | On-screen HUD |

---

## 📸 Screenshots

_Add screenshots or a GIF of the simulation here._

```
![Day view](screenshots/day.png)
![Night view](screenshots/night.png)
```

---

## 📄 License

Add your preferred license here (e.g. MIT) and include a `LICENSE` file in the repository.

---

## 👤 Author

**Your Name**
GitHub: [@your-username](https://github.com/your-username)
