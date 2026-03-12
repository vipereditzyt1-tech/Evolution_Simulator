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

if(EMSCRIPTEN)
  add_executable(evo_sim_web
    web_main.cpp
    app.cpp
  )

  target_link_libraries(evo_sim_web PRIVATE evo::core)

  target_link_options(evo_sim_web PRIVATE
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=EvolutionModule"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sENVIRONMENT=web"
    "-sFILESYSTEM=0"
    "-sEXPORTED_FUNCTIONS=['_evo_run_frames','_evo_version']"
    "-sEXPORTED_RUNTIME_METHODS=['ccall','UTF8ToString']"
  )

  set_target_properties(evo_sim_web PROPERTIES
    OUTPUT_NAME "evo_sim_web"
    SUFFIX ".js"
  )
endif()
