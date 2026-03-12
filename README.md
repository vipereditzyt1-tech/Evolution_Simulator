# Evolution Simulator

Starter C++ project structure for a cross-platform evolution simulator.

## Layout

- `engine/` - simulation/domain core library (`evo_sim_core`)
- `src/` - desktop app entry point (`evo_sim_app`)
- `tests/` - unit/integration tests (`evo_sim_tests`)
- `simulation/`, `creatures/`, `blocks/`, `environment/`, `ui/` - feature modules
- `assets/` - runtime assets
- `data/` - save data and generated datasets

## Build & Run

### Prerequisites

- CMake 3.21+
- A C++20 compiler:
  - **Windows:** Visual Studio 2022 (MSVC) or clang-cl
  - **Linux:** GCC 11+ or Clang 14+
  - **macOS:** Xcode 14+ (AppleClang)
- Git (required when `EVO_SIM_FETCH_DEPS=ON` to pull dependencies)

Dependencies are configured via **CMake FetchContent**:

- Rendering stack: GLFW + GLAD
- Physics: Bullet
- Serialization: nlohmann_json
- GUI: Dear ImGui
- Testing: GoogleTest

### Windows (Visual Studio generator)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\evo_sim_app.exe
```

### Linux (Ninja)

```bash
cmake -S . -B build -G Ninja
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/src/evo_sim_app
```

### macOS (Ninja)

```bash
cmake -S . -B build -G Ninja
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/src/evo_sim_app
```

### Optional configure flags

- `-DEVO_SIM_FETCH_DEPS=OFF` to disable FetchContent downloads.
- `-DEVO_SIM_BUILD_TESTS=OFF` to skip test target generation.
