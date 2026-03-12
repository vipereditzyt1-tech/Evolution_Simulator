include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(evo_sim_tests
  test_core.cpp
  test_simulation.cpp
  test_ui_data.cpp
  test_unit_sim_math.cpp
  test_integration_ecosystem.cpp
  test_deterministic_replay.cpp
  test_performance_regression.cpp
)

target_link_libraries(evo_sim_tests
  PRIVATE
    evo::core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(evo_sim_tests)
