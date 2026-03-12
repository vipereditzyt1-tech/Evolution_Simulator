add_executable(evo_sim_app
  main.cpp
  app.cpp
)

target_link_libraries(evo_sim_app
  PRIVATE
    evo::core
)

if(TARGET glfw)
  target_link_libraries(evo_sim_app PRIVATE glfw)
elseif(TARGET glfw::glfw)
  target_link_libraries(evo_sim_app PRIVATE glfw::glfw)
endif()

if(TARGET glad)
  target_link_libraries(evo_sim_app PRIVATE glad)
elseif(TARGET glad::glad)
  target_link_libraries(evo_sim_app PRIVATE glad::glad)
endif()

if(TARGET imgui)
  target_link_libraries(evo_sim_app PRIVATE imgui)
elseif(TARGET imgui::imgui)
  target_link_libraries(evo_sim_app PRIVATE imgui::imgui)
endif()
