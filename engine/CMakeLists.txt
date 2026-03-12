add_library(evo_sim_core
  core.cpp
  ../simulation/elements.cpp
  ../simulation/systems.cpp
)

add_library(evo::core ALIAS evo_sim_core)

target_include_directories(evo_sim_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_compile_features(evo_sim_core PUBLIC cxx_std_20)

if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(evo_sim_core PUBLIC nlohmann_json::nlohmann_json)
endif()

if(TARGET BulletDynamics)
  target_link_libraries(evo_sim_core PUBLIC BulletDynamics BulletCollision LinearMath)
elseif(TARGET Bullet3Dynamics)
  target_link_libraries(evo_sim_core PUBLIC Bullet3Dynamics Bullet3Collision Bullet3Common)
endif()
