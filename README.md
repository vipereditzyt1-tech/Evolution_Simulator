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

### Offline-friendly behavior

- When `EVO_SIM_FETCH_DEPS=ON`, tests fetch GoogleTest via FetchContent.
- When `EVO_SIM_FETCH_DEPS=OFF`, the build tries to use a preinstalled/system `GTest` package.
- If GoogleTest is unavailable and fetching is disabled, CMake emits a warning and skips creating
  `evo_sim_tests` instead of failing configuration.


## Performance foundations

The simulator uses three baseline performance mechanisms:

- **Thread pool/job system:** world chunk updates and per-creature block updates run in parallel via `sim::ThreadPool`.
- **Spatial partitioning:** environment chunks double as a broad-phase grid for neighbor and collision candidate queries.
- **Fixed timestep + interpolation:** simulation ticks run at a fixed dt (`1/60s`) while render frames can run independently and consume interpolation alpha.

### Profiling workflow

1. Configure a release build with tests enabled:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DEVO_SIM_BUILD_TESTS=ON
cmake --build build -j
```

2. Run deterministic and performance suites:

```bash
ctest --test-dir build --output-on-failure -R "DeterministicReplay|PerformanceRegression"
```

3. Collect a CPU profile around benchmark tests (Linux example):

```bash
perf record --call-graph dwarf ./build/tests/evo_sim_tests --gtest_filter=PerformanceRegression.*
perf report
```

4. Iterate on hot paths (`simulation/systems.cpp`, `environment/world.cpp`) and compare average tick time budgets for 500/1000/5000 creatures.

### Optimization toggles

- `-DEVO_SIM_BUILD_TESTS=OFF` to skip test/benchmark binaries in local iteration loops.
- `-DEVO_SIM_FETCH_DEPS=OFF` for offline builds with preinstalled dependencies.
- Runtime simulation dt can be tuned through `sim::FixedTimestepRunner::fixed_dt` for profiling experiments while preserving deterministic stepping semantics.
