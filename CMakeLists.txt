cmake_minimum_required(VERSION 3.21)

project(EvolutionSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EVO_SIM_BUILD_TESTS "Build Evo Sim tests" ON)
option(EVO_SIM_FETCH_DEPS "Fetch third-party dependencies with FetchContent" ON)

# Prefer system packages first, then fallback to FetchContent when enabled.
find_package(nlohmann_json QUIET CONFIG)
find_package(Bullet QUIET)
find_package(glfw3 QUIET)
find_package(glad QUIET CONFIG)
find_package(imgui QUIET CONFIG)

include(FetchContent)

if(EVO_SIM_FETCH_DEPS)
  message(STATUS "Dependency fetch fallback is enabled (FetchContent)")

  if(NOT TARGET glfw AND NOT TARGET glfw::glfw)
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
  endif()

  if(NOT TARGET glad AND NOT TARGET glad::glad)
    FetchContent_Declare(
      glad
      GIT_REPOSITORY https://github.com/Dav1dde/glad.git
      GIT_TAG v2.0.8
    )
    FetchContent_MakeAvailable(glad)
  endif()

  # GUI layer: Dear ImGui (OpenGL3 + GLFW backends expected)
  if(NOT TARGET imgui)
    FetchContent_Declare(
      imgui
      GIT_REPOSITORY https://github.com/ocornut/imgui.git
      GIT_TAG v1.91.9b
    )
    FetchContent_MakeAvailable(imgui)

    if(NOT TARGET imgui)
      add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
      )
      target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
      )

      if(TARGET glfw)
        target_link_libraries(imgui PUBLIC glfw)
      elseif(TARGET glfw::glfw)
        target_link_libraries(imgui PUBLIC glfw::glfw)
      endif()
    endif()
  endif()

  if(NOT TARGET BulletDynamics AND NOT TARGET Bullet3Dynamics)
    FetchContent_Declare(
      bullet3
      GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
      GIT_TAG 3.25
    )
    set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
    set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
    set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(bullet3)
  endif()

  if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(
      nlohmann_json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

add_subdirectory(engine)
add_subdirectory(src)

if(EVO_SIM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
