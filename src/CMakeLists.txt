add_executable(evo_sim_app
  main.cpp
)

target_link_libraries(evo_sim_app
  PRIVATE
    evo::core
)

if(TARGET glfw)
  target_link_libraries(evo_sim_app PRIVATE glfw)
endif()

if(TARGET glad)
  target_link_libraries(evo_sim_app PRIVATE glad)
elseif(TARGET glad::glad)
  target_link_libraries(evo_sim_app PRIVATE glad::glad)
endif()

if(TARGET imgui)
  target_link_libraries(evo_sim_app PRIVATE imgui)
endif()
